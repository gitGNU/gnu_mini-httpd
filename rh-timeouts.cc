/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "log.hh"

void RequestHandler::read_timeout(int)
    {
    TRACE();
    debug(("%d: Read timout; terminating connection.", sockfd));
    delete this;
    }

void RequestHandler::write_timeout(int)
    {
    TRACE();
    debug(("%d: Write timout; terminating connection.", sockfd));
    delete this;
    }

void RequestHandler::error_condition(int)
    {
    TRACE();
    debug(("%d: poll() reported an error condition; terminating connection.", sockfd));
    delete this;
    }

void RequestHandler::pollhup(int)
    {
    TRACE();
    if (state == TERMINATE)
        call_state_handler();
    else
        {
        debug(("%d: poll() says the other end aborted; terminating connection.", sockfd));
        delete this;
        }
    }
