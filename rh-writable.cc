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

        debug(("%d: write_buffer contains %d characters.", sockfd, write_buffer.size()));

        ssize_t rc = write(sockfd, write_buffer.data(), write_buffer.size());
        debug(("%d: write() returned %d.", sockfd, rc));
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



#if 0
	if (state == COPY_FILE && data_end == data)
	    {
	    // Buffer is empty. Read new data from file before
	    // continuing.

	    size_t rc = myread(filefd, buffer, buffer_end - buffer);
	    debug(("%d: Read %d bytes from file.", sockfd, rc));
	    data     = buffer;
	    data_end = data + rc;
	    if (rc == 0)
		{
		debug(("%d: The complete file is copied: going into WRITE_REMAINING_DATA state.", sockfd));
		state = WRITE_REMAINING_DATA;
		close(filefd);
		filefd = -1;
		}
	    }

	if ((state == COPY_FILE || state == WRITE_REMAINING_DATA) && data < data_end)
	    {
	    size_t rc = mywrite(sockfd, data, data_end - data);
	    bytes_sent += rc;
	    data       += rc;
	    ++write_calls;
	    debug(("%d: Wrote %d bytes from buffer to peer.", sockfd, rc));
	    }

	if (state == WRITE_REMAINING_DATA && data == data_end)
	    {
	    debug(("%d: Done. Going into TERMINATE state.", sockfd));
	    state = TERMINATE;
	    if (shutdown(sockfd, SHUT_RDWR) == -1)
		delete this;
	    return;
	    }

	if (state == TERMINATE)
	    delete this;
 	}
#endif
