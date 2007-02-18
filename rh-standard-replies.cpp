/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#include "RequestHandler.hpp"
#include "HTTPParser.hpp"
#include "escape-html-specials.hpp"
#include "timestamp-to-string.hpp"
#include "config.hpp"

using namespace std;

void RequestHandler::protocol_error(string const & message)
{
  HINFO() "protocol error: closing connection";

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
  HINFO() "URL '" << request.url.path << "' not found";

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

void RequestHandler::moved_permanently(string const & path)
{
  HTRACE() "Requested page "
    << request.url.path << " has moved to '"
    << path << "': going into FLUSH_BUFFER state"
    ;

  ostringstream buf;
  buf << "HTTP/1.1 301 Moved Permanently\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n"
      << "Content-Type: text/html\r\n"
      << "Location: http://" << request.host;
  if (request.port && *request.port != 80)
    buf << ":" << *request.port;
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
  if (request.port && *request.port != 80)
    buf << ":" << *request.port;
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
  HTRACE() "requested page not modified: going into FLUSH_BUFFER state";

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
