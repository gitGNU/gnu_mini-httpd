/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "config.hh"
using namespace std;

void RequestHandler::file_not_found(const char* url)
    {
    TRACE();
    debug("%d: Create file-not-found-page for %s in buffer and write it back to the user.", sockfd, url);
    int len = snprintf(buffer, buffer_end - buffer,
                       "HTTP/1.0 404 Not Found\r\n"                                             \
                       "Content-Type: text/html\r\n"                                            \
                       "\r\n"                                                                   \
                       "<html>\r\n"                                                             \
                       "<head>\r\n"                                                             \
                       "  <title>Page does not exist!</title>\r\n"                              \
                       "</head>\r\n"                                                            \
                       "<body>\r\n"                                                             \
                       "The requested page <tt>%s</tt> does not exist on this server ...\r\n"   \
                       "</body>\r\n"                                                            \
                       "</html>\r\n",
                       url);
    debug("%d: The snprintf() used %d bytes in the buffer.", sockfd, len);
    if (len > 0 && len <= buffer_end - buffer)
        {
        data     = buffer;
        data_end = data + len;
        }
    else
        throw runtime_error("Internal error: Our internal buffer is too small!");
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::protocol_error(const char* message)
    {
    TRACE();
    debug("%d: Create protocol-error page in buffer and write it back to the user.", sockfd);
    int len = snprintf(buffer, buffer_end - buffer,
                       "HTTP/1.0 400 Bad Request\r\n"                                   \
                       "Content-Type: text/html\r\n"                                    \
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
                       message);
    debug("%d: The snprintf() used %d bytes in the buffer.", sockfd, len);
    if (len > 0 && len <= buffer_end - buffer)
        {
        data     = buffer;
        data_end = data + len;
        }
    else
        throw runtime_error("Internal error: Our internal buffer is too small!");
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::moved_permanently(const char* url)
    {
    TRACE();
    debug("%d: Create page-has-moved page to URL %s in buffer and write it back to the user.", sockfd, url);
    int len = snprintf(buffer, buffer_end - buffer,
                       "HTTP/1.0 301 Moved Permanently\r\n"                             \
                       "Location: %s\r\n"                                               \
                       "Content-Type: text/html\r\n"                                    \
                       "\r\n"                                                           \
                       "<html>\r\n"                                                     \
                       "<head>\r\n"                                                     \
                       "  <title>Page has moved permanently!</title>\r\n"               \
                       "</head>\r\n"                                                    \
                       "<body>\r\n"                                                     \
                       "The document has moved <a href=\"%s\">here</a>.\r\n"            \
                       "</body>\r\n"                                                    \
                       "</html>\r\n",
                       url, url);
    debug("%d: The snprintf() used %d bytes in the buffer.", sockfd, len);
    if (len > 0 && len <= buffer_end - buffer)
        {
        data     = buffer;
        data_end = data + len;
        }
    else
        throw runtime_error("Internal error: Our internal buffer is too small!");
    state = WRITE_REMAINING_DATA;
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }
