/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
using namespace std;

void RequestHandler::file_not_found(const string& url)
    {
    TRACE();
    debug("%d: Create file-not-found-page for %s in buffer and write it back to the user.", sockfd, url.c_str());
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
    buffer += url;
    buffer += "</tt> does not exist on this server ...\r\n" \
	"</body>\r\n" \
	"</html>\r\n";
    state = TERMINATE;
    if (write_buffer_or_queue())
	delete this;
    }

void RequestHandler::protocol_error(const string& message)
    {
    TRACE();
    debug("%d: Create protocol-error page in buffer and write it back to the user.", sockfd);
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
    if (write_buffer_or_queue())
	delete this;
    }

void RequestHandler::moved_permanently(const string& url)
    {
    TRACE();
    debug("%d: Create page-has-moved page in buffer and write it back to the user.", sockfd);
    buffer = \
	"HTTP/1.0 301 Moved Permanently\r\n" \
	"Location: ";
    buffer += url;
    buffer += "\r\n" \
	"Content-Type: text/html\r\n" \
	"\r\n" \
	"<html>\r\n" \
	"<head>\r\n" \
	"  <title>Page has moved permanently!</title>\r\n" \
	"</head>\r\n" \
	"<body>\r\n" \
	"The document has moved <a href=\"";
    buffer += url;
    buffer += "\">here</a>.\r\n" \
	"</body>\r\n" \
	"</html>\r\n";
    state = TERMINATE;
    if (write_buffer_or_queue())
	delete this;
    }
