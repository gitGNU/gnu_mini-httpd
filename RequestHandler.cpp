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
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <ioxx/shared-handler.hpp>
#include <sanity/system-error.hpp>
#include <sys/socket.h>         // POSIX.1-2001: shutdown(2)
#include "HTTPParser.hpp"
#include "urldecode.hpp"
#include "timestamp-to-string.hpp"
#include "escape-html-specials.hpp"
#include "config.hpp"

using namespace std;

// ----- Construction/Destruction ---------------------------------------------

RequestHandler::RequestHandler(ioxx::socket s) : _sockfd(s)
{
  I(s);
  HTRACE() "accepted connection";
  _buf_array.resize(config->max_line_length);
  _inbuf = ioxx::stream_buffer<char>(&_buf_array[0], &_buf_array[_buf_array.size()]);
  reset();
}

RequestHandler::~RequestHandler()
{
  HTRACE() "closing connection";
}


// (Re-)initialize internals before reading a new request over a persistent
// connection.

void RequestHandler::reset()
{
  state = READ_REQUEST_LINE;
  _filefd.reset();
  request = HTTPRequest();
  request.start_up_time = time(0);
  go_to_read_mode();
}

// ----- i/o code -------------------------------------------------------------

void RequestHandler::acceptor::operator()(ioxx::socket const & s, ioxx::probe & p) const
{
  I(s);
  ioxx::add_shared_handler( p, s, ioxx::Readable
                          , boost::shared_ptr<RequestHandler>(new RequestHandler(s))
                          );
}

void RequestHandler::fd_is_readable()
{
  if (!_inbuf.full())
  {
    if (!read(_sockfd, _inbuf))
    {
      if (state != READ_REQUEST_LINE || _inbuf.empty() == false)
        HINFO() "connection terminated by peer ";
      terminate();
    }
  }
  else
    protocol_error("Aborting because of excessively long header lines.\r\n");

  call_state_handler();
}

void RequestHandler::fd_is_writable()
{
  if (state != TERMINATE && !write_buffer.empty())
  {
    const char * p = _sockfd.write(write_buffer.data(), write_buffer.data() + write_buffer.size());
    I(write_buffer.data() <= p);
    write_buffer.erase(0, p - write_buffer.data());
  }
  call_state_handler();
}

ioxx::event RequestHandler::operator() (ioxx::socket s, ioxx::event ev, ioxx::probe &)
{
  I(s == _sockfd);
  if (ioxx::is_error(ev))      return ioxx::Error;
  try
  {
    if (ioxx::is_readable(ev))   fd_is_readable();
    if (ioxx::is_writable(ev))   fd_is_writable();
  }
  catch(system_error const & e)
  {
    HINFO() << e.what();
    terminate();
  }
  catch(std::exception const & e)
  {
    HERROR() "run-time error: " << e.what();
    terminate();
  }
  catch(...)
  {
    HERROR() "unspecified run-time error";
    terminate();
  }

  HTRACE() "requesting evmask " << _evmask;
  return _evmask;
}

// ----- HTTP state machine ---------------------------------------------------

/**
 *  State handler configuration.
 */

RequestHandler::state_fun_t const RequestHandler::state_handlers[] =
  { &RequestHandler::get_request_line
  , &RequestHandler::get_request_header
  , &RequestHandler::get_request_body
  , &RequestHandler::setup_reply
  , &RequestHandler::copy_file
  , &RequestHandler::flush_buffer
  , &RequestHandler::terminate
  };

/// \todo get rid of the string copy

bool RequestHandler::get_request_line()
{
  for (char const * p = _inbuf.data().begin(); p != _inbuf.data().end(); ++p)
  {
    if (p[0] == '\r' && p + 1 != _inbuf.data().end() && p[1] == '\n')
    {
      ++p;
      string const read_buffer(_inbuf.data().begin(), p + 1 - _inbuf.data().begin());
      size_t len = http_parser.parse_request_line(request, read_buffer);
      if (len > 0)
      {
        HTRACE() "Read request line"
        << ": method = "        << request.method
        << ", http version = "  << request.major_version << '.' << request.minor_version
        << ", host = "          << request.url.host
        << ", port = "          << (!request.url.port ? -1 : static_cast<int>(*request.url.port))
        << ", path = '"         << request.url.path << "'"
        << ", query = '"        << request.url.query << "'"
        ;
        I(len == read_buffer.size());
        ioxx::reset_begin(_inbuf.data(), _inbuf.data().begin() + len);
        state = READ_REQUEST_HEADER;
        return true;
      }
      else
      {
        protocol_error("The HTTP request line you sent was syntactically incorrect.\r\n");
        break;
      }
    }
  }
  return false;
}

/// \todo The parsers need to be re-rewritten. We have one unnecessary
/// copy operation just because the bloody string interface is
/// hard-coded.

bool RequestHandler::get_request_header()
{
  // An empty line will terminate the request header.

  if (_inbuf.data().size() >= 2 && _inbuf.data()[0] == '\r' && _inbuf.data()[1] == '\n')
  {
    ioxx::reset_begin(_inbuf.data(), _inbuf.data().begin() + 2);
    HTRACE() "have complete header: going into READ_REQUEST_BODY state.";
    state = READ_REQUEST_BODY;
    return true;
  }

  // If we do have a complete header line in the read buffer,
  // process it. If not, we need more I/O before we can proceed.

  char const * p = HTTPParser::find_next_line(_inbuf.data().begin(), _inbuf.data().end());
  if (p != _inbuf.data().end())
  {
    size_t const llen = p - _inbuf.data().begin();
    string const read_buffer(_inbuf.data().begin(), llen);
    ioxx::reset_begin(_inbuf.data(), _inbuf.data().begin() + llen);
    string name, data;
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
          HTRACE() "Read Host header"
            << ": host = " << request.host
            << "; port = " << (!request.port ? -1 : static_cast<int>(*request.port))
            ;
      }
      else if (strcasecmp("If-Modified-Since", name.c_str()) == 0)
      {
        if (http_parser.parse_if_modified_since_header(request, data) == 0)
          HINFO() "Ignoring malformed If-Modified-Since header '" << data << "'";
        else
          HTRACE() "Read If-Modified-Since header: timestamp = " << *request.if_modified_since;
      }
      else if (strcasecmp("Connection", name.c_str()) == 0)
      {
        HTRACE() "Read Connection header: data = '" << data << "'";
        request.connection = data;
      }
      else if (strcasecmp("Keep-Alive", name.c_str()) == 0)
      {
        HTRACE() "Read Keep-Alive header: data = '" << data << "'";
        request.keep_alive = data;
      }
      else if (strcasecmp("User-Agent", name.c_str()) == 0)
      {
        HTRACE() "Read User-Agent header: data = '" << data << "'";
        request.user_agent = data;
      }
      else if (strcasecmp("Referer", name.c_str()) == 0)
      {
        HTRACE() "Read Referer header: data = '" << data << "'";
        request.referer = data;
      }
      else
        HTRACE() "Ignoring unknown header: '" << name << "' = '" << data << "'";

      I(len == llen);
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

bool RequestHandler::get_request_body()
{
  // We ain't reading any bodies yet.

  HTRACE() "No request body; going into SETUP_REPLY state.";
  state = SETUP_REPLY;
  return true;
}

/**
 *  In COPY_FILE state, all we do is fill up the write_buffer with
 *  data when it's empty, and if the file is through, we'll go into
 *  any of the FLUSH_BUFFER states -- depending on whether we support
 *  persistent connections or not -- to go on.
 */
bool RequestHandler::copy_file()
{
  if (write_buffer.empty())
  {
    char  buf[4096];
    const char * const p = _filefd.read(buf, buf + sizeof(buf));
    if (p)     write_buffer.assign(buf, p - buf);
    else
    {
      HTRACE() "file copy complete: going into FLUSH_BUFFER state";
      state = FLUSH_BUFFER;
      _filefd.reset();
    }
    return true;
  }
  return false;
}

/**
 *  In FLUSH_BUFFER state, ...
 */
bool RequestHandler::flush_buffer()
{
  if (write_buffer.empty())
  {
    log_access();
    if (use_persistent_connection)
    {
      HTRACE() "restarting persistent connection";
      reset();
      return true;
    }
    else
    {
      state = TERMINATE;
      if (::shutdown(_sockfd->fd, SHUT_RDWR) == -1)
        terminate();
    }
  }
  return false;
}

/**
 * This routine will normalize the path of our document root and of the
 * file that our client is trying to access. Then it will check,
 * whether that file is _in_ the document root. As a nice side effect,
 * realpath() will report any kind of error, for instance, if a part of
 * the path gives us "permission denied", does not exist or whatever.
 *
 * I am not happy with this routine, nor am I happy with this whole
 * approach. I'll improve it soon.
 *
 * Greetings to Ralph!
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

// ----- Logging --------------------------------------------------------------

inline void escape_quotes(string & input)
{
  boost::algorithm::replace_all(input, "\"", "\\\"");
}

void RequestHandler::log_access()
{
  if (!request.status_code)
  {
    HERROR() "Can't write access log entry because there is no status code.";
    return;
  }

  string logfile( config->logfile_directory + "/" );
  if (request.host.empty())     logfile += "no-hostname";
  else                          logfile += request.host + "-access";

  // Convert the object size now, so that we can write a string.
  // That's necessary, because in some cases we write "-" rather
  // than a number.

  char object_size[32];
  if (!request.object_size)
    strcpy(object_size, "-");
  else
  {
    int len = snprintf(object_size, sizeof(object_size), "%u", *request.object_size);
    if (len < 0 || len > static_cast<int>(sizeof(object_size)))
      HERROR() "internal error: snprintf() exceeded buffer size while formatting log entry";
  }
  // Open it and write the entry.

  ofstream os(logfile.c_str(), ios_base::out); // no ios_base::trunc
  if (!os.is_open())
    throw system_error((string("Can't open logfile '") + logfile + "'"));

  escape_quotes(request.url.path);
  escape_quotes(request.referer);
  escape_quotes(request.user_agent);

  // "%s - - [%s] \"%s %s HTTP/%u.%u\" %u %s \"%s\" \"%s\"\n"
  os << "TODO" << " - - [" << time_to_logdate(request.start_up_time) << "]"
     << " \"" << request.method
     << " "  << request.url.path
     << " HTTP/" << request.major_version
     << "." << request.minor_version << "\""
     << " " << *request.status_code
     << " " << object_size
     << " \"" << request.referer << "\""
     << " \"" << request.user_agent << "\""
     << endl;
}
