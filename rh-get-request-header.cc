/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "HTTPParser.hh"
#include "log.hh"

using namespace std;
using namespace spirit;

bool RequestHandler::get_request_header()
    {
    TRACE();

    if (read_buffer.find("\r\n") == 0)
        {
        read_buffer.erase(0, 2);
        state = READ_REQUEST_BODY;
        return true;
        }

    if (HTTPParser::have_complete_header_line(read_buffer.begin(), read_buffer.end()))
        {
        http_parser.res_name.erase();
        http_parser.res_data.erase();

        HTTPParser::parse_info_t info =
            parse(read_buffer.data(), read_buffer.data() + read_buffer.size(), http_parser.Header);
        if (info.match)
            {
            debug(("%d: name = '%s'; data = '%s'", sockfd, http_parser.res_name.c_str(), http_parser.res_data.c_str()));
            read_buffer.erase(0, info.length);
            return true;
            }
        else
            protocol_error("The HTTP request you sent was syntactically incorrect.\r\n");
        }

    return false;
    }
