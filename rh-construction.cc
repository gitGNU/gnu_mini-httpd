/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "request-handler.hh"
#include "system-error.hh"
#include "config.hh"

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
