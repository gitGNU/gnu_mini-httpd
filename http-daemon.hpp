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

#ifndef HTTPD_REQUESTHANDLER_HPP
#define HTTPD_REQUESTHANDLER_HPP

#include <vector>
#include <ioxx/stream-buffer.hpp>
#include <ioxx/probe.hpp>
#include "HTTPRequest.hpp"
#include "log.hpp"

// Continue with literal string or operator<<().
#define HTRACE() SOCKET_TRACE(RequestHandler::_sockfd)
#define HINFO()  SOCKET_INFO(RequestHandler::_sockfd)
#define HERROR() SOCKET_ERROR(RequestHandler::_sockfd)

/**
 *  \brief This is the HTTP protocol driver class.
 */
class RequestHandler : private boost::noncopyable
{
public:
  struct acceptor
  {
    void operator()(ioxx::socket const & s, ioxx::probe & p) const;
  };

  RequestHandler(ioxx::socket s);
  ~RequestHandler();
  ioxx::event operator() (ioxx::socket s, ioxx::event ev, ioxx::probe &);

private:
  ioxx::data_socket                     _sockfd, _filefd;
  std::vector<char>                     _buf_array;
  ioxx::stream_buffer<char>             _inbuf;

  void reset();
  void fd_is_readable();
  void fd_is_writable();

  ioxx::event    _evmask;
  void go_to_read_mode()        { _evmask = ioxx::Readable; }
  void go_to_write_mode()       { _evmask = ioxx::Writable; }

  // The whole class is implemented as a state machine. Depending on
  // the contents of the state variable, the appropriate state
  // handler will be called via a lookup table. Each state handler
  // may return true (go on) or false (re-schedule). Errors are
  // reported via exceptions.

  enum state_t
    {
      READ_REQUEST_LINE,
      READ_REQUEST_HEADER,
      READ_REQUEST_BODY,
      SETUP_REPLY,
      COPY_FILE,
      FLUSH_BUFFER,
      TERMINATE
    };
  state_t state;

  typedef bool (RequestHandler::*state_fun_t)();
  static state_fun_t const state_handlers[];

  bool get_request_line();
  bool get_request_header();
  bool get_request_body();
  bool setup_reply();
  bool copy_file();
  bool flush_buffer();
  bool terminate()              { _evmask = ioxx::Error; return false; }

  void call_state_handler()
  {
    while((this->*state_handlers[state])())
      ;
  }

private:
  // standard replies

  void protocol_error(const std::string& message);
  void moved_permanently(const std::string& path);
  void file_not_found();
  void not_modified();

  // write an access log entry

  void log_access();

  /// \todo get rid of the separate buffer
  std::string                   write_buffer;

  // the HTTP request

  HTTPRequest  request;
  bool         use_persistent_connection;

  // file information for the request

  std::string document_root;
  std::string filename;
};

#endif // HTTPD_REQUESTHANDLER_HPP
