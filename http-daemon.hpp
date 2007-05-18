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

#include "http.hpp"
#include "io.hpp"
#include "input-buffer.hpp"
#include "output-buffer.hpp"
#include <map>
#include <fstream>

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
    std::size_t io_block_size;

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
      << "; index page = " << cfg.default_page
      << "; pre-http/1.1 host = " << (cfg.default_hostname.empty() ? "disabled" : cfg.default_hostname)
      << "; server id = " << (cfg.server_string.empty() ? "disabled" : cfg.server_string)
      << "; i/o block size = " << cfg.io_block_size;
  }

  /**
   *  \brief This is the HTTP protocol driver class.
   */
  class daemon : public async_streambuf
  {
    input_buffer        _inbuf;
    output_buffer       _outbuf;

    byte_range get_input_buffer();
    scatter_vector const & get_output_buffer();
    void append_input(size_t);
    void drop_output(size_t);

  public:
    struct acceptor
    {
      void operator() (shared_socket const & s) const;
    };

    void operator()();

    daemon();
    ~daemon();

    static configuration _config;

  private:
    // transaction state

    Request                                     _request;
    bool                                        _use_persistent_connection;
    int                                         _data_fd;
    byte_buffer                                 _data_buffer;

    // state machine

    enum state_t
      { READ_REQUEST_LINE
      , READ_REQUEST_HEADER
      , READ_REQUEST_BODY
      , SETUP_RESPONSE
      , WRITE_RESPONSE
      , TERMINATE
      };
    state_t _state;
    typedef state_t (daemon::*state_fun_t)();
    static state_fun_t const state_handlers[TERMINATE];

    void reset();

    state_t get_request_line();
    state_t get_request_header();
    state_t get_request_body();
    state_t setup_response();
    state_t write_response();
    state_t restart();

    // standard responses

    state_t protocol_error(const std::string& message);
    state_t moved_permanently(const std::string& path);
    state_t file_not_found(std::string const &);
    void    not_modified();

    void log_access();
  };

} // namespace http

#endif // HTTPD_REQUESTHANDLER_HPP
