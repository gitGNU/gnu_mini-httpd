/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "HTTPParser.hh"
#include "log.hh"

using namespace std;
using namespace spirit;

const RequestHandler::header_parser_t RequestHandler::header_parsers[] =
    {
    { "host", &RequestHandler::parse_host_header },
    { 0, 0 }
    };

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
            read_buffer.erase(0, info.length);

            for (const header_parser_t* p = header_parsers; p->name; ++p)
                {
                if (strcasecmp(p->name, http_parser.res_name.c_str()) == 0)
                    {
                    return (this->*(p->parser))();
                    }
                }

            debug(("%d: name = '%s'; data = '%s'", sockfd, http_parser.res_name.c_str(), http_parser.res_data.c_str()));
            return true;
            }
        else
            protocol_error("The HTTP request you sent was syntactically incorrect.\r\n");
        }

    return false;
    }

bool RequestHandler::parse_host_header()
    {
    TRACE();

    http_parser.res_host.erase();
    http_parser.res_port = 80;

    HTTPParser::parse_info_t info = parse(http_parser.res_data.data(),
                                          http_parser.res_data.data() + http_parser.res_data.size(),
                                          http_parser.Host_Header);
    if (info.full)
        {
        debug(("%d: Host header: name = '%s'; port = %d", sockfd, http_parser.res_name.c_str(), http_parser.res_port));
        if (!host.empty())
            {
            if (strcasecmp(http_parser.res_host.c_str(), host.c_str()) != 0)
                {
                protocol_error("Received a full URL <em>and</em> a <tt>Host</tt>\r\n" \
                               "header, but the two host names differ!\r\n");
                return false;
                }
            }
        host = http_parser.res_host;
        return true;
        }
    else
        protocol_error("Malformed <tt>Host</tt> header.\r\n");

    return false;
    }
