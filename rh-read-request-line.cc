/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "HTTPParser.hh"
#include "urldecode.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::get_request_line()
    {
    TRACE();

    if (read_buffer.find("\r\n") != string::npos)
        {
        size_t len = http_parser.parse_request_line(request, read_buffer);
        if (len > 0)
            {
            debug(("%d: Read request line:", sockfd));
            debug(("%d:     host  = '%s'", sockfd, request.url.host.data().c_str()));
            debug(("%d:     port  = '%d'", sockfd, request.url.port.data()));
            debug(("%d:     path  = '%s'", sockfd, request.url.path.data().c_str()));
            debug(("%d:     query = '%s'", sockfd, request.url.query.data().c_str()));
            read_buffer.erase(0, len);
            state = READ_REQUEST_HEADER;
            return true;
            }
        else
            {
            protocol_error("The HTTP request you sent was syntactically incorrect.\r\n");
            return false;
            }
        }

    return false;
    }
