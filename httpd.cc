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

const RegExp RequestHandler::full_get_port_regex("^GET http://([^:]+):[0-9]+([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_port_regex("^Host: +([^ ]+):[0-9]+", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

char* RequestHandler::tmp = 0;
unsigned RequestHandler::instances = 0;

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	:
    state(READ_REQUEST), mysched(sched), sockfd(fd), filefd(-1)
    {
    TRACE();

    // If the buffer is not initialized yet, do it now.

    if (tmp == 0)
	{
	tmp = new char[config->read_block_size];
	debug("This is the first RequestHandler instance; initialized static 'tmp' buffer.");
	}
    ++instances;

    // Store the peer's address as ASCII string; we'll need that
    // again later.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_addr_str, sizeof(peer_addr_str)) == NULL)
	strcpy(peer_addr_str, "unknown");

    // Set socket parameters.

    linger ling;
    ling.l_onoff  = 0;
    ling.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
	throw system_error("Can't switch LINGER mode off");

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
	throw system_error("Can set non-blocking mode");

    // Register ourselves with the scheduler.

    network_properties.poll_events   = 0;
    network_properties.read_timeout  = config->network_read_timeout;
    network_properties.write_timeout = config->network_write_timeout;
    file_properties.poll_events      = 0;
    file_properties.read_timeout     = config->file_read_timeout;
    file_properties.write_timeout    = 0;
    register_network_read_handler();
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

void RequestHandler::fd_is_readable(int fd)
    {
    TRACE();
    ssize_t rc = read(fd, tmp, config->read_block_size);

    if (state == READ_REQUEST)
	{
	if (rc <= 0)
	    {
	    info("%d: Reading from network returned an error: closing connection.", sockfd);
	    delete this;
	    return;
	    }
	debug("%d: Read %d bytes from network peer.", sockfd, rc);
	const char* begin;
	const char* current;
	const char* end;
	const char* eol;
	if (buffer.empty())
	    {
	    debug("%d: Our input buffer is empty, we can handle the input without copying.", sockfd);
	    begin = tmp;
	    end   = tmp + rc;
	    }
	else
	    {
	    debug("%d: Our input buffer contains %d bytes, we have to copy.", sockfd, buffer.size());
	    buffer.append(tmp, rc);
	    begin = buffer.c_str();
	    end   = buffer.c_str() + buffer.size();
	    }
	current = begin;
	do
	    {
	    eol = strstr(current, "\r\n");
	    if (eol)
		{
		if (process_input(current, eol) == true)
		    {
		    debug("%d: Read request is complete.", sockfd);
		    remove_network_read_handler();
		    if (host.empty())
			{
			protocol_error("no host");
			return;
			}
		    else if (uri.empty())
			{
			protocol_error("no uri");
			return;
			}

		    string filename = config->document_root + "/" + host + uri;
		    struct stat sbuf;
		    if (stat(filename.c_str(), &sbuf) == -1)
			{
			error("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
			file_not_found(filename);
			return;
			}

		    if (S_ISDIR(sbuf.st_mode))
			filename.append("/index.html");

		    filefd = open(filename.c_str(), O_RDONLY | O_NONBLOCK, 0);
		    if (filefd == -1)
			{
			error("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
			file_not_found(filename);
			return;
			}
		    info("%d: %s GET %s --> %s", sockfd, peer_addr_str, uri.c_str(), filename.c_str());
		    state = WRITE_ANSWER;
		    buffer = "HTTP/1.0 200 OK\r\nContent-Type: ";
		    buffer += config->get_content_type(filename);
		    buffer += "\r\n\r\n";
		    rc = write(sockfd, buffer.data(), buffer.size());
		    if (rc < 0)
			{
			if (errno == EAGAIN || errno == EINTR)
			    {
			    debug("%d: Can't write the header for the request answer right now. Using buffer.", sockfd);
			    register_network_write_handler();
			    }
			else
			    {
			    info("%d: Writing to connection returned error '%s'; closing connection.",
				 sockfd, strerror(errno));
			    delete this;
			    }
			}
		    else
			{
			debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
			if (static_cast<string::size_type>(rc) < buffer.size())
			    {
			    debug("%d: Could not write all %d bytes at once, using buffer for remaining %d bytes.",
				  sockfd, buffer.size(), buffer.size()-rc);
			    buffer.erase(0, rc);
			    register_network_write_handler();
			    }
			else
			    {
			    buffer.clear();
			    register_file_read_handler();
			    }
			}
		    return;
		    }
		current = eol + 2;
		}
	    else
		debug("%d: Our input buffer does not contain any more complete lines.", sockfd);
	    }
	while (eol);
	if (current < end)
	    {
	    if (buffer.empty())
		{
		debug("%d: Append remaining %d input bytes to the (empty) buffer.", sockfd, end-current);
		buffer.append(current, end-current);
		}
	    else
		{
		debug("%d: Remove the processed %d input bytes from the buffer.", sockfd, current-begin);
		buffer.erase(0, current-begin);
		debug("%d: Buffer contains %d bytes after removal.", sockfd, buffer.size());
		}
	    }
	else
	    {
	    if (!buffer.empty())
		{
		debug("%d: We have processed the whole buffer contents, so we'll flush the buffer.", sockfd);
		buffer.clear();
		}
	    else
		debug("%d: We have processed all input without the need to copy.", sockfd);
	    }
	}

    else if (state == WRITE_ANSWER)
	{
	if (rc < 0)
	    {
	    info("%d: Reading from file (fd %d) returned error '%s': closing connection.",
		 sockfd, filefd, strerror(errno));
	    delete this;
	    }
	else if (rc == 0)
	    {
	    debug("%d: We have copied the complete file. Terminating.", sockfd);
	    delete this;
	    }
	else
	    {
	    debug("%d: Read %d bytes from file fd %d.", sockfd, rc, filefd);
	    ssize_t rc2 = write(sockfd, tmp, rc);
	    if (rc2 < 0)
		{
		if (errno == EAGAIN || errno == EINTR)
		    {
		    debug("%d: Can't write the read data immediately. Using buffer ...", sockfd);
		    buffer.append(tmp, rc);
		    register_network_write_handler();
		    remove_file_read_handler();
		    }
		else
		    {
		    info("%d: Writing to connection returned error '%s'; closing connection.",
			 sockfd, strerror(errno));
		    delete this;
		    }
		}
	    else
		{
		debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc2);
		if (rc2 < rc)
		    {
		    debug("%d: Could not write all %d bytes at once, using buffer for remaining %d bytes.",
			  sockfd, rc, rc-rc2);
		    buffer.append(tmp+rc2, rc-rc2);
		    register_network_write_handler();
		    remove_file_read_handler();
		    }
		}
	    }
	}

    else
	throw logic_error("The internal state of the RequestHandler is messed up.");
    }

bool RequestHandler::process_input(const char* begin, const char* end)
    {
    TRACE();
    string line(begin, end-begin);
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
	return false;
	}
    else
	return true;
    }

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    ssize_t rc = write(sockfd, buffer.data(), buffer.size());
    if (rc < 0)
	{
	info("%d: Writing to connection returned error '%s'; closing connection.",
	     sockfd, strerror(errno));
	delete this;
	return;
	}
    debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
    buffer.erase(0, rc);

    if (state == WRITE_ANSWER)
	{
	if (buffer.empty())
	    {
	    debug("%d: Write buffer is empty. Start the read handler, stop the write handler.", sockfd);
	    register_file_read_handler();
	    remove_network_write_handler();
	    }
	}
    else if (state == TERMINATE)
	{
	if (buffer.empty())
	    {
	    debug("%d: Buffer is empty and we're in TERMINATE state ... terminating.", sockfd);
	    delete this;
	    }
	}
    else
	throw logic_error("The internal state of the RequestHandler is messed up.");
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

void RequestHandler::error_condition(int)
    {
    TRACE();
    info("%d: poll() reported an error condition; terminating connection.", sockfd);
    delete this;
    }

void RequestHandler::pollhup(int)
    {
    TRACE();
    info("%d: poll() says the other end aborted; terminating connection.", sockfd);
    delete this;
    }

// void RequestHandler::read_request()
//     {
//     TRACE();
//     ssize_t rc = read(sockfd, tmp, config->read_block_size);
//     if (rc <= 0)
// 	{
// 	info("%d: Reading from connection returned an error; closing connection.", sockfd);
// 	delete this;
// 	return;
// 	}
//     buffer.append(tmp, rc);
//     while(!buffer.empty())
// 	{
// 	string::size_type pos = buffer.find("\r\n");
// 	if (pos == string::npos)
// 	    break;
//
// 	string line = buffer.substr(0, pos);
// 	buffer.erase(0, pos+2);
// 	if (!line.empty())
// 	    {
// 	    vector<string> vec;
// 	    if (host.empty() && uri.empty() && full_get_port_regex.submatch(line, vec))
// 		{
// 		host = vec[1];
// 		uri  = vec[2];
// 		debug("%d: Got full GET request including port: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
// 		}
// 	    else if (host.empty() && uri.empty() && full_get_regex.submatch(line, vec))
// 		{
// 		host = vec[1];
// 		uri  = vec[2];
// 		debug("%d: Got full GET request: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
// 		}
// 	    else if (uri.empty() && get_regex.submatch(line, vec))
// 		{
// 		uri = vec[1];
// 		debug("%d: Got GET request: uri = '%s'.", sockfd, uri.c_str());
// 		}
// 	    else if (host.empty() && host_port_regex.submatch(line, vec))
// 		{
// 		host = vec[1];
// 		debug("%d: Got Host header including port: host = '%s'.", sockfd, host.c_str());
// 		}
// 	    else if (host.empty() && host_regex.submatch(line, vec))
// 		{
// 		host = vec[1];
// 		debug("%d: Got Host header: host = '%s'.", sockfd, host.c_str());
// 		}
// 	    else
// 		{
// 		debug("%d: Got unknown header '%s'.", sockfd, line.c_str());
// 		}
// 	    }
// 	else
// 	    {
// 	    // We have the complete request. Now we can open the file
// 	    // we'll reply with and continue to the next state.
//
// 	    string filename = config->document_root + "/" + host + uri;
// 	    struct stat sbuf;
// 	    if (stat(filename.c_str(), &sbuf) == -1)
// 		{
// 		error("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
// 		file_not_found(filename);
// 		return;
// 		}
//
// 	    if (S_ISDIR(sbuf.st_mode))
// 		filename.append("/index.html");
//
// 	    filefd = open(filename.c_str(), O_RDONLY | O_NONBLOCK, 0);
// 	    if (filefd == -1)
// 		{
// 		error("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
// 		file_not_found(filename);
// 		return;
// 		}
//
// 	    info("%d: %s GET %s --> %s", sockfd, peer_addr_str, uri.c_str(), filename.c_str());
//
// 	    state = WRITE_ANSWER;
// 	    buffer = "HTTP/1.0 200 OK\r\nContent-Type: ";
// 	    buffer += config->get_content_type(filename);
// 	    buffer += "\r\n\r\n";
// 	    debug("%d: Registering read-handler to read from file (fd %d); going into WRITE_ANSWER state.", sockfd, filefd);
// 	    prop.poll_events  = POLLIN;
// 	    prop.read_timeout = config->file_read_timeout;
// 	    mysched.register_handler(filefd, *this, prop);
//
// 	    break;
// 	    }
// 	}
//     }
//
// void RequestHandler::read_file()
//     {
//     TRACE();
//     ssize_t rc = read(filefd, tmp, config->read_block_size);
//     if (rc < 0)
// 	{
// 	error("%d: Reading from file returned an error; closing connection.", sockfd);
// 	delete this;
// 	}
//     else if (rc == 0)
// 	{
// 	if (!buffer.empty())
// 	    {
// 	    debug("%d: Read whole file; going into TERMINATE state.", sockfd);
// 	    state = TERMINATE;
// 	    prop.poll_events = POLLOUT;
// 	    prop.write_timeout = config->network_write_timeout;
// 	    mysched.register_handler(sockfd, *this, prop);
// 	    mysched.remove_handler(filefd);
// 	    close(filefd);
// 	    filefd = -1;
// 	    }
// 	else
// 	    {
// 	    debug("%d: Read and sent the whole file; terminating.", sockfd);
// 	    delete this;
// 	    }
// 	}
//     else
// 	{
// 	debug("%d: Read %d bytes from file into the buffer.", sockfd, rc);
// 	buffer.append(tmp, rc);
// 	prop.poll_events = POLLOUT;
// 	prop.write_timeout = config->network_write_timeout;
// 	mysched.register_handler(sockfd, *this, prop);
// 	if (buffer.size() <= config->max_buffer_fill_size)
// 	    {
// 	    debug("%d: Buffer contains %d bytes, so we'll continue to read into it.", sockfd, buffer.size());
// 	    prop.poll_events = POLLIN;
// 	    prop.read_timeout = config->file_read_timeout;
// 	    mysched.register_handler(filefd, *this, prop);
// 	    }
// 	else
// 	    {
// 	    debug("%d: Buffer contains %d bytes; we'll empty it before continuing to read from file.", sockfd, buffer.size());
// 	    mysched.remove_handler(filefd);
// 	    }
// 	}
//     }

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
    register_network_write_handler();
    }

void RequestHandler::protocol_error(const string& message)
    {
    TRACE();
    debug("%d: Create protocol-error page for in buffer and write it back to the user.", sockfd);
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
    remove_network_read_handler();
    }

void RequestHandler::register_network_read_handler()
    {
    TRACE();
    if ((network_properties.poll_events & POLLIN) == 0)
	{
	debug("%d: Registering network read handler.", sockfd);
	network_properties.poll_events |= POLLIN;
	mysched.register_handler(sockfd, *this, network_properties);
	}
    else
	debug("%d: Network read handler is registered already.", sockfd);
    }

void RequestHandler::remove_network_read_handler()
    {
    TRACE();
    if ((network_properties.poll_events & POLLIN) != 0)
	{
	debug("%d: Unregistering network read handler.", sockfd);
	network_properties.poll_events &= ~POLLIN;
	mysched.register_handler(sockfd, *this, network_properties);
	}
    else
	debug("%d: Network read handler is not registered already.", sockfd);
    }

void RequestHandler::register_network_write_handler()
    {
    TRACE();
    if ((network_properties.poll_events & POLLOUT) == 0)
	{
	debug("%d: Registering network write handler.", sockfd);
	network_properties.poll_events |= POLLOUT;
	mysched.register_handler(sockfd, *this, network_properties);
	}
    else
	debug("%d: Network write handler is registered already.", sockfd);
    }

void RequestHandler::remove_network_write_handler()
    {
    TRACE();
    if ((network_properties.poll_events & POLLOUT) != 0)
	{
	debug("%d: Unregistering network write handler.", sockfd);
	network_properties.poll_events &= ~POLLOUT;
	mysched.register_handler(sockfd, *this, network_properties);
	}
    else
	debug("%d: Network write handler is not registered already.", sockfd);
    }

void RequestHandler::register_file_read_handler()
    {
    TRACE();
    if ((file_properties.poll_events & POLLIN) == 0)
	{
	debug("%d: Registering file read handler for fd %d.", sockfd, filefd);
	file_properties.poll_events |= POLLIN;
	mysched.register_handler(filefd, *this, file_properties);
	}
    else
	debug("%d: File read handler for fd %d is registered already.", sockfd, filefd);
    }

void RequestHandler::remove_file_read_handler()
    {
    TRACE();
    if ((file_properties.poll_events & POLLIN) != 0)
	{
	debug("%d: Unregistering file read handler for fd %d.", sockfd, filefd);
	file_properties.poll_events &= ~POLLIN;
	mysched.register_handler(filefd, *this, file_properties);
	}
    else
	debug("%d: File read handler for %d is not registered already.", sockfd, filefd);
    }
