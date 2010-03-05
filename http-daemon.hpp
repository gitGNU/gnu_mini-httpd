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
#include "http.hpp"
#include "io.hpp"

namespace http
{
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
    daemon();
    ~daemon();

    bool operator() (input_buffer &, size_t, output_buffer &);

  private:
    Request       _request;
    bool          _use_persistent_connection;

    // The class is a state machine. Depending on the contents of the
    // state variable, the appropriate handler will be called via a
    // lookup table. Each state handler may return true (go on) or false
    // (re-schedule). Errors are reported via exceptions.

    enum state_t
      { READ_REQUEST_LINE
      , READ_REQUEST_HEADER
      , READ_REQUEST_BODY
      , RESPOND
      , TERMINATE
      };
    state_t _state;

    typedef state_t (daemon::*state_fun_t)(input_buffer &, output_buffer &);
    static state_fun_t const state_handlers[TERMINATE];

    state_t reset();

    // mini-httpd state machine

    state_t get_request_line(input_buffer &, output_buffer &);
    state_t get_request_header(input_buffer &, output_buffer &);
    state_t get_request_body(input_buffer &, output_buffer &);
    state_t respond(input_buffer &, output_buffer &);

    // standard responses

    state_t protocol_error(output_buffer &, const std::string& message);
    state_t moved_permanently(output_buffer &, const std::string& path);
    state_t file_not_found(output_buffer &);
    state_t not_modified(output_buffer &);

    void log_access();

    // file information for the request

    std::string document_root;
    std::string filename;
  };

} // namespace http

#endif // HTTPD_REQUESTHANDLER_HPP
