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

const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: state(READ_REQUEST), mysched(sched), sockfd(fd), filefd(-1)
    {
    TRACE();

    // Store the peer's address as ASCII string; we'll need that
    // again later.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_addr_str, sizeof(peer_addr_str)) == NULL)
	strcpy(peer_addr_str, "unknown");

    // Set socket parameters.

    int true_flag = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &true_flag, sizeof(int)) == -1)
	throw system_error("Can't set KEEPALIVE mode");

    linger ling;
    ling.l_onoff  = 0;
    ling.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
	throw system_error("Can't switch LINGER mode off");

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
    ssize_t rc = write(sockfd, buffer.c_str(), buffer.size());
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
	if (buffer.size() < 4*1024)
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
    char buf[1024];
    ssize_t rc = read(sockfd, buf, sizeof(buf));
    if (rc <= 0)
	{
	info("%d: Reading from connection returned an error; closing connection.", sockfd);
	delete this;
	return;
	}
    buffer.append(buf, rc);
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
	    if (host.empty() && uri.empty() && full_get_regex.submatch(line, vec))
		{
		host = vec[1];
		uri  = vec[2];
		}
	    else if (uri.empty() && get_regex.submatch(line, vec))
		{
		uri = vec[1];
		}
	    else if (host.empty() && host_regex.submatch(line, vec))
		{
		host = vec[1];
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

	    filefd = open(filename.c_str(), O_RDONLY, 0);
	    if (filefd == -1)
		{
		error("Can't open requested file %s: %s", filename.c_str(), strerror(errno));
		file_not_found(filename);
		return;
		}

	    info("%d: %s GET %s --> %s", sockfd, peer_addr_str, uri.c_str(), filename.c_str());
	    state = WRITE_ANSWER;
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
    char buf[16 * 1024];
    ssize_t rc = read(filefd, buf, sizeof(buf));
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
	buffer.append(buf, rc);
	prop.poll_events = POLLOUT;
	prop.write_timeout = config->network_write_timeout;
	mysched.register_handler(sockfd, *this, prop);
	if (buffer.size() <= 8 * 1024)
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
	"HTTP/1.0 200 OK\r\n" \
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
