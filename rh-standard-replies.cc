/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "escape-html-specials.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

void RequestHandler::make_standard_header(ostringstream& os)
    {


    }

void RequestHandler::protocol_error(const string& message)
    {
    TRACE();
    debug(("%d: Protocol error; going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd));

    ostringstream buf;
    buf << "HTTP/1.1 400 Bad Request\r\n"
        << "Content-Type: text/html\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << "<html>\r\n"
        << "<head>\r\n"
        << "  <title>Bad HTTP Request!</title>\r\n"
        << "</head>\r\n"
        << "<body>\r\n"
        << "The HTTP request received by this server was incorrect:<p>\r\n"
        << "<blockquote>\r\n"
        << message
        << "</blockquote>\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    write_buffer = buf.str();
    request.status_code = 400;
    request.object_size = 0;
    state = FLUSH_BUFFER_AND_TERMINATE;
    go_to_write_mode();
    }

void RequestHandler::file_not_found()
    {
    TRACE();
    //debug(("%d: file '%s' not found; going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd, url));

    ostringstream buf;
    buf << "HTTP/1.1 404 Not Found\r\n"
        << "Content-Type: text/html\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << "<html>\r\n"
        << "<head>\r\n"
        << "  <title>Page does not exist!</title>\r\n"
        << "</head>\r\n"
        << "<body>\r\n"
        << "<p>The requested page <tt></tt> does not exist on this server ...</p>\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    write_buffer = buf.str();
    request.status_code = 404;
    request.object_size = 0;
    state = FLUSH_BUFFER_AND_TERMINATE;
    go_to_write_mode();
    }

void RequestHandler::moved_permanently(const string& path)
    {
    TRACE();
    //debug(("%d: Requested page has moved to '%s'; going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd, url));

    ostringstream buf;
    buf << "HTTP/1.1 301 Moved Permanently\r\n"
        << "Location: http://" << request.host;
    buf << path << "\r\n"
        << "Content-Type: text/html\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << "<html>\r\n"
        << "<head>\r\n"
        << "  <title>Page has moved permanently!</title>\r\n"
        << "</head>\r\n"
        << "<body>\r\n"
        << "The document has moved <a href=\"http://" << request.host;
    if (request.port != 80)
        buf << ":" << request.port;
    buf << path << "\">here</a>.\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    write_buffer        = buf.str();
    request.status_code = 301;
    request.object_size = 0;
    state = FLUSH_BUFFER_AND_TERMINATE;
    go_to_write_mode();
    }

void RequestHandler::not_modified()
    {
    TRACE();
    debug(("%d: Requested page was not modified; going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd));

    ostringstream buf;
    buf << "HTTP/1.1 304 Not Modified\r\n";
    buf << "\r\n";
    write_buffer = buf.str();
    request.status_code = 304;
    state = FLUSH_BUFFER_AND_TERMINATE;
    go_to_write_mode();
    }
