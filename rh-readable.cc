/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "request-handler.hh"
#include "config.hh"
using namespace std;

void RequestHandler::fd_is_readable(int fd)
    {
    try
	{
	TRACE();
	size_t rc = myread(fd, tmp, config->read_block_size);

	if (state == READ_REQUEST)
	    {
	    if (rc == 0)
		{
		info("%d: Connection was dropped by the peer. Aborting.", sockfd);
		delete this;
		return;
		}
	    debug("%d: Read %d bytes from network peer.", sockfd, rc);
	    const char* begin;
	    const char* current;
	    const char* end;
	    const char* eol;
	    if (buffer.empty())
		{
		debug("%d: Our input buffer is empty, we can handle the input without copying.", sockfd);
		begin = tmp;
		end   = tmp + rc;
		}
	    else
		{
		debug("%d: Our input buffer contains %d bytes, we have to copy.", sockfd, buffer.size());
		buffer.append(tmp, rc);
		begin = buffer.c_str();
		end   = buffer.c_str() + buffer.size();
		}
	    current = begin;
	    do
		{
		eol = strstr(current, "\r\n");
		if (eol)
		    {
		    if (process_input(current, eol) == true)
			{
			debug("%d: Read request is complete.", sockfd);
			remove_network_read_handler();
			if (host.empty())
			    {
			    protocol_error("The HTTP request did not contain a hostname.\r\n" \
					   "In order to fulfill the request, we need the\r\n" \
					   "hostname either in the URL or via the <tt>Host:</tt>\r\n" \
					   "header.\r\n");
			    return;
			    }
			else if (url.empty())
			    {
			    protocol_error("The HTTP request did not contain an URL!\r\n");
			    return;
			    }

			string filename = config->document_root + "/" + host + url;
			struct stat sbuf;
			if (stat(filename.c_str(), &sbuf) == -1)
			    {
			    info("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
			    file_not_found(url);
			    return;
			    }

			if (S_ISDIR(sbuf.st_mode))
			    filename.append("/index.html");

			filefd = open(filename.c_str(), O_RDONLY | O_NONBLOCK, 0);
			if (filefd == -1)
			    {
			    info("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
			    file_not_found(filename);
			    return;
			    }
			info("%d: %s GET %s --> %s", sockfd, peer_addr_str, url.c_str(), filename.c_str());
			state = WRITE_ANSWER;
			buffer = "HTTP/1.0 200 OK\r\nContent-Type: ";
			buffer += config->get_content_type(filename);
			buffer += "\r\n\r\n";
			if (write_buffer_or_queue())
			    {
			    buffer.clear();
			    register_file_read_handler();
			    }
			return;
			}
		    current = eol + 2;
		    }
		else
		    debug("%d: Our input buffer does not contain any more complete lines.", sockfd);
		}
	    while (eol);
	    if (current < end)
		{
		if (buffer.empty())
		    {
		    debug("%d: Append remaining %d input bytes to the (empty) buffer.", sockfd, end-current);
		    buffer.append(current, end-current);
		    }
		else
		    {
		    debug("%d: Remove the processed %d input bytes from the buffer.", sockfd, current-begin);
		    buffer.erase(0, current-begin);
		    debug("%d: Buffer contains %d bytes after removal.", sockfd, buffer.size());
		    }
		}
	    else
		{
		if (!buffer.empty())
		    {
		    debug("%d: We have processed the whole buffer contents, so we'll flush the buffer.", sockfd);
		    buffer.clear();
		    }
		else
		    debug("%d: We have processed all input without the need to copy.", sockfd);
		}
	    }

	else if (state == WRITE_ANSWER)
	    {
	    if (rc == 0)
		{
		debug("%d: We have copied the complete file. Terminating.", sockfd);
		delete this;
		}
	    else
		{
		debug("%d: Read %d bytes from file fd %d.", sockfd, rc, filefd);
		size_t rc2 = mywrite(sockfd, tmp, rc);
		debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc2);
		if (rc2 < rc)
		    {
		    debug("%d: Could not write all %d bytes at once, using buffer for remaining %d bytes.",
			  sockfd, rc, rc-rc2);
		    buffer.append(tmp+rc2, rc-rc2);
		    register_network_write_handler();
		    remove_file_read_handler();
		    }
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
