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
#include <map>
#include <boost/log/log.hpp>
#include <ioxx/stream-buffer.hpp>
#include <ioxx/probe.hpp>
#include "http.hpp"

namespace http
{
  namespace logging
  {
    BOOST_DECLARE_LOG(access)
    BOOST_DECLARE_LOG(misc)
    BOOST_DECLARE_LOG_DEBUG(debug)
  }

  class configuration : private boost::noncopyable
  {
  public:
    // Construction and Destruction.
    configuration(int, char**);

    // Timeouts.
    unsigned int network_read_timeout;
    unsigned int network_write_timeout;
    unsigned int hard_poll_interval_threshold;
    int          hard_poll_interval;

    // Buffer sizes.
    unsigned int max_line_length;

    // Paths.
    std::string  chroot_directory;
    std::string  logfile_directory;
    std::string  document_root;
    std::string  default_page;

    // Run-time stuff.
    char const *               default_content_type;
    std::string                default_hostname;
    unsigned int               http_port;
    std::string                server_string;
    boost::optional<uid_t>     setuid_user;
    boost::optional<gid_t>     setgid_group;
    bool                       debugging;
    bool                       detach;

    // Content-type mapping.
    char const * get_content_type(char const * filename) const;

    // The error class throw in case version or usage information has
    // been requested and we're supposed to terminate.
    struct no_error { };

  private:
    struct ltstr
    {
      bool operator()(char const * lhs, char const * rhs) const
      {
        return ::strcasecmp(lhs, rhs) < 0;
      }
    };

    typedef std::map<char const *, char const *, ltstr> map_t;
    map_t content_types;
  };

  extern configuration const * config;


  /**
   *  \brief This is the HTTP protocol driver class.
   */
  class daemon : private boost::noncopyable
  {
  public:
    struct acceptor
    {
      void operator()(ioxx::socket const & s, ioxx::probe & p) const;
    };

    daemon(ioxx::socket s);
    ~daemon();
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

    typedef bool (daemon::*state_fun_t)();
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

    Request       request;
    bool          use_persistent_connection;

    // file information for the request

    std::string document_root;
    std::string filename;
  };

} // namespace http

#endif // HTTPD_REQUESTHANDLER_HPP
