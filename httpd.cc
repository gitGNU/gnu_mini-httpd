/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace std;
#include "httpd.hh"
#include "system-error.hh"
#include "log.hh"
#include "config.hh"

const RegExp RequestHandler::full_get_port_regex("^GET http://([^:]+):[0-9]+(/[^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +(/[^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_port_regex("^Host: +([^ ]+):[0-9]+", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

char* RequestHandler::tmp = 0;
unsigned RequestHandler::instances = 0;

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: state(READ_REQUEST), mysched(sched), sockfd(fd), filefd(-1)
    {
    TRACE();

    // If the buffer is not initialized yet, do it now.

    if (tmp == 0)
	{
	tmp = new char[config->read_block_size];
	debug("This is the first RequestHandler instance; initialized static 'tmp' buffer.");
	}

    // Count active instances of this class.

    ++instances;

    // Store the peer's address as ASCII string; we'll need that
    // again later.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_addr_str, sizeof(peer_addr_str)) == NULL)
	strcpy(peer_addr_str, "unknown");

    // Set socket parameters.

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
	throw system_error("Can set non-blocking mode");

    // Register ourselves with the scheduler.

    prop.poll_events   = POLLIN;
    prop.read_timeout  = config->network_read_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

RequestHandler::~RequestHandler()
    {
    TRACE();
    mysched.remove_handler(sockfd);
    close(sockfd);
    if (filefd >= 0)
	{
	mysched.remove_handler(filefd);
	close(filefd);
	}
    if (--instances == 0 && tmp != 0)
	{
	debug("No RequestHandler instances left; freeing static 'tmp' buffer.");
	delete[] tmp;
	tmp = 0;
	}
    }

void RequestHandler::fd_is_readable(int)
    {
    TRACE();
    switch(state)
	{
	case READ_REQUEST:
	    trace("%d: fd_is_readable() invoked in READ_REQUEST state.", sockfd);
	    read_request();
	    break;
	case WRITE_ANSWER:
	    trace("%d: fd_is_readable() invoked in WRITE_ANSWER state.", sockfd);
	    read_file();
	    break;
	case TERMINATE:
	    throw logic_error("The reading handler should not be registered in this state!");
	default:
	    throw logic_error("The internal state of the RequestHandler is messed up.");
	}
    }

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    ssize_t rc = write(sockfd, buffer.data(), buffer.size());
    if (rc <= 0)
	{
	info("%d: Writing to connection returned an error; closing connection.", sockfd);
	delete this;
	return;
	}
    debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
    buffer.erase(0, rc);

    if (state == WRITE_ANSWER)
	{
	if (buffer.empty())
	    {
	    debug("%d: Write buffer is empty. Don't engage write handler until we have data again.", sockfd);
	    mysched.remove_handler(sockfd);
	    }
	if (buffer.size() < config->min_buffer_fill_size)
	    {
	    debug("%d: Write buffer contains %d bytes; engage read handler again.", sockfd, buffer.size());
	    prop.poll_events   = POLLIN;
	    prop.read_timeout  = config->file_read_timeout;
	    mysched.register_handler(filefd, *this, prop);
	    }
	}
    if (state == TERMINATE && buffer.empty())
	{
	debug("%d: Buffer is empty and we're in TERMINATE state ... terminating.", sockfd);
	delete this;
	}
    }

void RequestHandler::read_timeout(int)
    {
    TRACE();
    info("%d: Read timout; terminating connection.", sockfd);
    delete this;
    }

void RequestHandler::write_timeout(int)
    {
    TRACE();
    info("%d: Write timout; terminating connection.", sockfd);
    delete this;
    }

void RequestHandler::read_request()
    {
    TRACE();
    ssize_t rc = read(sockfd, tmp, config->read_block_size);
    if (rc <= 0)
	{
	info("%d: Reading from connection returned an error; closing connection.", sockfd);
	delete this;
	return;
	}
    buffer.append(tmp, rc);
    while(!buffer.empty())
	{
	string::size_type pos = buffer.find("\r\n");
	if (pos == string::npos)
	    break;

	string line = buffer.substr(0, pos);
	buffer.erase(0, pos+2);
	if (!line.empty())
	    {
	    vector<string> vec;
	    if (host.empty() && uri.empty() && full_get_port_regex.submatch(line, vec))
		{
		host = vec[1];
		uri  = vec[2];
		debug("%d: Got full GET request including port: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
		}
	    else if (host.empty() && uri.empty() && full_get_regex.submatch(line, vec))
		{
		host = vec[1];
		uri  = vec[2];
		debug("%d: Got full GET request: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
		}
	    else if (uri.empty() && get_regex.submatch(line, vec))
		{
		uri = vec[1];
		debug("%d: Got GET request: uri = '%s'.", sockfd, uri.c_str());
		}
	    else if (host.empty() && host_port_regex.submatch(line, vec))
		{
		host = vec[1];
		debug("%d: Got Host header including port: host = '%s'.", sockfd, host.c_str());
		}
	    else if (host.empty() && host_regex.submatch(line, vec))
		{
		host = vec[1];
		debug("%d: Got Host header: host = '%s'.", sockfd, host.c_str());
		}
	    else
		{
		debug("%d: Got unknown header '%s'.", sockfd, line.c_str());
		}
	    }
	else
	    {
	    // We have the complete request. Now we can open the file
	    // we'll reply with and continue to the next state.

	    string filename = config->document_root + "/" + host + uri;
	    struct stat sbuf;
	    if (stat(filename.c_str(), &sbuf) == -1)
		{
		error("Can't stat requested file %s: %s", filename.c_str(), strerror(errno));
		file_not_found(filename);
		return;
		}

	    if (S_ISDIR(sbuf.st_mode))
		filename.append("/index.html");

	    filefd = open(filename.c_str(), O_RDONLY | O_NONBLOCK, 0);
	    if (filefd == -1)
		{
		error("Can't open requested file %s: %s", filename.c_str(), strerror(errno));
		file_not_found(filename);
		return;
		}

	    info("%d: %s GET %s --> %s", sockfd, peer_addr_str, uri.c_str(), filename.c_str());

	    state = WRITE_ANSWER;
	    buffer = "HTTP/1.0 200 OK\r\nContent-Type: ";
	    buffer += config->get_content_type(filename);
	    buffer += "\r\n\r\n";
	    debug("%d: Registering read-handler to read from file; going into WRITE_ANSWER state.", sockfd);
	    prop.poll_events  = POLLIN;
	    prop.read_timeout = config->file_read_timeout;
	    mysched.register_handler(filefd, *this, prop);

	    break;
	    }
	}
    }

void RequestHandler::read_file()
    {
    TRACE();
    ssize_t rc = read(filefd, tmp, config->read_block_size);
    if (rc < 0)
	{
	error("%d: Reading from file returned an error; closing connection.", sockfd);
	delete this;
	}
    else if (rc == 0)
	{
	if (!buffer.empty())
	    {
	    debug("%d: Read whole file; going into TERMINATE state.", sockfd);
	    state = TERMINATE;
	    prop.poll_events = POLLOUT;
	    prop.write_timeout = config->network_write_timeout;
	    mysched.register_handler(sockfd, *this, prop);
	    mysched.remove_handler(filefd);
	    filefd = -1;
	    close(filefd);
	    }
	else
	    {
	    debug("%d: Read and sent the whole file; terminating.", sockfd);
	    delete this;
	    }
	}
    else
	{
	debug("%d: Read %d bytes from file into the buffer.", sockfd, rc);
	buffer.append(tmp, rc);
	prop.poll_events = POLLOUT;
	prop.write_timeout = config->network_write_timeout;
	mysched.register_handler(sockfd, *this, prop);
	if (buffer.size() <= config->max_buffer_fill_size)
	    {
	    debug("%d: Buffer contains %d bytes, so we'll continue to read into it.", sockfd, buffer.size());
	    prop.poll_events = POLLIN;
	    prop.read_timeout = config->file_read_timeout;
	    mysched.register_handler(filefd, *this, prop);
	    }
	else
	    {
	    debug("%d: Buffer contains %d bytes; we'll empty it before continuing to read from file.", sockfd, buffer.size());
	    mysched.remove_handler(filefd);
	    }
	}
    }

void RequestHandler::file_not_found(const string& file)
    {
    TRACE();
    debug("%d: Create file-not-found-page for %s in buffer and write it back to the user.", sockfd, file.c_str());
    buffer = \
	"HTTP/1.0 404 Not Found\r\n" \
	"Content-Type: text/html\r\n" \
	"\r\n" \
	"<html>\r\n" \
	"<head>\r\n" \
	"  <title>Page does not exist!</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"The page you requested does not exist on this server ...\r\n" \
	"</body>\r\n" \
	"</html>\r\n";
    state = TERMINATE;
    prop.poll_events = POLLOUT;
    mysched.register_handler(sockfd, *this, prop);
    }
