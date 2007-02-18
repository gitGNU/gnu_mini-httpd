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

#ifndef HTTPPARSER_HH
#define HTTPPARSER_HH

#if 0                           // enable spirit debugging
#  define BOOST_SPIRIT_DEBUG
#  define BOOST_SPIRIT_DEBUG_OUT std::cerr
#endif

#include <string>
#include <ctime>
#include <boost/spirit.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/symbols/symbols.hpp>
#include <boost/noncopyable.hpp>
#include "HTTPRequest.hpp"

namespace rfc2616               // http://www.faqs.org/rfcs/rfc2616.html
{
  namespace spirit = boost::spirit;

  spirit::chlit<> const ht_p(9);        // '\t'
  spirit::chlit<> const lf_p(10);       // '\n'
  spirit::chlit<> const cr_p(13);       // '\r'
  spirit::chlit<> const sp_p(32);       // ' '
  spirit::range<> const char_p(0, 127);
  spirit::chset<> const mark_p("-_.!~*'()");
  spirit::chset<> const reserved_p(";/?:@&=+$,");
  spirit::chset<> const separators_p("()<>@,;:\\\"/[]?={}\x20\x09");

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

}


class HTTPParser : private boost::noncopyable
{
public:
  HTTPParser();

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

  static bool supports_persistent_connection(const HTTPRequest& request);

  // Parse an HTTP request line.

  size_t parse_request_line(HTTPRequest& request, const std::string& input) const;

  // Split an HTTP header into the header's name and data part.

  size_t parse_header(std::string& name, std::string& data, const std::string& input) const;

  // Parse various headers.

  size_t parse_host_header(HTTPRequest& request, const std::string& input) const;
  size_t parse_if_modified_since_header(HTTPRequest& request, const std::string& input) const;

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
  mutable HTTPRequest* req_ptr;
  mutable struct tm    tm_date;
};

extern HTTPParser const http_parser;

#endif // !defined(HTTPPARSER_HH)
