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
            debug(("%d: method = '%s'; host = '%s'; port = %d; path = '%s'; query = '%s'; " \
                   "http_major_version = %u; http_minor_version = %u",
                   sockfd, http_parser.res_method.c_str(), http_parser.res_host.c_str(),
                   http_parser.res_port, http_parser.res_path.c_str(),
                   http_parser.res_query.c_str(), http_parser.res_major_version,
                   http_parser.res_minor_version));
            method        = http_parser.res_method;
            host          = http_parser.res_host;
            path          = http_parser.res_path; urldecode(path);
            major_version = http_parser.res_major_version;
            minor_version = http_parser.res_minor_version;
            read_buffer.erase(0, info.length);
            debug(("Request line is complete; going into READ_REQUEST_HEADER state."));
            state = READ_REQUEST_HEADER;
            return true;
            }
        else
            protocol_error("The HTTP request you sent was syntactically incorrect.\r\n");
        }

    return false;
    }
