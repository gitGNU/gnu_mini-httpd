/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

void RequestHandler::fd_is_readable(int)
    {
    TRACE();
    try
	{
        // Protect against flooding.

        if (read_buffer.size() > config->max_line_length)
            {
            protocol_error("This server won't process excessively long\r\n" \
                           "request header lines.\r\n");
            return;
            }

        // Read sockfd stuff into the buffer.

        char buf[4096];
        ssize_t rc = read(sockfd, buf, sizeof(buf));
        debug(("%d: read() returned %d.", sockfd, rc));
        if (rc < 0)
            throw system_error("read() failed");
        else if (rc == 0)
            throw runtime_error("peer closed the connection");
        else
            {
            read_buffer.append(buf, rc);
            bytes_received += rc;
            }

        // Call the state handler.

        while((this->*state_handlers[state])())
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
