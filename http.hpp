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

#if 0                           // enable spirit debugging
#  define BOOST_SPIRIT_DEBUG
#  define BOOST_SPIRIT_DEBUG_OUT std::cerr
#endif

#include <string>
#include <ctime>
#include <stdexcept>
#include <utility>
#include <boost/cstdint.hpp>
#include <boost/range.hpp>
#include <boost/optional.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/spirit/phoenix/binders.hpp>
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

namespace http                  // http://www.faqs.org/rfcs/rfc2616.html
{
  namespace spirit = boost::spirit;

  // core types

  typedef char const *                          char_pointer;
  typedef boost::uint16_t                       port_t;
  typedef boost::iterator_range<char_pointer>   buffer_t;

  inline buffer_t drop_front(buffer_t const & buf, buffer_t::size_type n)
  {
    BOOST_ASSERT(n <= buf.size());
    return buffer_t(buf.begin() + n, buf.end());
  }

  struct Version : public std::pair<unsigned int, unsigned int>
  {
    typedef std::pair<unsigned int, unsigned int> pair;

    Version()                               : pair(0u, 0u)              { }
    Version(unsigned int x, unsigned int y) : pair(x,   y)              { }
    Version(pair p)                         : pair(p)                   { }

    unsigned int major() const  { return first; }
    unsigned int minor() const  { return second; }
  };

  struct version_closure : public spirit::closure<version_closure, Version>
  {
    member1 val;
  };

  spirit::rule<spirit::scanner<>, version_closure::context_t> const version_p
    (   spirit::str_p("HTTP")
    >>  spirit::uint_p[ phoenix::bind(&Version::first)(version_p.val) ]
    >>  spirit::ch_p('/')
    >>  spirit::uint_p[ phoenix::bind(&Version::second)(version_p.val) ]
    );

  struct Uri
  {
    enum method_t { http };

    method_t    method;
    buffer_t    host;
    port_t      port;           // 0 is invalid
    buffer_t    path;
    buffer_t    query;
  };


  // http data types and parsers for them

  spirit::chlit<> const ht_p(9);        // '\t'
  spirit::chlit<> const lf_p(10);       // '\n'
  spirit::chlit<> const cr_p(13);       // '\r'
  spirit::chlit<> const sp_p(32);       // ' '
  spirit::range<> const char_p(0, 127);
  spirit::chset<> const mark_p("-_.!~*'()");
  spirit::chset<> const reserved_p(";/?:@&=+$,");
  spirit::chset<> const separators_p("()<>@,;:\\\"/[]?={}\x20\x09");

  struct Request_line
  {
    enum method_t { get, head };

    method_t    method;
    Uri         uri;
    Version     version;
  };

  struct URL
  {
    URL() : port(0u) { }

    std::string   host;
    port_t        port;
    std::string   path;
    std::string   query;
  };


  // This class contains all relevant information in an HTTP request.

  struct Request
  {
    Request() : port(0u) { }

    std::string                      method;
    URL                              url;
    time_t                           start_up_time;
    unsigned int                     major_version;
    unsigned int                     minor_version;
    std::string                      host;
    unsigned int                     port;
    std::string                      connection;
    std::string                      keep_alive;
    boost::optional<time_t>          if_modified_since;
    std::string                      user_agent;
    std::string                      referer;
    boost::optional<unsigned int>    status_code;
    boost::optional<size_t>          object_size;
  };


#define GEN_PARSER(NAME, GRAMMAR)                                       \
  struct NAME ## _parser : public spirit::grammar<NAME ## _parser>      \
  {                                                                     \
    NAME ## _parser() { }                                               \
                                                                        \
    template<typename scannerT>                                         \
    struct definition                                                   \
    {                                                                   \
      spirit::rule<scannerT>    NAME;                                   \
                                                                        \
      definition(NAME ## _parser const &)                               \
      {                                                                 \
        using namespace spirit;                                         \
        NAME = GRAMMAR;                                                 \
        BOOST_SPIRIT_DEBUG_NODE(NAME);                                  \
      }                                                                 \
                                                                        \
      spirit::rule<scannerT> const & start() const { return NAME; }     \
    };                                                                  \
  };                                                                    \
                                                                        \
  NAME ## _parser const NAME ## _p;

  GEN_PARSER(crlf,              cr_p >> lf_p)
  GEN_PARSER(unreserved,        alnum_p | mark_p)
  GEN_PARSER(escaped,           '%' >> xdigit_p >> xdigit_p)
  GEN_PARSER(pchar,             unreserved_p | escaped_p | chset_p(":@&=+$,"))
  GEN_PARSER(param,             *pchar_p)
  GEN_PARSER(segment,           *pchar_p >> *( ';' >> param_p ))
  GEN_PARSER(segments,          segment_p >> *( '/' >> segment_p ))
  GEN_PARSER(abs_path,          '/' >> segments_p)
  GEN_PARSER(domainlabel,       alnum_p >> *( !ch_p('-') >> alnum_p ))
  GEN_PARSER(toplabel,          alpha_p >> *( !ch_p('-') >> alnum_p ))
  GEN_PARSER(hostname,          *( domainlabel_p >> '.' ) >> toplabel_p >> !ch_p('.'))
  GEN_PARSER(ipv4address,       +digit_p >> '.' >> +digit_p >> '.' >> +digit_p >> '.' >> +digit_p)
  GEN_PARSER(host,              hostname_p | ipv4address_p)
  GEN_PARSER(ctl,               range_p(0, 31) | ch_p(127))
  GEN_PARSER(text,              anychar_p - ctl_p)
  GEN_PARSER(token,             +( char_p - ( ctl_p | separators_p ) ))
  GEN_PARSER(lws,               !crlf_p >> +( sp_p | ht_p ))
  GEN_PARSER(quoted_pair,       '\\' >> char_p)
  GEN_PARSER(qdtext,            anychar_p - '"')
  GEN_PARSER(quoted_string,     '"' >> *(qdtext_p | quoted_pair_p ) >> '"')
  GEN_PARSER(field_content,     +text_p | ( token_p | separators_p | quoted_string_p ))
  GEN_PARSER(field_value,       *( field_content_p | lws_p ))
  GEN_PARSER(field_name,        token_p)
  GEN_PARSER(method,            token_p)
  GEN_PARSER(uric,              reserved_p | unreserved_p | escaped_p)
  GEN_PARSER(query,             *uric_p)

#undef GEN_PARSER

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

    size_t parse_request_line(Request &, std::string const & input) const;

    // Split an HTTP header into the header's name and data part.

    size_t parse_header(std::string& name, std::string& data, std::string const & input) const;

    // Parse various headers.

    size_t parse_host_header(Request &, std::string const & input) const;
    size_t parse_if_modified_since_header(Request &, std::string const & input) const;

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


  /**
   * Throwing an exception in case the URL contains a syntax error may
   * seem a bit harsh, but consider that this should never happen as all
   * URL stuff we see went through the HTTP parser, who would not let
   * the URL pass if it contained an error.
   */

  inline std::string urldecode(std::string const & input)
  {
    std::string url = input;
    for(std::string::iterator i = url.begin(); i != url.end(); ++i)
    {
      if (*i == '+')
        *i = ' ';
      else if(*i == '%')
      {
        unsigned char c;
        std::string::size_type start = i - url.begin();

        if (++i == url.end())
          throw std::runtime_error("Invalid encoded character in URL!");

        if (*i >= '0' && *i <= '9')
          c = *i - '0';
        else if (*i >= 'a' && *i <= 'f')
          c = *i - 'a' + 10;
        else if (*i >= 'A' && *i <= 'F')
          c = *i - 'A' + 10;
        else
          throw std::runtime_error("Invalid encoded character in URL!");
        c = c << 4;

        if (++i == url.end())
          throw std::runtime_error("Invalid encoded character in URL!");

        if (*i >= '0' && *i <= '9')
          c += *i - '0';
        else if (*i >= 'a' && *i <= 'f')
          c += *i - 'a' + 10;
        else if (*i >= 'A' && *i <= 'F')
          c += *i - 'A' + 10;
        else
          throw std::runtime_error("Invalid encoded character in URL!");

        url.replace(start, 3, 1, c);
      }
    }
    return url;
  }

  inline std::string escape_html_specials(std::string const & input)
  {
    std::string tmp = input;
    for (std::string::size_type pos = 0; pos <= tmp.size(); ++pos)
    {
      switch(tmp[pos])
      {
        case '<':
          tmp.replace(pos, 1, "&lt;");
          pos += 3;
          break;
        case '>':
          tmp.replace(pos, 1, "&gt;");
          pos += 3;
          break;
        case '&':
          tmp.replace(pos, 1, "&amp;");
          pos += 4;
          break;
      }
    }
    return tmp;
  }

} // namespace http

#endif // MINI_HTTPD_HTTP_HPP
