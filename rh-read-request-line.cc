/*
 * Copyright (c) 2001-2016 Peter Simons <simons@cryp.to>
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

#include <config.h>

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
      debug(("%d: Read request line: method = '%s', http version = '%u.%u', host = '%s', " \
             "port = '%d', path = '%s', query = '%s'",
             sockfd, request.method.c_str(), request.major_version,
             request.minor_version, request.url.host.c_str(),
             ((request.url.port.empty()) ? -1 : static_cast<int>(request.url.port.data())),
             request.url.path.c_str(), request.url.query.c_str()));

      read_buffer.erase(0, len);
      state = READ_REQUEST_HEADER;
      return true;
    }
    else
    {
      protocol_error("The HTTP request line you sent was syntactically incorrect.\r\n");
      return false;
    }
  }

  return false;
}
