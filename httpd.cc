/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace std;
#include "httpd.hh"
#include "system-error.hh"
#include "log.hh"

const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: mysched(sched), sockfd(fd)
    {
    TRACE();

    // Store the peer's address as ASCII string; we'll need that
    // again later.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_addr_str, sizeof(peer_addr_str)) == NULL)
	strcpy(peer_addr_str, "unknown");
    log(DEBUG, "Peer's address is %s.", peer_addr_str);

    // Set socket parameters.

    int true_flag = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &true_flag, sizeof(int)) == -1)
	throw system_error("Can't set KEEPALIVE mode");
    log(DEBUG, "Set socket %d to KEEPALIVE mode.", sockfd);

    linger ling;
    ling.l_onoff  = 0;
    ling.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
	throw system_error("Can't switch LINGER mode off");
    log(DEBUG, "Set socket %d to non-LINGER mode.", sockfd);

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
	throw system_error("Can set non-blocking mode");
    log(DEBUG, "Set socket %d to non-blocking mode.", sockfd);

    // Register ourselves with the scheduler.

    prop.poll_events   = POLLIN;
    prop.read_timeout  = 30;
    prop.write_timeout = 0;
    log(DEBUG, "About to Register handler for socket %d.", sockfd);
    mysched.register_handler(sockfd, *this, prop);
    log(DEBUG, "Registered handler for socket %d.", sockfd);
    }

RequestHandler::~RequestHandler()
    {
    TRACE();
    mysched.remove_handler(sockfd);
    close(sockfd);
    }

void RequestHandler::fd_is_readable(int)
    {
    TRACE();
    char buf[1024];
    ssize_t rc = read(sockfd, buf, sizeof(buf));
    if (rc <= 0)
	{
	log(ERROR, "%d: Connection broke down!", sockfd);
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
	    else
		;		// Unknown contents of 'line'.
	    }
	else
	    {
	    string filename = string(DOCUMENT_ROOT) + "/" + host + uri;
	    log(INFO, "%d: %s GET %s --> %s", sockfd, peer_addr_str, uri.c_str(), filename.c_str());

	    buffer = \
		"HTTP/1.0 200 OK\r\n" \
		"Content-Type: text/html\r\n" \
		"\r\n" \
		"<html>\r\n" \
		"<head>\r\n" \
		"  <title>Virtual Page</title>\r\n" \
		"</head>\r\n" \
		"<body>\r\n" \
		"This is not a real page ...\r\n" \
		"</body>\r\n" \
		"</html>\r\n";

	    prop.poll_events   = POLLOUT;
	    prop.read_timeout  = 0;
	    prop.write_timeout = 30;
	    mysched.register_handler(sockfd, *this, prop);
	    break;
	    }
	}
    }

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    ssize_t rc = write(sockfd, buffer.c_str(), buffer.size());
    if (rc <= 0)
	{
	log(ERROR, "%d: Connection broke down!", sockfd);
	delete this;
	return;
	}

    buffer.erase(0, rc);
    if (buffer.empty())
	delete this;		// done
    }

void RequestHandler::read_timeout(int)
    {
    TRACE();
    log(ERROR, "%d: Read timout; terminating connection.", sockfd);
    delete this;
    }

void RequestHandler::write_timeout(int)
    {
    TRACE();
    log(ERROR, "%d: Write timout; terminating connection.", sockfd);
    delete this;
    }
