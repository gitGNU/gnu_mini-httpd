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
#include "escape-html-specials.hh"
#include "timestamp-to-string.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

void RequestHandler::protocol_error(const string& message)
{
  TRACE();
  debug(("%d: Protocol error; going into FLUSH_BUFFER state.", sockfd));

  ostringstream buf;
  buf << "HTTP/1.1 400 Bad Request\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n"
  << "Content-Type: text/html\r\n";
  if (!request.connection.empty())
    buf << "Connection: close\r\n";
  buf << "\r\n"
  << "<html>\r\n"
  << "<head>\r\n"
  << "  <title>Bad HTTP Request</title>\r\n"
  << "</head>\r\n"
  << "<body>\r\n"
  << "<h1>Bad HTTP Request</h1>\r\n"
  << "<p>The HTTP request received by this server was incorrect:</p>\r\n"
  << "<blockquote>\r\n"
  << message
  << "</blockquote>\r\n"
  << "</body>\r\n"
  << "</html>\r\n";
  write_buffer = buf.str();
  request.status_code = 400;
  request.object_size = 0;
  use_persistent_connection = false;
  state = FLUSH_BUFFER;
  go_to_write_mode();
}

void RequestHandler::file_not_found()
{
  TRACE();
  debug(("%d: URL '%s' not found; going into FLUSH_BUFFER state.", sockfd, request.url.path.c_str()));

  ostringstream buf;
  buf << "HTTP/1.1 404 Not Found\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n"
  << "Content-Type: text/html\r\n";
  if (!request.connection.empty())
    buf << "Connection: close\r\n";
  buf << "\r\n"
  << "<html>\r\n"
  << "<head>\r\n"
  << "  <title>Page Not Found</title>\r\n"
  << "</head>\r\n"
  << "<body>\r\n"
  << "<h1>Page Not Found</h1>\r\n"
  << "<p>The requested page <tt>"
  << escape_html_specials(request.url.path)
  << "</tt> does not exist on this server.</p>\r\n"
  << "</body>\r\n"
  << "</html>\r\n";
  write_buffer = buf.str();
  request.status_code = 404;
  request.object_size = 0;
  use_persistent_connection = false;
  state = FLUSH_BUFFER;
  go_to_write_mode();
}

void RequestHandler::moved_permanently(const string& path)
{
  TRACE();
  debug(("%d: Requested page %s has moved to '%s'; going into FLUSH_BUFFER state.",
         sockfd, request.url.path.c_str(), path.c_str()));

  ostringstream buf;
  buf << "HTTP/1.1 301 Moved Permanently\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n"
  << "Content-Type: text/html\r\n"
  << "Location: http://" << request.host;
  if (!request.port.empty() && request.port != 80)
    buf << ":" << request.port;
  buf << path << "\r\n";
  if (!request.connection.empty())
    buf << "Connection: close\r\n";
  buf << "\r\n"
  << "<html>\r\n"
  << "<head>\r\n"
  << "  <title>Page has moved permanently/title>\r\n"
  << "</head>\r\n"
  << "<body>\r\n"
  << "<h1>Document Has Moved</h1>\r\n"
  << "<p>The document has moved <a href=\"http://" << request.host;
  if (!request.port.empty() && request.port != 80)
    buf << ":" << request.port;
  buf << path << "\">here</a>.\r\n"
  << "</body>\r\n"
  << "</html>\r\n";
  write_buffer        = buf.str();
  request.status_code = 301;
  request.object_size = 0;
  use_persistent_connection = false;
  state = FLUSH_BUFFER;
  go_to_write_mode();
}

void RequestHandler::not_modified()
{
  TRACE();
  debug(("%d: Requested page was not modified; going into FLUSH_BUFFER state.", sockfd));

  ostringstream buf;
  buf << "HTTP/1.1 304 Not Modified\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n";
  if (!request.connection.empty())
  {
    if (use_persistent_connection)
    {
      buf << "Connection: keep-alive\r\n"
      << "Keep-Alive: timeout=" << config->network_read_timeout << ", max=100\r\n";
    }
    else
    {
      buf << "Connection: close\r\n";
    }
  }
  buf << "\r\n";
  write_buffer = buf.str();
  request.status_code = 304;
  state = FLUSH_BUFFER;
  go_to_write_mode();
}


