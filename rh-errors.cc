/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

void RequestHandler::protocol_error(const char* message)
    {
    TRACE();
    debug(("%d: Protocol error; going into WRITE_REMAINING_DATA state.", sockfd));

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
    connection = "close";
    write_buffer = buf.str();
    returned_status_code = 400;
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::file_not_found(const char* url)
    {
    TRACE();
    debug(("%d: file '%s' not found; going into WRITE_REMAINING_DATA state.", sockfd, url));

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
        << "<p>The requested page <tt>" << url << "</tt> does not exist on this server ...</p>\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    connection = "close";
    write_buffer = buf.str();
    returned_status_code = 404;
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::moved_permanently(const char* url)
    {
    TRACE();
    debug(("%d: Requested page has moved to '%s'; going into WRITE_REMAINING_DATA state.", sockfd, url));

    ostringstream buf;
    buf << "HTTP/1.1 301 Moved Permanently\r\n"
        << "Location: http://" << host;
    if (port != 80)
        buf << ":" << port;
    buf << url << "\r\n"
        << "Content-Type: text/html\r\n";
    connect_header(buf);
    buf << "\r\n"
        << "<html>\r\n"
        << "<head>\r\n"
        << "  <title>Page has moved permanently!</title>\r\n"
        << "</head>\r\n"
        << "<body>\r\n"
        << "The document has moved <a href=\"http://" << host;
    if (port != 80)
        buf << ":" << port;
    buf << url << "\">here</a>.\r\n"
        << "</body>\r\n"
        << "</html>\r\n";
    write_buffer = buf.str();
    returned_status_code = 301;
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::not_modified()
    {
    TRACE();
    debug(("%d: Requested page has not modified; going into WRITE_REMAINING_DATA state.", sockfd));

    ostringstream buf;
    buf << "HTTP/1.1 304 Not Modified\r\n";
    connect_header(buf);
    buf << "\r\n";
    write_buffer = buf.str();
    returned_status_code = 304;
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }
