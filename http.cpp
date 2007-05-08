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

#include "rfc2616.hpp"
#include "http.hpp"
#include <stdexcept>
#include <cstring>

using namespace std;
using namespace rfc2616;
using namespace rfc2616::spirit;

bool http::Request::supports_persistent_connection() const
{
  if (strcasecmp(connection.c_str(), "close") == 0)
    return false;
  if (strcasecmp(connection.c_str(), "keep-alive") == 0)
    return true;
  if (major_version >= 1 && minor_version >= 1)
    return true;
  else
    return false;
}

std::size_t http::parse_header(std::string & name, std::string & data, char const * b, char const * e)
{
  BOOST_ASSERT(b < e);
  rule<> const header_p
    (  field_name_p[assign_a(name)]
    >> *lws_p
    >> ":"
    >> *lws_p
    >> !( field_value_p[assign_a(data)] )
    >> crlf_p
    );
  BOOST_SPIRIT_DEBUG_NODE(header_p);
  parse_info<> info( parse(b, e, header_p) );
  return info.full ? info.length : 0u;
}

std::size_t http::Request::parse_request_line(char const * b, char const * e)
{
  BOOST_ASSERT(b < e);
  rule<> const url_p
    (  nocase_d["http://"][assign_a(method, "HTTP")]
    >> host_p[assign_a(url.host)]
    >> !( ':' >> uint_p[assign_a(url.port)] )
    >> !( abs_path_p[assign_a(url.path)] >> !( '?' >> query_p[assign_a(url.query)] ) )
    );
  rule<> const uri_p
    (
      url_p | abs_path_p[assign_a(url.path)] >> !( '?' >> query_p[assign_a(url.query)] )
    );
  rule<> const version_p
    (
      nocase_d["http/"] >> uint_p[assign_a(major_version)] >> '.' >> uint_p[assign_a(minor_version)]
    );
  rule<> const request_line_p
    (
      method_p[assign_a(method)] >> sp_p >> uri_p >> sp_p >> version_p >> crlf_p
    );
  BOOST_SPIRIT_DEBUG_NODE(url_p);
  BOOST_SPIRIT_DEBUG_NODE(uri_p);
  BOOST_SPIRIT_DEBUG_NODE(version_p);
  BOOST_SPIRIT_DEBUG_NODE(request_line_p);
  parse_info<> info( parse(b, e, request_line_p) );
  return info.full ? info.length : 0u;
}

size_t http::Request::parse_host_header(char const * b, char const * e)
{
  BOOST_ASSERT(b < e);
  parse_info<> info
    (
      parse(b, e, host_p[assign_a(host)] >> !( ":" >> uint_p[assign_a(port)] ))
    );
  return info.full ? info.length : 0u;
}

size_t http::Request::parse_if_modified_since_header(char const * b, char const * e)
{
  BOOST_ASSERT(b < e);
  struct tm ts;
  memset(&ts, 0, sizeof(ts));
  rule<> const time_p
    (
      uint_p[assign_a(ts.tm_hour)] >> ":" >> uint_p[assign_a(ts.tm_min)] >> ":" >> uint_p[assign_a(ts.tm_sec)]
    );
  rule<> const date1_p
    (
      uint_p[assign_a(ts.tm_mday)] >> sp_p >> month_p[assign_a(ts.tm_mon)] >> sp_p >> uint_p[assign_a(ts.tm_year)]
    );
  rule<> const date2_p
    (
      uint_p[assign_a(ts.tm_mday)] >> "-" >> month_p[assign_a(ts.tm_mon)] >> "-" >> uint_p[assign_a(ts.tm_year)]
    );
  rule<> const date3_p
    (
      month_p[assign_a(ts.tm_mon)] >> sp_p >> ( uint_p[assign_a(ts.tm_mday)] | ( sp_p >> uint_p[assign_a(ts.tm_mday)] ) )
    );
  rule<> const rfc1123_date_p( wday_p >> "," >> sp_p >> date1_p >> sp_p >> time_p >> sp_p >> "GMT" );
  rule<> const rfc850_date_p( weekday_p >> "," >> sp_p >> date2_p >> sp_p >> time_p >> sp_p >> "GMT" );
  rule<> const asctime_date_p( wday_p >> sp_p >> date3_p >> sp_p >> time_p >> sp_p >> uint_p[assign_a(ts.tm_year)] );
  BOOST_SPIRIT_DEBUG_NODE(time_p);
  BOOST_SPIRIT_DEBUG_NODE(date1_p);
  BOOST_SPIRIT_DEBUG_NODE(date2_p);
  BOOST_SPIRIT_DEBUG_NODE(date3_p);
  BOOST_SPIRIT_DEBUG_NODE(rfc1123_date_p);
  BOOST_SPIRIT_DEBUG_NODE(rfc850_date_p);
  BOOST_SPIRIT_DEBUG_NODE(asctime_date_p);
  parse_info<> info( parse(b, e, rfc1123_date_p | rfc850_date_p | asctime_date_p) );
  if (!info.full) return 0u;

  // Make sure the tm structure contains no nonsense.

  if (ts.tm_year < 1970 || ts.tm_hour > 23 || ts.tm_min > 59 || ts.tm_sec > 59)
    return 0;

  switch(ts.tm_mon)
  {
    case 0: case 2: case 4:
    case 6: case 7: case 9:
    case 11:
      if (ts.tm_mday > 31)
        return 0;
      break;
    case 1:
      if (ts.tm_year % 4 == 0 && ts.tm_year % 100 != 0)
      {
        if (ts.tm_mday > 29)
          return 0;
      }
      else
      {
        if (ts.tm_mday > 28)
          return 0;
      }
      break;
    case 3: case 5: case 8:
    case 10:
      if (ts.tm_mday > 30)
        return 0;
      break;
    default:
      throw logic_error("The month in 'struct tm' is screwed badly.");
  }

  // The date is fine. Now turn it into a time_t.

  ts.tm_year -= 1900;
  if_modified_since = mktime(&ts) - timezone;

  // Done.

  return info.length;
}

/**
 * Throwing an exception in case the URL contains a syntax error may
 * seem a bit harsh, but consider that this should never happen as all
 * URL stuff we see went through the HTTP parser, who would not let
 * the URL pass if it contained an error.
 */

std::string http::urldecode(std::string const & input)
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

std::string http::escape_html_specials(std::string const & input)
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

std::string http::to_rfcdate(std::time_t t)
{
  char buffer[64];
  size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
  if (len == 0 || len >= sizeof(buffer))
    throw length_error("strftime() failed because an internal buffer is too small!");
  return buffer;
}
