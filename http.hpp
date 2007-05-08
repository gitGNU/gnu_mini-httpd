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
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

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
    Request() : startup_time(0), major_version(0u), minor_version(0u), port(0u)
    {
    }

    std::string                         method;
    URL                                 url;
    std::time_t                         startup_time;
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

    std::size_t parse_request_line(char const *, char const *);
    std::size_t parse_host_header(char const *, char const *);
    std::size_t parse_if_modified_since_header(char const *, char const *);

    bool supports_persistent_connection() const;
  };

  std::string urldecode(std::string const & input);
  std::string to_rfcdate(std::time_t ts);
  std::string escape_html_specials(std::string const & input);

  // Split an HTTP header into the header's name and data part.

  std::size_t parse_header(std::string& name, std::string& data, char const *, char const *);

  /**
   *  \brief Find the end of an RFC2616 line.
   *
   *  Find the end of an RFC2616. RFC lines may continue over more than one
   *  text line if \c CRLF is followed by more whitespace.
   *
   *  \param  begin begin of input range
   *  \param  end   end of input range
   *  \return Begin of next RCF line or \p end to signify an incomplete buffer.
   */
  template <class Iterator>
  inline Iterator find_next_line(Iterator begin, Iterator end)
  {
    for (/**/; begin != end; ++begin)
    {
      Iterator p( begin );
      if (*begin == '\r')
      {
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

} // namespace http

#endif // MINI_HTTPD_HTTP_HPP
