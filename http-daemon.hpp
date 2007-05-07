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
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "http.hpp"
#include "io.hpp"

namespace http
{
  class configuration : private boost::noncopyable
  {
  public:
    // Construct default configuration.
    configuration();

    // Paths.
    std::string logfile_root;
    std::string document_root;
    std::string default_page;

    // Run-time stuff.
    std::string default_content_type;
    std::string default_hostname;
    std::string server_string;

    // Content-type mapping.
    char const * get_content_type(char const * filename) const;

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

  inline std::ostream & operator<< (std::ostream & os, configuration const & cfg)
  {
    return os
      << "access logging = " << (cfg.logfile_root.empty() ? "disabled" : cfg.logfile_root)
      << "; htdocs = " << cfg.document_root
      << "; server id = " << (cfg.server_string.empty() ? "disabled" : cfg.server_string)
      << "; pre-http/1.1 host = " << (cfg.default_hostname.empty() ? "disabled" : cfg.default_hostname)
      << "; index page = " << cfg.default_page;
  }

  /**
   *  \brief This is the HTTP protocol driver class.
   */
  class daemon : private boost::noncopyable
  {
  public:
    daemon();
    ~daemon();

    bool operator() (input_buffer &, size_t, output_buffer &);

    static configuration _config;

  private:
    // transaction state

    Request                                     _request;
    bool                                        _use_persistent_connection;

    typedef boost::shared_ptr<char const>       shared_buffer;
    std::vector<shared_buffer>                  _payload;

    // state machine

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

    void reset();

    state_t get_request_line(input_buffer &, output_buffer &);
    state_t get_request_header(input_buffer &, output_buffer &);
    state_t get_request_body(input_buffer &, output_buffer &);
    state_t respond(input_buffer &, output_buffer &);
    state_t restart(input_buffer & ibuf, output_buffer & obuf);

    // standard responses

    state_t protocol_error(output_buffer &, const std::string& message);
    state_t moved_permanently(output_buffer &, const std::string& path);
    state_t file_not_found(output_buffer &, std::string const &);
    void    not_modified(output_buffer &);

    void log_access();

    // file information for the request

    std::string document_root;
    std::string filename;
  };

} // namespace http

#endif // HTTPD_REQUESTHANDLER_HPP
