/*
 * Copyright (c) 2001 by Peter Simons <simons@cryp.to>.
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
            debug(("%d: Connection is persistent; restarting.", sockfd));
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
