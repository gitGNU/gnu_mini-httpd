/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <cstdio>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "request-handler.hh"
#include "config.hh"
#include "urldecode.hh"
using namespace std;

namespace
    {
    string time_to_ascii(time_t t)
        {
        char buffer[1024];
        size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
        if (len == 0 || len >= sizeof(buffer))
            throw std::logic_error("strftime() failed because an internal buffer is too small!");
        return buffer;
        }
    }

void RequestHandler::fd_is_readable(int)
    {
    TRACE();
    try
	{
	if (state != READ_REQUEST)
	    throw logic_error("Internal Error: RequestHandler::fd_is_readable() called in wrong state!");

	if (buffer_end == data_end)
	    {
	    size_t offset = data - buffer;
	    if (offset == 0)
		throw runtime_error("The I/O buffer is too small to hold a single line! Aborting.");
	    debug(("%d: The first %d bytes of the buffer are unused. Moving data down to make space.",
		  sockfd, offset));
	    memmove(buffer, data, data_end - data);
	    data     -= offset;
	    data_end -= offset;
	    }

	debug(("%d: Internal buffer is %d bytes large, has %d bytes content, the first %d bytes are unused, " \
	      "and %d bytes are free at the end.",
	      sockfd, buffer_end - buffer, data_end - data, data - buffer, buffer_end - data_end));

	size_t rc = myread(sockfd, data_end, buffer_end - data_end);
	bytes_received += rc;
	++read_calls;
	debug(("%d: Read %d bytes from network peer.", sockfd, rc));
	if (rc == 0)
	    {
	    debug(("%d: Peer has shut down his channel of the connection.", sockfd));
	    delete this;
	    return;
	    }
	else
	    {
	    data_end += rc;
	    *data_end = '\0';
	    }

	char* eol;
	do
	    {
	    eol = strstr(data, "\r\n");
	    if (eol)
		{
		if (process_input(data, eol) == true)
		    {
		    debug(("%d: Read request is complete.", sockfd));

		    // Do we have all information we need? If not,
		    // report a protocol error.

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

                    // Decode any special characters that the url
                    // might contain.

                    debug(("Original url is '%s'.", url.c_str()));
                    urldecode(url);
                    debug(("Decoded url is '%s'.", url.c_str()));

		    // Construct the actual file name associated with
		    // the hostname and URL, then check whether the
		    // file exists. If not, report an error. If the
		    // URL points to a directory, send a redirect
		    // reply pointing to the "index.html" file in that
		    // directory.

		    string filename = config->document_root;
		    filename += "/" + host + url;
		    struct stat sbuf;
		    if (stat(filename.c_str(), &sbuf) == -1)
			{
			info("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
			file_not_found(url.c_str());
			return;
			}

		    if (S_ISDIR(sbuf.st_mode))
			{
			if (url[url.size()-1] == '/')
			    moved_permanently((url + "index.html").c_str());
			else
			    moved_permanently((url + "/index.html").c_str());
			return;
			}

                    filefd = open(filename.c_str(), O_RDONLY, 0);
                    if (filefd == -1)
                        {
                        info("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
                        file_not_found(filename.c_str());
                        return;
                        }
                    state = COPY_FILE;
                    debug(("%d: Retrieved file from disk: Going into COPY_FILE state.", sockfd));

		    int len = snprintf(buffer, buffer_end - buffer,
				       "HTTP/1.0 200 OK\r\n"     \
				       "Content-Type: %s\r\n"    \
				       "Content-Length: %ld\r\n" \
				       "Date: %s\r\n" \
				       "Last-Modified: %s\r\n" \
				       "\r\n",
				       config->get_content_type(filename.c_str()),
                                       sbuf.st_size,
                                       time_to_ascii(time(0)).c_str(),
                                       time_to_ascii(sbuf.st_mtime).c_str());
		    if (len > 0 && len <= buffer_end - buffer)
			{
			data     = buffer;
			data_end = data + len;
			}
		    else
			throw runtime_error("Internal error: Our internal buffer is too small!");

		    scheduler::handler_properties prop;
		    prop.poll_events   = POLLOUT;
		    prop.write_timeout = config->network_write_timeout;
		    mysched.register_handler(sockfd, *this, prop);
		    return;
		    }
		data = eol + 2;
		}
	    else
		debug(("%d: Our input buffer does not contain any more complete lines.", sockfd));
	    }
	while (eol);
	debug(("%d: %d bytes could not be processed.", sockfd, data_end - data));
	if (data == data_end)
	    data = data_end = buffer;
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
