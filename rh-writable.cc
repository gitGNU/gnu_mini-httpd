/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <unistd.h>
#include "request-handler.hh"
using namespace std;

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    try
 	{
	if (state == COPY_FILE && data_end == data)
	    {
	    // Buffer is empty. Read new data from file before
	    // continuing.

	    size_t rc = myread(filefd, buffer, buffer_end - buffer);
	    debug("%d: Read %d bytes from file.", sockfd, rc);
	    data     = buffer;
	    data_end = data + rc;
	    if (rc == 0)
		{
		debug("%d: The complete file is copied: going into WRITE_REMAINING_DATA state.", sockfd);
		state = WRITE_REMAINING_DATA;
		close(filefd);
		filefd = -1;
		}
	    }

	if ((state == COPY_FILE || state == WRITE_CACHED_FILE || state == WRITE_REMAINING_DATA) && data < data_end)
	    {
	    size_t rc = mywrite(sockfd, data, data_end - data);
	    data += rc;
	    debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
	    }

	if (state == WRITE_CACHED_FILE && data == data_end)
	    {
	    data     = const_cast<char*>(cached_file.data.get());
	    data_end = data + cached_file.data_len;
	    state    = WRITE_REMAINING_DATA;
	    debug("%d: Wrote header successfully: Going into WRITE_REMAINING_DATA state.", sockfd);
	    }

	if (state == WRITE_REMAINING_DATA && data == data_end)
	    {
	    debug("%d: Done. Going into TERMINATE state.", sockfd);
	    state = TERMINATE;
	    if (shutdown(sockfd, SHUT_RDWR) == -1)
		delete this;
	    return;
	    }

	if (state == TERMINATE)
	    delete this;
 	}
    catch(const exception& e)
 	{
 	debug("%d: Caught exception: %s", sockfd, e.what());
 	delete this;
 	}
    catch(...)
 	{
 	debug("%d: Caught unknown exception. Terminating.", sockfd);
 	delete this;
 	}
    }
