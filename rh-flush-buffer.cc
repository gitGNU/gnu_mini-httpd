/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::flush_buffer()
    {
    TRACE();

    if (write_buffer.empty())
        {
        log_access();
        if (use_persistent_connection)
            {
            reset();
            return true;
            }
        else
            {
            state = TERMINATE;
            if (shutdown(sockfd, SHUT_RDWR) == -1)
                delete this;
            }
        }
    return false;
    }
