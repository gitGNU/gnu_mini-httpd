/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "HTTPParser.hh"
#include "urldecode.hh"
#include "log.hh"

using namespace std;
using namespace spirit;

bool RequestHandler::get_request_line()
    {
    TRACE();

    if (read_buffer.find("\r\n") != string::npos)
        {
        http_parser.res_method.erase();
        http_parser.res_host.erase();
        http_parser.res_query.erase();
        http_parser.res_path = "/";
        http_parser.res_port = 80;

        HTTPParser::parse_info_t info =
            parse(read_buffer.data(), read_buffer.data() + read_buffer.size(), http_parser.Request_Line);
        if (info.match)
            {
            debug(("%d: method = '%s'; host = '%s'; port = %d; path = '%s'; query = '%s'",
                   sockfd, http_parser.res_method.c_str(), http_parser.res_host.c_str(),
                   http_parser.res_port, http_parser.res_path.c_str(),
                   http_parser.res_query.c_str()));
            method = http_parser.res_method;
            host   = http_parser.res_host;
            path   = http_parser.res_path; urldecode(path);
            read_buffer.erase(0, info.length);
            state = READ_REQUEST_HEADER;
            return true;
            }
        else
            protocol_error("The HTTP request you sent was syntactically incorrect.\r\n");
        }

    return false;
    }
