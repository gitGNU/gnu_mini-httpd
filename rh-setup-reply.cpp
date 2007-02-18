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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>              // for stat(2)

#include <climits>              // PATH_MAX
#include <cctype>
#include <cstdlib>
#include <cerrno>

#include "HTTPParser.hpp"
#include "RequestHandler.hpp"
#include "timestamp-to-string.hpp"
#include "escape-html-specials.hpp"
#include "urldecode.hpp"
#include "config.hpp"

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

static inline bool is_path_in_hierarchy(char const * hierarchy, char const * path)
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
  struct ::stat         file_stat;

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

  if (!request.port && request.url.port)
      request.port = *request.url.port;

  // Construct the actual file name associated with the hostname and
  // URL, then check whether we can send that file.

  document_root = config->document_root + "/" + request.host;
  filename = document_root + urldecode(request.url.path);

  if (!is_path_in_hierarchy(document_root.c_str(), filename.c_str()))
  {
    if (errno != ENOENT)
    {
      HINFO() "peer requested URL "
        << "'http://" << request.host
        << ':' << (!request.port ? 80 : *request.port)
        << request.url.path
        << "' ('" << filename << "'), which fails the hierarchy check"
        ;
    }
    file_not_found();
    return false;
  }

 stat_again:
  if (stat(filename.c_str(), &file_stat) == -1)
  {
    if (errno != ENOENT)
    {
      HINFO() "peer requested URL "
        << "'http://" << request.host
        << ':' << (!request.port ? 80 : *request.port)
        << request.url.path
        << "' ('" << filename << "'), which fails stat(2): "
        << ::strerror(errno)
        ;
    }
    file_not_found();
    return false;
  }

  if (S_ISDIR(file_stat.st_mode))
  {
    if (*request.url.path.rbegin() == '/')
    {
      filename += config->default_page;
      goto stat_again;    // What does Nikolas Wirth know?
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

  if (request.if_modified_since)
  {
    if (file_stat.st_mtime <= *request.if_modified_since)
    {
      HTRACE() << "Requested file ('" << filename << "')"
               << " has mtime '" << file_stat.st_mtime
               << " and if-modified-since was " << *request.if_modified_since
               << ": Not modified."
        ;
      not_modified();
      return false;
    }
    else
      HTRACE() << "Requested file ('" << filename << "')"
               << " has mtime '" << file_stat.st_mtime
               << " and if-modified-since was " << *request.if_modified_since
               << ": Modified."
        ;
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
    HTRACE() "answering HEAD: going into FLUSH_BUFFER state";
  }
  else // must be GET
  {
    _filefd = ioxx::data_socket(ioxx::posix_socket( ::open(filename.c_str(), O_RDONLY, 0) ));
    if (!_filefd)
    {
      HERROR() "Cannot open requested file " << filename << ": " << ::strerror(errno);
      file_not_found();
      return false;
    }
    state = COPY_FILE;
    HTRACE() "answering GET: going into COPY_FILE state";
  }

  go_to_write_mode();
  return false;
}
