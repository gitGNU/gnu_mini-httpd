/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"

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
