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

#include "http-daemon.hpp"
#include <fstream>
#include <boost/algorithm/string/replace.hpp>
#include <ioxx/shared-handler.hpp>
#include <sanity/system-error.hpp>
#include <sys/socket.h>         // POSIX.1-2001: shutdown(2)

#ifdef _GNU_SOURCE
#  include <getopt.h>           // getopt_long(3)
#endif

// Abstract access to our logger.
#define TRACE()         BOOST_LOGL(::http::logging::debug, dbg)
#define INFO()          BOOST_LOGL(::http::logging::misc,  info)
#define ERROR()         BOOST_LOGL(::http::logging::misc,  err)

// Continue with literal string or operator<<().
#define SOCKET_TRACE(s) TRACE() << "socket " << s << ": "
#define SOCKET_INFO(s)  INFO()  << "socket " << s << ": "
#define SOCKET_ERROR(s) ERROR() << "socket " << s << ": "

// Continue with literal string or operator<<().
#define HTRACE() SOCKET_TRACE(daemon::_sockfd)
#define HINFO()  SOCKET_INFO(daemon::_sockfd)
#define HERROR() SOCKET_ERROR(daemon::_sockfd)

using namespace std;

// ----- Construction/Destruction ---------------------------------------------

http::daemon::daemon(ioxx::socket s) : _sockfd(s)
{
  I(s);
  HTRACE() "accepted connection";
  _buf_array.resize(config->max_line_length);
  _inbuf = ioxx::stream_buffer<char>(&_buf_array[0], &_buf_array[_buf_array.size()]);
  reset();
}

http::daemon::~daemon()
{
  HTRACE() "closing connection";
}


// (Re-)initialize internals before reading a new request over a persistent
// connection.

void http::daemon::reset()
{
  state = READ_REQUEST_LINE;
  _filefd.reset();
  request = Request();
  request.start_up_time = time(0);
  go_to_read_mode();
}

// ----- i/o code -------------------------------------------------------------

void http::daemon::acceptor::operator()(ioxx::socket const & s, ioxx::probe & p) const
{
  I(s);
  ioxx::add_shared_handler( p, s, ioxx::Readable
                          , boost::shared_ptr<daemon>(new daemon(s))
                          );
}

void http::daemon::fd_is_readable()
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

void http::daemon::fd_is_writable()
{
  if (state != TERMINATE && !write_buffer.empty())
  {
    const char * p = _sockfd.write(write_buffer.data(), write_buffer.data() + write_buffer.size());
    I(write_buffer.data() <= p);
    write_buffer.erase(0, p - write_buffer.data());
  }
  call_state_handler();
}

ioxx::event http::daemon::operator() (ioxx::socket s, ioxx::event ev, ioxx::probe &)
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
  catch(exception const & e)
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

http::daemon::state_fun_t const http::daemon::state_handlers[] =
  { &http::daemon::get_request_line
  , &http::daemon::get_request_header
  , &http::daemon::get_request_body
  , &http::daemon::setup_reply
  , &http::daemon::copy_file
  , &http::daemon::flush_buffer
  , &http::daemon::terminate
  };

/// \todo get rid of the string copy

bool http::daemon::get_request_line()
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
        << ", port = "          << request.url.port
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

bool http::daemon::get_request_header()
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

  char const * p = parser::find_next_line(_inbuf.data().begin(), _inbuf.data().end());
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
            << "; port = " << request.port
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

bool http::daemon::get_request_body()
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
bool http::daemon::copy_file()
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
bool http::daemon::flush_buffer()
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

inline string to_rfcdate(time_t t)
{
  char buffer[64];
  size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
  if (len == 0 || len >= sizeof(buffer))
    throw length_error("strftime() failed because an internal buffer is too small!");
  return buffer;
}

bool http::daemon::setup_reply()
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
    if (request.host.empty())
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
      request.port = request.url.port;

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
        << ':' << (request.port ? request.port : 80u)
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
        << ':' << (!request.port ? 80 : request.port)
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

  use_persistent_connection = parser::supports_persistent_connection(request);

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
  buf << "Date: " << to_rfcdate(time(0)) << "\r\n"
      << "Content-Type: " << config->get_content_type(filename.c_str()) << "\r\n"
      << "Content-Length: " << file_stat.st_size << "\r\n"
      << "Last-Modified: " << to_rfcdate(file_stat.st_mtime) << "\r\n";
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

inline string to_logdate(time_t t)
{
  char buffer[64];
  size_t len = strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", localtime(&t));
  if (len == 0 || len >= sizeof(buffer))
    throw length_error("strftime() failed because the internal buffer is too small");
  return buffer;
}

void http::daemon::log_access()
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
  os << "TODO" << " - - [" << to_logdate(request.start_up_time) << "]"
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

// ----- Standard Responses ---------------------------------------------------

void http::daemon::protocol_error(string const & message)
{
  HINFO() "protocol error: closing connection";

  ostringstream buf;
  buf << "HTTP/1.1 400 Bad Request\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << to_rfcdate(time(0)) << "\r\n"
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

void http::daemon::file_not_found()
{
  HINFO() "URL '" << request.url.path << "' not found";

  ostringstream buf;
  buf << "HTTP/1.1 404 Not Found\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << to_rfcdate(time(0)) << "\r\n"
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

void http::daemon::moved_permanently(string const & path)
{
  HTRACE() "Requested page "
    << request.url.path << " has moved to '"
    << path << "': going into FLUSH_BUFFER state"
    ;

  ostringstream buf;
  buf << "HTTP/1.1 301 Moved Permanently\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << to_rfcdate(time(0)) << "\r\n"
      << "Content-Type: text/html\r\n"
      << "Location: http://" << request.host;
  if (request.port && request.port != 80)
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
  if (request.port && request.port != 80)
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

void http::daemon::not_modified()
{
  HTRACE() "requested page not modified: going into FLUSH_BUFFER state";

  ostringstream buf;
  buf << "HTTP/1.1 304 Not Modified\r\n";
  if (!config->server_string.empty())
    buf << "Server: " << config->server_string << "\r\n";
  buf << "Date: " << to_rfcdate(time(0)) << "\r\n";
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


// ----- Boost.Log ------------------------------------------------------------

#include <boost/log/log.hpp>
namespace http { namespace logging
{
  BOOST_DEFINE_LOG(access, "httpd.access")
  BOOST_DEFINE_LOG(misc,   "httpd.misc")
  BOOST_DEFINE_LOG(debug,  "httpd.debug")
}}

// ----- Configuration --------------------------------------------------------

#define kb * 1024
#define mb * 1024 kb
#define sec * 1

#define USAGE_MSG                                                       \
  "Usage: httpd  [ --version ]\n"                                       \
  "              [ -h        | --help ]\n"                              \
  "              [ -d        | --debug ]\n"                             \
  "              [ -D        | --no-detach ]\n"                         \
  "              [ -p number | --port               number ]\n"         \
  "              [ -r path   | --change-root        path   ]\n"         \
  "              [ -l path   | --logfile-directory  path   ]\n"         \
  "              [ -s string | --server-string      string ]\n"         \
  "              [ -u uid    | --uid                uid    ]\n"         \
  "              [ -g gid    | --gid                gid    ]\n"         \
  "              [ -H string | --default-hostname   string ]\n"         \
  "              [ --default-page  filename ]\n"    \
  "              [ --document-root path ]\n"

http::configuration::configuration(int argc, char** argv)
{
  // timeouts
  network_read_timeout                  = 30 sec;
  network_write_timeout                 = 30 sec;

  // buffer size
  max_line_length                       =  4 kb;

  // paths
  chroot_directory                      = "";
  logfile_directory                     = "/logs";
  document_root                         = "/htdocs";
  default_page                          = "index.html";

  // run-time stuff
  server_string                         = "mini-httpd";
  default_hostname                      = "localhost";
  default_content_type                  = "application/octet-stream";
  http_port                             = 80;

  debugging                             = false;
  detach                                = true;

  // Parse the command line.

  char const * optstring = "hdp:r:l:s:u:g:DH:";
  const option longopts[] =
    {
      { "help",               no_argument,       0, 'h' },
      { "version",            no_argument,       0, 'v' },
      { "debug",              no_argument,       0, 'd' },
      { "port",               required_argument, 0, 'p' },
      { "change-root",        required_argument, 0, 'r' },
      { "logfile-directory",  required_argument, 0, 'l' },
      { "server-string",      required_argument, 0, 's' },
      { "uid",                required_argument, 0, 'u' },
      { "gid",                required_argument, 0, 'g' },
      { "no-detach",          required_argument, 0, 'D' },
      { "default-hostname",   required_argument, 0, 'H' },
      { "document-root",      required_argument, 0, 'y' },
      { "default-page",       required_argument, 0, 'z' },
      { 0, 0, 0, 0 }          // mark end of array
    };
  int rc;
  opterr = 0;
  while ((rc = getopt_long(argc, argv, optstring, longopts, 0)) != -1)
  {
    switch(rc)
    {
      case 'h':
        fprintf(stderr, USAGE_MSG);
        throw no_error();
      case 'v':
        printf("mini-httpd version 2007-02-27\n");
        throw no_error();
      case 'd':
        debugging = true;
        break;
      case 'p':
        http_port = atoi(optarg);
        if (http_port > 65535)
          throw runtime_error("The specified port number is out of range!");
        break;
      case 'g':
        setgid_group = atoi(optarg);
        break;
      case 'u':
        setuid_user = atoi(optarg);
        break;
      case 'r':
        chroot_directory = optarg;
        break;
      case 'y':
        document_root = optarg;
        break;
      case 'z':
        default_page = optarg;
        break;
      case 'l':
        logfile_directory = optarg;
        break;
      case 's':
        server_string = optarg;
        break;
      case 'D':
        detach = false;
        break;
      case 'H':
        default_hostname = optarg;
        break;
      default:
        fprintf(stderr, USAGE_MSG);
        throw runtime_error("Incorrect command line syntax.");
    }

    // Consistency checks on the configured parameters.

    if (default_page.empty())
      throw invalid_argument("Setting an empty --default-page is not allowed.");
    if (document_root.empty())
      throw invalid_argument("Setting an empty --document-root is not allowed.");
    if (logfile_directory.empty())
      throw invalid_argument("Setting an empty --document-root is not allowed.");
  }

  // Initialize the content type lookup map.

  content_types["ai"]      = "application/postscript";
  content_types["aif"]     = "audio/x-aiff";
  content_types["aifc"]    = "audio/x-aiff";
  content_types["aiff"]    = "audio/x-aiff";
  content_types["asc"]     = "text/plain";
  content_types["au"]      = "audio/basic";
  content_types["avi"]     = "video/x-msvideo";
  content_types["bcpio"]   = "application/x-bcpio";
  content_types["bmp"]     = "image/bmp";
  content_types["cdf"]     = "application/x-netcdf";
  content_types["cpio"]    = "application/x-cpio";
  content_types["cpt"]     = "application/mac-compactpro";
  content_types["csh"]     = "application/x-csh";
  content_types["css"]     = "text/css";
  content_types["dcr"]     = "application/x-director";
  content_types["dir"]     = "application/x-director";
  content_types["doc"]     = "application/msword";
  content_types["dvi"]     = "application/x-dvi";
  content_types["dxr"]     = "application/x-director";
  content_types["eps"]     = "application/postscript";
  content_types["etx"]     = "text/x-setext";
  content_types["gif"]     = "image/gif";
  content_types["gtar"]    = "application/x-gtar";
  content_types["hdf"]     = "application/x-hdf";
  content_types["hqx"]     = "application/mac-binhex40";
  content_types["htm"]     = "text/html";
  content_types["html"]    = "text/html";
  content_types["ice"]     = "x-conference/x-cooltalk";
  content_types["ief"]     = "image/ief";
  content_types["iges"]    = "model/iges";
  content_types["igs"]     = "model/iges";
  content_types["jpe"]     = "image/jpeg";
  content_types["jpeg"]    = "image/jpeg";
  content_types["jpg"]     = "image/jpeg";
  content_types["js"]      = "application/x-javascript";
  content_types["kar"]     = "audio/midi";
  content_types["latex"]   = "application/x-latex";
  content_types["man"]     = "application/x-troff-man";
  content_types["me"]      = "application/x-troff-me";
  content_types["mesh"]    = "model/mesh";
  content_types["mid"]     = "audio/midi";
  content_types["midi"]    = "audio/midi";
  content_types["mov"]     = "video/quicktime";
  content_types["movie"]   = "video/x-sgi-movie";
  content_types["mp2"]     = "audio/mpeg";
  content_types["mp3"]     = "audio/mpeg";
  content_types["mpe"]     = "video/mpeg";
  content_types["mpeg"]    = "video/mpeg";
  content_types["mpg"]     = "video/mpeg";
  content_types["mpga"]    = "audio/mpeg";
  content_types["ms"]      = "application/x-troff-ms";
  content_types["msh"]     = "model/mesh";
  content_types["nc"]      = "application/x-netcdf";
  content_types["oda"]     = "application/oda";
  content_types["pbm"]     = "image/x-portable-bitmap";
  content_types["pdb"]     = "chemical/x-pdb";
  content_types["pdf"]     = "application/pdf";
  content_types["pgm"]     = "image/x-portable-graymap";
  content_types["pgn"]     = "application/x-chess-pgn";
  content_types["png"]     = "image/png";
  content_types["pnm"]     = "image/x-portable-anymap";
  content_types["ppm"]     = "image/x-portable-pixmap";
  content_types["ppt"]     = "application/vnd.ms-powerpoint";
  content_types["ps"]      = "application/postscript";
  content_types["qt"]      = "video/quicktime";
  content_types["ra"]      = "audio/x-realaudio";
  content_types["ram"]     = "audio/x-pn-realaudio";
  content_types["ras"]     = "image/x-cmu-raster";
  content_types["rgb"]     = "image/x-rgb";
  content_types["rm"]      = "audio/x-pn-realaudio";
  content_types["roff"]    = "application/x-troff";
  content_types["rpm"]     = "audio/x-pn-realaudio-plugin";
  content_types["rtf"]     = "application/rtf";
  content_types["rtf"]     = "text/rtf";
  content_types["rtx"]     = "text/richtext";
  content_types["sgm"]     = "text/sgml";
  content_types["sgml"]    = "text/sgml";
  content_types["sh"]      = "application/x-sh";
  content_types["shar"]    = "application/x-shar";
  content_types["silo"]    = "model/mesh";
  content_types["sit"]     = "application/x-stuffit";
  content_types["skd"]     = "application/x-koan";
  content_types["skm"]     = "application/x-koan";
  content_types["skp"]     = "application/x-koan";
  content_types["skt"]     = "application/x-koan";
  content_types["snd"]     = "audio/basic";
  content_types["spl"]     = "application/x-futuresplash";
  content_types["src"]     = "application/x-wais-source";
  content_types["sv4cpio"] = "application/x-sv4cpio";
  content_types["sv4crc"]  = "application/x-sv4crc";
  content_types["swf"]     = "application/x-shockwave-flash";
  content_types["t"]       = "application/x-troff";
  content_types["tar"]     = "application/x-tar";
  content_types["tcl"]     = "application/x-tcl";
  content_types["tex"]     = "application/x-tex";
  content_types["texi"]    = "application/x-texinfo";
  content_types["texinfo"] = "application/x-texinfo";
  content_types["tif"]     = "image/tiff";
  content_types["tiff"]    = "image/tiff";
  content_types["tr"]      = "application/x-troff";
  content_types["tsv"]     = "text/tab-separated-values";
  content_types["txt"]     = "text/plain";
  content_types["ustar"]   = "application/x-ustar";
  content_types["vcd"]     = "application/x-cdlink";
  content_types["vrml"]    = "model/vrml";
  content_types["wav"]     = "audio/x-wav";
  content_types["wrl"]     = "model/vrml";
  content_types["xbm"]     = "image/x-xbitmap";
  content_types["xls"]     = "application/vnd.ms-excel";
  content_types["xml"]     = "text/xml";
  content_types["xpm"]     = "image/x-xpixmap";
  content_types["xwd"]     = "image/x-xwindowdump";
  content_types["xyz"]     = "chemical/x-pdb";
  content_types["zip"]     = "application/zip";
  content_types["hpp"]     = "text/plain";
  content_types["cpp"]     = "text/plain";
}

char const * http::configuration::get_content_type(char const * filename) const
{
  char const * last_dot;
  char const * current;
  for (current = filename, last_dot = 0; *current != '\0'; ++current)
    if (*current == '.')
      last_dot = current;
  if (last_dot == 0)
    return default_content_type;
  else
    ++last_dot;
  map_t::const_iterator i = content_types.find(last_dot);
  if (i != content_types.end())
    return i->second;
  else
    return default_content_type;
}

http::configuration const * http::config;
