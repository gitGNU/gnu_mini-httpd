/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
using namespace std;

void RequestHandler::fd_is_writable(int)
    {
    try
	{
	TRACE();
	size_t rc = mywrite(sockfd, buffer.data(), buffer.size());
	debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
	buffer.erase(0, rc);

	if (state == WRITE_ANSWER)
	    {
	    if (buffer.empty())
		{
		debug("%d: Write buffer is empty. Start the read handler, stop the write handler.", sockfd);
		register_file_read_handler();
		remove_network_write_handler();
		}
	    }
	else if (state == TERMINATE)
	    {
	    if (buffer.empty())
		{
		debug("%d: Buffer is empty and we're in TERMINATE state ... terminating.", sockfd);
		delete this;
		}
	    }
	else
	    throw logic_error("The internal state of the RequestHandler is messed up.");
	}
    catch(const exception& e)
	{
	info("%d: Caught exception: %s", sockfd, e.what());
	delete this;
	return;
	}
    catch(...)
	{
	info("%d: Caught unknown exception. Terminating.", sockfd);
	delete this;
	return;
	}
    }
