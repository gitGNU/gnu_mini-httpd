/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "system-error/system-error.hh"
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"

unsigned int RequestHandler::instances = 0;

const RequestHandler::state_fun_t RequestHandler::state_handlers[] =
    {
    &RequestHandler::get_request_line,
    &RequestHandler::get_request_header,
    &RequestHandler::get_request_body,
    &RequestHandler::setup_reply,
    &RequestHandler::copy_file,
    &RequestHandler::write_remaining_data,
    &RequestHandler::terminate
    };

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: mysched(sched), sockfd(fd), filefd(-1)
    {
    TRACE();

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

    // Initialize internal variables.

    init();
    ++instances;
    }

void RequestHandler::init()
    {
    // Init the class' internal variables. May be used in persistent
    // connections.

    state = READ_REQUEST_LINE;
    if (filefd >= 0)
        {
	close(filefd);
        filefd = -1;
        }
    user_agent           = "-";
    referer              = "-";
    returned_status_code = 0;
    returned_object_size = 0;
    port                 = 0;
    minor_version        = 0;
    major_version        = 0;
    if_modified_since    = 0;

    // Register ourselves with the scheduler.

    scheduler::handler_properties prop;
    prop.poll_events   = POLLIN;
    prop.read_timeout  = config->network_read_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

RequestHandler::~RequestHandler()
    {
    TRACE();

    --instances;

    mysched.remove_handler(sockfd);
    close(sockfd);
    if (filefd >= 0)
	close(filefd);
    }
