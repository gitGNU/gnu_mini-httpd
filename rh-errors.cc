/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <cstdio>
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

void RequestHandler::protocol_error(const char* message)
    {
    TRACE();
    debug(("%d: Protocol error; going into WRITE_REMAINING_DATA state.", sockfd));
    char buf[4096];
    snprintf(buf, sizeof(buf),
             "HTTP/1.1 400 Bad Request\r\n"                                   \
             "Content-Type: text/html\r\n"                                    \
             "%s"                                                             \
             "\r\n"                                                           \
             "<html>\r\n"                                                     \
             "<head>\r\n"                                                     \
             "  <title>Bad HTTP Request!</title>\r\n"                         \
             "</head>\r\n"                                                    \
             "<body>\r\n"                                                     \
             "The HTTP request received by this server was incorrect:<p>\r\n" \
             "<blockquote>\r\n"                                               \
             "%s"                                                             \
             "</blockquote>\r\n"                                              \
             "</body>\r\n"                                                    \
             "</html>\r\n",
             make_connection_header(), message);
    write_buffer = buf;
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
    char buf[4096];
    snprintf(buf, sizeof(buf),
             "HTTP/1.1 404 Not Found\r\n"                                             \
             "Content-Type: text/html\r\n"                                            \
             "%s"                                                                     \
             "\r\n"                                                                   \
             "<html>\r\n"                                                             \
             "<head>\r\n"                                                             \
             "  <title>Page does not exist!</title>\r\n"                              \
             "</head>\r\n"                                                            \
             "<body>\r\n"                                                             \
             "The requested page <tt>%s</tt> does not exist on this server ...\r\n"   \
             "</body>\r\n"                                                            \
             "</html>\r\n",
             make_connection_header(), url);
    write_buffer = buf;
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
    char buf[4096];
    snprintf(buf, sizeof(buf),
             "HTTP/1.1 301 Moved Permanently\r\n"                             \
             "Location: %s\r\n"                                               \
             "Content-Type: text/html\r\n"                                    \
             "%s"                                                             \
             "\r\n"                                                           \
             "<html>\r\n"                                                     \
             "<head>\r\n"                                                     \
             "  <title>Page has moved permanently!</title>\r\n"               \
             "</head>\r\n"                                                    \
             "<body>\r\n"                                                     \
             "The document has moved <a href=\"%s\">here</a>.\r\n"            \
             "</body>\r\n"                                                    \
             "</html>\r\n",
             url, make_connection_header(), url);
    write_buffer = buf;
    returned_status_code = 301;
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }
