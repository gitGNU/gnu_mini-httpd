/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::flush_buffer_and_reset()
    {
    TRACE();

    if (write_buffer.empty())
        {
        debug(("%d: Request is finished; restarting handler.", sockfd));
        log_access();
        reset();
        return true;
        }
    else
        return false;
    }

bool RequestHandler::flush_buffer_and_terminate()
    {
    TRACE();

    if (write_buffer.empty())
        {
        log_access();
        debug(("%d: Request is finished; going into TERMINATE state.", sockfd));
        state = TERMINATE;
        if (shutdown(sockfd, SHUT_RDWR) == -1)
            delete this;
        }
    return false;
    }
