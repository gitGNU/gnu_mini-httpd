/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
#include "request-handler.hh"
#include "log.hh"
using namespace std;

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    try
 	{
        // If there is output waiting in the write buffer, write it.

        ssize_t rc = write(sockfd, write_buffer.data(), write_buffer.size());
        if (rc < 0)
            throw system_error("read() failed");
        else if (rc == 0)
            throw runtime_error("peer closed the connection");
        else
            {
            write_buffer.erase(0, rc);
            bytes_sent += rc;
            }

        // Call state handler.

        while ((this->*state_handlers[state])())
            ;
        }
    catch(const exception& e)
 	{
 	debug(("%d: Caught exception: %s", sockfd, e.what()));
 	delete this;
 	}
    catch(...)
 	{
 	debug(("%d: Caught unknown exception. Terminating.", sockfd));
 	delete this;
 	}
    }
