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

unsigned int RequestHandler::instances = 0;

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: state(READ_REQUEST), mysched(sched), sockfd(fd), filefd(-1),
	  bytes_sent(0), bytes_received(0), read_calls(0), write_calls(0)
    {
    TRACE();

    // Get the current time so that we can calculate our run-time
    // later.

    gettimeofday(&connection_start, 0);

    // Initialize the internal buffer.

    buffer     = new char[config->io_buffer_size + 1];
    buffer_end = buffer + config->io_buffer_size;
    data       = buffer;
    data_end   = buffer;

    try
	{
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

	scheduler::handler_properties prop;
	prop.poll_events   = POLLIN;
	prop.read_timeout  = config->network_read_timeout;
	mysched.register_handler(sockfd, *this, prop);
	if (instances++ == config->hard_poll_interval_threshold)
	    {
	    debug("We have more than %d connections: Switching to a hard poll interval of %d seconds.",
		  config->hard_poll_interval_threshold, config->hard_poll_interval);
	    mysched.set_poll_interval(config->hard_poll_interval * 1000);
	    }
	}
    catch(...)
	{
	delete[] buffer;
	throw;
	}
    }

RequestHandler::~RequestHandler()
    {
    TRACE();

    timeval now, runtime;
    gettimeofday(&now, 0);
    timersub(&now, &connection_start, &runtime);

    log_access((state == TERMINATE), host.c_str(), url.c_str(), peer_addr_str, sockfd,
	       runtime, bytes_received, bytes_sent, read_calls, write_calls);

    if (--instances == config->hard_poll_interval_threshold)
	{
	debug("We have %d active connections: Switching back to an accurate poll interval.",
	      config->hard_poll_interval_threshold);
	mysched.use_accurate_poll_interval();
	}
    mysched.remove_handler(sockfd);
    close(sockfd);
    if (filefd >= 0)
	close(filefd);
    if (buffer != 0)
	delete[] buffer;
    }
