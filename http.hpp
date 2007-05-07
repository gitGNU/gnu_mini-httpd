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

#ifndef MINI_HTTPD_HTTP_HPP
#define MINI_HTTPD_HTTP_HPP

#include <string>
#include <ctime>
#include <stdexcept>
#include <utility>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <boost/spirit.hpp>
#include <boost/noncopyable.hpp>

namespace http                  // http://www.faqs.org/rfcs/rfc2616.html
{
  struct URL
  {
    URL() : port(0u) { }

    std::string         host;
    boost::uint16_t     port;
    std::string         path;
    std::string         query;
  };

  // This class contains all relevant information in an HTTP request.

  struct Request
  {
    Request() : port(0u) { }

    std::string                         method;
    URL                                 url;
    std::time_t                         start_up_time;
    unsigned int                        major_version;
    unsigned int                        minor_version;
    std::string                         host;
    boost::uint16_t                     port;
    std::string                         connection;
    std::string                         keep_alive;
    boost::optional<std::time_t>        if_modified_since;
    std::string                         user_agent;
    std::string                         referer;
    boost::optional<unsigned int>       status_code;
    boost::optional<std::size_t>        object_size;
  };

  std::string urldecode(std::string const & input);
  std::string to_rfcdate(std::time_t ts);
  std::string escape_html_specials(std::string const & input);

  class parser : private boost::noncopyable
  {
  public:
    parser();

    // Find the end of an RFC-style line (and consequently the begin of
    // the next one). Lines may be continued by beginning the next line
    // with a whitespace. Return iterator positioned _after_ the CRLF,
    // or end to signal an incomplete line.

    template <class iteratorT>
    static inline iteratorT find_next_line(iteratorT begin, iteratorT end)
    {
      for (/**/; begin != end; ++begin)
      {
        if (*begin == '\r')
        {
          iteratorT p( begin );
          if (++p == end) return end;
          if (*p == '\n')
          {
            ++begin;
            if ( ++p == end || (*p != ' ' && *p != '\t') )
              return p;
          }
        }
      }
      return begin;
    }

    // Does the given request allow a persistent connection?

    static bool supports_persistent_connection(Request const &);

    // Parse an HTTP request line.

    std::size_t parse_request_line(Request &, char const *, char const *) const;

    // Split an HTTP header into the header's name and data part.

    std::size_t parse_header(std::string& name, std::string& data, char const *, char const *) const;

    // Parse various headers.

    std::size_t parse_host_header(Request &, char const *, char const *) const;
    std::size_t parse_if_modified_since_header(Request &, char const *, char const *) const;

  private:
    typedef char                                    value_t;
    typedef const value_t*                          iterator_t;
    typedef boost::spirit::chlit<value_t>           chlit_t;
    typedef boost::spirit::range<value_t>           range_t;
    typedef boost::spirit::chset<value_t>           chset_t;
    typedef boost::spirit::rule<>                   rule_t;
    typedef boost::spirit::symbols<int, value_t>    symbol_t;
    typedef boost::spirit::parse_info<iterator_t>   parse_info_t;

    rule_t http_URL, Request_URI, HTTP_Version,
      Request_Line, Header, Host_Header,
      date1, date2, date3, time, rfc1123_date, rfc850_date,
      asctime_date, HTTP_date, If_Modified_Since_Header;
    symbol_t weekday, month, wkday;

    // Because the parsers dereference pointers at construction time,
    // we need an layer of indirection to assign the parser's results
    // to the variables the caller has given us.

    mutable std::string* name_ptr;
    mutable std::string* data_ptr;
    mutable URL*         url_ptr;
    mutable Request *    req_ptr;
    mutable struct tm    tm_date;
  };

  extern parser const http_parser;

} // namespace http

#endif // MINI_HTTPD_HTTP_HPP
