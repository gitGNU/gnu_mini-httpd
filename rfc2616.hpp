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

#ifndef MINI_HTTPD_RFC2616_HPP
#define MINI_HTTPD_RFC2616_HPP

#if 0                           // enable spirit debugging
#  define BOOST_SPIRIT_DEBUG
#  define BOOST_SPIRIT_DEBUG_OUT std::cerr
#endif

#include <boost/spirit.hpp>

namespace rfc2616               // http://www.faqs.org/rfcs/rfc2616.html
{
  // http data types and parsers for them

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

  struct wday_parser : public spirit::symbols<int>
  {
    wday_parser()
    {
      add ("Sun", 0)("Mon", 1)("Tue", 2)("Wed", 3)
          ("Thu", 4)("Fri", 5)("Sat", 6);
    }
  };

  wday_parser const     wday_p;

  struct weekday_parser : public spirit::symbols<int>
  {
    weekday_parser()
    {
      add ("Sunday", 0)("Monday", 1)("Tuesday", 2)("Wednesday", 3)
          ("Thursday", 4)("Friday", 5)("Saturday", 6);
    }
  };

  weekday_parser const  weekday_p;

  struct month_parser : public spirit::symbols<int>
  {
    month_parser()
    {
      add ("Jan", 0)("Feb", 1)("Mar", 2)("Apr", 3)
          ("May", 4)("Jun", 5)("Jul", 6)("Aug", 7)
          ("Sep", 8)("Oct", 9)("Nov", 10)("Dec", 11);
    }
  };

  month_parser const    month_p;

} // namespace rfc2616

#endif // MINI_HTTPD_RFC2616_HPP
