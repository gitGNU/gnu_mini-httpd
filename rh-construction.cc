/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "ScopeGuard/ScopeGuard.hh"
#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

unsigned int RequestHandler::instances = 0;

const RequestHandler::state_fun_t RequestHandler::state_handlers[] =
    {
    &RequestHandler::get_request_line,
    &RequestHandler::get_request_header,
    &RequestHandler::get_request_body,
    &RequestHandler::setup_reply,
    &RequestHandler::copy_file,
    &RequestHandler::flush_buffer_and_reset,
    &RequestHandler::flush_buffer_and_terminate,
    &RequestHandler::terminate
    };

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
	: mysched(sched), sockfd(fd), filefd(-1)
    {
    TRACE();

    // Store the peer's address as ASCII string.

    if (inet_ntop(AF_INET, &sin.sin_addr, peer_address, sizeof(peer_address)) == 0)
        throw system_error("inet_ntop() failed");

    // Set socket parameters.

    linger ling;
    ling.l_onoff  = 0;
    ling.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
        throw system_error("Can't switch LINGER mode off");

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
        throw system_error("Can set non-blocking mode");

    // Allocate our line buffer for reading.

    line_buffer = static_cast<char*>(malloc(config->max_line_length));
    if (line_buffer == 0)
        throw runtime_error("Failed to allocate memory for the internal line buffer.");
    ScopeGuard sg_line_buffer = MakeGuard(free, line_buffer);

    // Initialize internal variables.

    reset();
    debug(("%d: Accepted new connection from peer '%s'.", sockfd, peer_address));
    ++instances;
    sg_line_buffer.Dismiss();
    }

void RequestHandler::reset()
    {
    TRACE();

    // Freshen up the internal variables.

    state = READ_REQUEST_LINE;

    if (filefd >= 0)
        {
	close(filefd);
        filefd = -1;
        }

    request = HTTPRequest();
    request.start_up_time = time(0);

    go_to_read_mode();
    }

RequestHandler::~RequestHandler()
    {
    TRACE();

    debug(("%d: Closing connection with peer '%s'.", sockfd, peer_address));

    --instances;

    mysched.remove_handler(sockfd);

    close(sockfd);

    if (filefd >= 0)
	close(filefd);

    free(line_buffer);
    }
