/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
using namespace std;

void RequestHandler::file_not_found(const string& file)
    {
    TRACE();
    debug("%d: Create file-not-found-page for %s in buffer and write it back to the user.", sockfd, file.c_str());
    buffer = \
	"HTTP/1.0 404 Not Found\r\n" \
	"Content-Type: text/html\r\n" \
	"\r\n" \
	"<html>\r\n" \
	"<head>\r\n" \
	"  <title>Page does not exist!</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"The requested page <tt>";
    buffer += file;
    buffer += "</tt> does not exist on this server ...\r\n" \
	"</body>\r\n" \
	"</html>\r\n";
    state = TERMINATE;
    size_t rc = mywrite(sockfd, buffer.data(), buffer.size());
    debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
    if (rc < buffer.size())
	{
	debug("%d: Could not write all %d bytes at once, using buffer for remaining %d bytes.",
	      sockfd, buffer.size(), buffer.size()-rc);
	buffer.erase(0, rc);
	register_network_write_handler();
	}
    else
	{
	debug("%d: Wrote the whole page. Terminating.", sockfd);
	delete this;
	}
    }

void RequestHandler::protocol_error(const string& message)
    {
    TRACE();
    debug("%d: Create protocol-error page for in buffer and write it back to the user.", sockfd);
    buffer = \
	"HTTP/1.0 400 Bad Request\r\n" \
	"Content-Type: text/html\r\n" \
	"\r\n" \
	"<html>\r\n" \
	"<head>\r\n" \
	"  <title>Bad HTTP Request!</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"The HTTP request received by this server was incorrect:<p>\r\n" \
	"<blockquote>\r\n";
    buffer += message;
    buffer += "</blockquote>\r\n" \
	"</body>\r\n" \
	"</html>\r\n";
    state = TERMINATE;
    size_t rc = mywrite(sockfd, buffer.data(), buffer.size());
    debug("%d: Wrote %d bytes from buffer to peer.", sockfd, rc);
    if (rc < buffer.size())
	{
	debug("%d: Could not write all %d bytes at once, using buffer for remaining %d bytes.",
	      sockfd, buffer.size(), buffer.size()-rc);
	buffer.erase(0, rc);
	register_network_write_handler();
	}
    else
	{
	debug("%d: Wrote the whole page. Terminating.", sockfd);
	delete this;
	}
    }
