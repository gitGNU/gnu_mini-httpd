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

#include <sys/socket.h>         // POSIX.1-2001: shutdown(2)
#include <ioxx/shared-handler.hpp>
#include <sanity/system-error.hpp>
#include "HTTPParser.hpp"
#include "urldecode.hpp"
#include "config.hpp"

using namespace std;

// ----- Construction/Destruction ---------------------------------------------

RequestHandler::RequestHandler(ioxx::socket s) : _sockfd(s)
{
  I(s);
  HTRACE() << "accepted connection";
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

  HTRACE() << "requesting evmask " << _evmask;
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
