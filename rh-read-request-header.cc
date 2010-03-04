/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RequestHandler.hh"
#include "HTTPParser.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::get_request_header()
    {
    TRACE();

    // An empty line will terminate the request header.

    if (read_buffer.size() >= 2 && read_buffer[0] == '\r' && read_buffer[1] == '\n')
        {
        read_buffer.erase(0, 2);
        debug(("%d: Request header is complete; going into READ_REQUEST_BODY state.", sockfd));
        state = READ_REQUEST_BODY;
        return true;
        }

    // If we do have a complete header line in the read buffer,
    // process it. If not, we need more I/O before we can proceed.

    if (HTTPParser::have_complete_header_line(read_buffer))
        {
        std::string name, data;
        size_t len = http_parser.parse_header(name, data, read_buffer);
        if (len > 0)
            {
            if (strcasecmp("Host", name.c_str()) == 0)
                {
                if (http_parser.parse_host_header(request, data) == 0)
                    {
                    protocol_error("Malformed <tt>Host</tt> header.\r\n");
                    return false;
                    }
                else
                    debug(("%d: Read Host header: host = '%s', port = %d", sockfd,
                           request.host.c_str(),
                           ((request.port.empty()) ? -1 : static_cast<int>(request.port.data()))));
                }
            else if (strcasecmp("If-Modified-Since", name.c_str()) == 0)
                {
                if (http_parser.parse_if_modified_since_header(request, data) == 0)
                    {
                    info("Ignoring malformed If-Modified-Since header from peer %s: '%s'.",
                         peer_address, data.c_str());
                    }
                else
                    debug(("%d: Read If-Modified-Since header: timestamp = '%d'", sockfd, request.if_modified_since.data()));
                }
            else if (strcasecmp("Connection", name.c_str()) == 0)
                {
                debug(("%d: Read Connection header: data = '%s'", sockfd, data.c_str()));
                request.connection = data;
                }
            else if (strcasecmp("Keep-Alive", name.c_str()) == 0)
                {
                debug(("%d: Read Keep-Alive header: data = '%s'", sockfd, data.c_str()));
                request.keep_alive = data;
                }
            else if (strcasecmp("User-Agent", name.c_str()) == 0)
                {
                debug(("%d: Read User-Agent header: data = '%s'", sockfd, data.c_str()));
                request.user_agent = data;
                }
            else if (strcasecmp("Referer", name.c_str()) == 0)
                {
                debug(("%d: Read Referer header: data = '%s'", sockfd, data.c_str()));
                request.referer = data;
                }
            else
                debug(("%d: Ignoring unknown header: '%s' = '%s'", sockfd, name.c_str(), data.c_str()));

            read_buffer.erase(0, len);
            return true;
            }
        else
            {
            protocol_error("Your HTTP request is syntactically incorrect.\r\n");
            return false;
            }
        }

    return false;
    }
