/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "log.hh"
using namespace std;

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    try
 	{
        // If there is output waiting in the write buffer, write it.

        if (state != TERMINATE && !write_buffer.empty())
            {
            ssize_t rc = write(sockfd, write_buffer.data(), write_buffer.size());
            if (rc < 0)
                throw system_error("read() failed");
            else if (rc == 0)
                throw runtime_error("peer closed the connection");
            else
                write_buffer.erase(0, rc);
            }

        // Call state handler.

        call_state_handler();
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
