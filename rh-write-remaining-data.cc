/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::write_remaining_data()
    {
    TRACE();

    if (write_buffer.empty())
        {
        log_access();
        if (is_persistent_connection())
            {
            debug(("%d: Persistent connection; starting over in GET_REQUEST_LINE state.", sockfd));
            init();
            return true;
            }
        else
            {
            debug(("%d: Nothing left to write; going into terminate state.", sockfd));
            state = TERMINATE;
            if (shutdown(sockfd, SHUT_RDWR) == -1)
                delete this;
            }
        }
    return false;
    }
