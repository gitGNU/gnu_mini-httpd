/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace std;
#include "httpd.hh"
#include "system-error.hh"

const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: mysched(sched), sockfd(fd)
    {
    // Store the peer's address as ASCII string; we'll need that
    // again later.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_addr_str, sizeof(peer_addr_str)) == NULL)
	strcpy(peer_addr_str, "unknown");

    // Log the new connection.

    cerr << "Accepting new connection on file descriptor " << fd << "; "
	 << "peer is " << peer_addr_str << ":" << ntohs(sin.sin_port)
	 << ".\n";

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
    prop.read_timeout  = 30;
    prop.write_timeout = 0;
    mysched.register_handler(sockfd, *this, prop);
    }

RequestHandler::~RequestHandler()
    {
    mysched.remove_handler(sockfd);
    close(sockfd);
    }

void RequestHandler::fd_is_readable(int)
    {
    char buf[1024];
    ssize_t rc = read(sockfd, buf, sizeof(buf));
    if (rc <= 0)
	{
	cerr << sockfd << ": Connection broke down!\n";
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
	    if (full_get_regex.submatch(line, vec))
		{
		cerr << sockfd << ": Host " << vec[1] << "; GET " << vec[2]
		     << " HTTP" << vec[3] << "." << vec[4]
		     << "\n";
		if (host.empty() && uri.empty())
		    {
		    host = vec[1];
		    uri  = vec[2];
		    }
		}
	    else if (get_regex.submatch(line, vec))
		{
		cerr << sockfd << ": GET " << vec[1] << " HTTP"
		     << vec[2] << "." << vec[3]
		     << "\n";
		if (uri.empty())
		    uri = vec[1];
		}
	    else if (host_regex.submatch(line, vec))
		{
		cerr << sockfd << ": Host " << vec[1] << "\n";
		if (host.empty())
		    host = vec[1];
		}
	    else
		cerr << sockfd << ": Unknown line: " << line << "\n";
	    }
	else
	    {
	    cerr << sockfd << ": Got complete request.\n";
	    string filename = string(DOCUMENT_ROOT) + "/" + host + uri;
	    cout << sockfd << ": Getting " << filename << "\n";
	    }
	}
    }

void RequestHandler::fd_is_writable(int)
    {
    }

void RequestHandler::read_timeout(int)
    {
    cerr << sockfd << ": Read timout; terminating connection.\n";
    delete this;
    }

void RequestHandler::write_timeout(int)
    {
    }
