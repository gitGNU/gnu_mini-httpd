/*
 * Copyright (c) 2001-2013 Peter Simons <simons@cryp.to>
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

#include <climits>
#include <cctype>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "system-error.hh"
#include "HTTPParser.hh"
#include "RequestHandler.hh"
#include "timestamp-to-string.hh"
#include "escape-html-specials.hh"
#include "urldecode.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

/*
  This routine will normalize the path of our document root and of the
  file that our client is trying to access. Then it will check,
  whether that file is _in_ the document root. As a nice side effect,
  realpath() will report any kind of error, for instance, if a part of
  the path gives us "permission denied", does not exist or whatever.

  I am not happy with this routine, nor am I happy with this whole
  approach. I'll improve it soon.

  Greetings to Ralph!
*/

static inline bool is_path_in_hierarchy(const char* hierarchy, const char* path)
{
  char resolved_hierarchy[PATH_MAX];
  char resolved_path[PATH_MAX];

  if (realpath(hierarchy, resolved_hierarchy) == 0 || realpath(path, resolved_path) == 0)
    return false;
  if (strlen(resolved_hierarchy) > strlen(resolved_path))
    return false;
  if (strncmp(resolved_hierarchy, resolved_path, strlen(resolved_hierarchy)) != 0)
    return false;
  return true;
}

bool RequestHandler::setup_reply()
{
  TRACE();

  // Now that we have the whole request, we can get to work. Let's
  // start by testing whether we understand the request at all.

  if (request.method != "GET" && request.method != "HEAD")
  {
    protocol_error(string("<p>This server does not support an HTTP request called <tt>")
                   + escape_html_specials(request.method) + "</tt>.</p>\r\n");
    return false;
  }

  // Make sure we have a hostname, and make sure it's in lowercase.

  if (request.host.empty())
  {
    if (request.url.host.empty())
    {
      if (!config->default_hostname.empty() &&
          (request.major_version == 0 || (request.major_version == 1 && request.minor_version == 0))
         )
        request.host = config->default_hostname;
      else
      {
        protocol_error("<p>Your HTTP request did not contain a <tt>Host</tt> header.</p>\r\n");
        return false;
      }
    }
    else
      request.host = request.url.host;
  }
  for (string::iterator i = request.host.begin(); i != request.host.end(); ++i)
    *i = tolower(*i);

  // Make sure we have a port number.

  if (request.port.empty())
  {
    if (!request.url.port.empty())
      request.port = request.url.port;
  }

  // Construct the actual file name associated with the hostname and
  // URL, then check whether we can send that file.

  document_root = config->document_root + "/" + request.host;
  filename = document_root + urldecode(request.url.path);

  if (!is_path_in_hierarchy(document_root.c_str(), filename.c_str()))
  {
    if (errno != ENOENT)
    {
      info("Peer %s requested URL 'http://%s:%u%s' ('%s'), which fails the hirarchy check.",
           peer_address, request.host.c_str(), ((request.port.empty()) ? 80 : request.port.data()),
           request.url.path.c_str(), filename.c_str());
    }
    file_not_found();
    return false;
  }

stat_again:
  if (stat(filename.c_str(), &file_stat) == -1)
  {
    if (errno != ENOENT)
    {
      info("Peer %s requested URL 'http://%s:%u%s' ('%s'), but stat() failed: %s",
           peer_address, request.host.c_str(), request.port.data(), request.url.path.c_str(),
           filename.c_str(), strerror(errno));
    }
    file_not_found();
    return false;
  }

  if (S_ISDIR(file_stat.st_mode))
  {
    if (*request.url.path.rbegin() == '/')
    {
      filename += config->default_page;
      goto stat_again;    // What the fuck does Nikolas Wirth know?
    }
    else
    {
      moved_permanently(request.url.path + "/");
      return false;
    }
  }

  // Decide whether to use a persistent connection.

  use_persistent_connection = HTTPParser::supports_persistent_connection(request);

  // Check whether the If-Modified-Since header applies.

  if (!request.if_modified_since.empty())
  {
    if (file_stat.st_mtime <= request.if_modified_since)
    {
      debug(("%d: Requested file ('%s') has mtime '%d' and if-modified-since was '%d: Not modified.",
             sockfd, filename.c_str(), file_stat.st_mtime, request.if_modified_since.data()));
      not_modified();
      return false;
    }
    else
      debug(("%d: Requested file ('%s') has mtime '%d' and if-modified-since was '%d: Modified.",
             sockfd, filename.c_str(), file_stat.st_mtime, request.if_modified_since.data()));
  }

  // Now answer the request, which may be either HEAD or GET.

  ostringstream buf;
  buf << "HTTP/1.1 200 OK\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n"
  << "Content-Type: " << config->get_content_type(filename.c_str()) << "\r\n"
  << "Content-Length: " << file_stat.st_size << "\r\n"
  << "Last-Modified: " << time_to_rfcdate(file_stat.st_mtime) << "\r\n";
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
  write_buffer        = buf.str();
  request.status_code = 200;
  request.object_size = file_stat.st_size;

  if (request.method == "HEAD")
  {
    state = FLUSH_BUFFER;
    debug(("%d: Answering HEAD; going into FLUSH_BUFFER state.", sockfd));
  }
  else // must be GET
  {
    filefd = open(filename.c_str(), O_RDONLY, 0);
    if (filefd == -1)
    {
      error("cannot open requested file %s: %s", filename.c_str(), strerror(errno));
      file_not_found();
      return false;
    }
    state = COPY_FILE;
    debug(("%d: Answering GET; going into COPY_FILE state.", sockfd));
  }

  go_to_write_mode();
  return false;
}
