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

#include "HTTPParser.hpp"
#include <stdexcept>

using namespace std;
using namespace boost::spirit;
using namespace rfc2616;

/**
 *  \todo A lot of this code is unnecessary with the latest Spirit
 *        version. The parsers need a lot of cleanup.
 */

// Proxy class that will assign a parser result via a pointer to classT.

template<typename classT>
class var_assign
{
public:
  var_assign(classT** i) : instance(i) { }
  void operator() (const classT& val) const
  {
    *instance = val;
  }
private:
  classT** instance;
};

// This specialized version for std::string is necessary because the
// parser will give me a pointer to the begin and the end of the
// string, not a std::string directly.

template<>
class var_assign<string>
{
public:
  var_assign(string** i) : instance(i) { }
  void operator() (char const * first, char const * last) const
  {
    **instance = string(first, last - first);
  }
private:
  string** instance;
};

// Proxy class that will assign a parser result via a pointer to a
// member variable and a pointer to the class instance.

template<typename classT, typename memberT>
class member_assign
{
public:
  member_assign(classT** i, memberT classT::* m)
            : instance(i), member(m)
  {
  }
  void operator() (const memberT& val) const
  {
    (*instance)->*member = val;
  }
private:
  classT** instance;
  memberT classT::* member;
};

// One more specialized version for a memberT of std::string.

template<typename classT>
class member_assign<classT, string>
{
public:
  member_assign(classT** i, string classT::* m)
            : instance(i), member(m)
  {
  }
  void operator() (char const * first, char const * last) const
  {
    (*instance)->*member = string(first, last - first);
  }
private:
  classT** instance;
  string classT::* member;
};

// Function expression that will return an apropriate instance of
// var_assign.

template<typename classT>
inline var_assign<classT> assign(classT** dst)
{
  return var_assign<classT>(dst);
}

// Function expression that will return an apropriate instance of
// member_assign.

template<typename classT, typename memberT>
inline member_assign<classT,memberT> assign(classT** dst, memberT classT::* var)
{
  return member_assign<classT,memberT>(dst, var);
}

// The parser.

HTTPParser::HTTPParser() : name_ptr(0), data_ptr(0), url_ptr(0), req_ptr(0)
{
  http_URL      = nocase_d["http://"]
                  >> host_p[assign(&url_ptr, &URL::host)]
                  >> !( ':' >> uint_p[assign(&url_ptr, &URL::port)] )
                  >> !( abs_path_p[assign(&url_ptr, &URL::path)]
                  >> !( '?' >> query_p[assign(&url_ptr, &URL::query)] ) );
  Request_URI   = http_URL | abs_path_p[assign(&url_ptr, &URL::path)]
                  >> !( '?' >> query_p[assign(&url_ptr, &URL::query)] );
  HTTP_Version  = nocase_d["http/"] >> uint_p[assign(&req_ptr, &HTTPRequest::major_version)]
                  >> '.' >> uint_p[assign(&req_ptr, &HTTPRequest::minor_version)];
  Request_Line  = method_p[assign(&req_ptr, &HTTPRequest::method)] >> sp_p >> Request_URI >> sp_p >> HTTP_Version >> crlf_p;

  Header        = ( field_name_p[assign(&name_ptr)] >> *lws_p >> ":" >> *lws_p
                  >> !( field_value_p[assign(&data_ptr)] ) ) >> crlf_p;
  Host_Header   = host_p[assign(&req_ptr, &HTTPRequest::host)]
                  >> !( ":" >> uint_p[assign(&req_ptr, &HTTPRequest::port)] );
  weekday       = "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday";
  wkday         = "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun";
  month.add       ("Jan", 0)("Feb", 1)("Mar", 2)("Apr", 3)("May", 4)("Jun", 5)
                  ("Jul", 6)("Aug", 7)("Sep", 8)("Oct", 9)("Nov", 10)("Dec", 11);
  time          = uint_p[assign(tm_date.tm_hour)] >> ":" >> uint_p[assign(tm_date.tm_min)] >>
                  ":" >> uint_p[assign(tm_date.tm_sec)];
  date1         = uint_p[assign(tm_date.tm_mday)] >> sp_p >> month[assign(tm_date.tm_mon)] >>
                  sp_p >> uint_p[assign(tm_date.tm_year)];
  date2         = uint_p[assign(tm_date.tm_mday)] >> "-" >> month[assign(tm_date.tm_mon)] >>
                  "-" >> uint_p[assign(tm_date.tm_year)];
  date3         = month[assign(tm_date.tm_mon)] >> sp_p >>
                  ( uint_p[assign(tm_date.tm_mday)] | ( sp_p >> uint_p[assign(tm_date.tm_mday)] ) );
  rfc1123_date  = wkday >> "," >> sp_p >> date1 >> sp_p >> time >> sp_p >> "GMT";
  rfc850_date   = weekday >> "," >> sp_p >> date2 >> sp_p >> time >> sp_p >> "GMT";
  asctime_date  = wkday >> sp_p >> date3 >> sp_p >> time >> sp_p >> uint_p[assign(tm_date.tm_year)];
  HTTP_date     = rfc1123_date | rfc850_date | asctime_date;
  If_Modified_Since_Header = HTTP_date;

  BOOST_SPIRIT_DEBUG_NODE(http_URL);
  BOOST_SPIRIT_DEBUG_NODE(Request_URI);
  BOOST_SPIRIT_DEBUG_NODE(HTTP_Version);
  BOOST_SPIRIT_DEBUG_NODE(Request_Line);
  BOOST_SPIRIT_DEBUG_NODE(Header);
  BOOST_SPIRIT_DEBUG_NODE(Host_Header);
  BOOST_SPIRIT_DEBUG_NODE(time);
  BOOST_SPIRIT_DEBUG_NODE(date1);
  BOOST_SPIRIT_DEBUG_NODE(date2);
  BOOST_SPIRIT_DEBUG_NODE(date3);
  BOOST_SPIRIT_DEBUG_NODE(rfc1123_date);
  BOOST_SPIRIT_DEBUG_NODE(rfc850_date);
  BOOST_SPIRIT_DEBUG_NODE(asctime_date);
  BOOST_SPIRIT_DEBUG_NODE(HTTP_date);
  BOOST_SPIRIT_DEBUG_NODE(If_Modified_Since_Header);

  // Initialize the global variables telling us our time zone and
  // stuff. We'll need that to turn the GMT dates in the headers to
  // local time.

  tzset();
  }


bool HTTPParser::supports_persistent_connection(const HTTPRequest& request)
{
  if (strcasecmp(request.connection.c_str(), "close") == 0)
    return false;
  if (strcasecmp(request.connection.c_str(), "keep-alive") == 0 && !request.keep_alive.empty())
    return true;
  if (request.major_version >= 1 && request.minor_version >= 1)
    return true;
  else
    return false;
}

size_t HTTPParser::parse_header(string& name, string& data, const string& input) const
{
  name_ptr = &name;
  data_ptr = &data;

  parse_info_t info = parse(input.data(), input.data() + input.size(), Header);
  if (info.hit)
    return info.length;
  else
    return 0;
}

size_t HTTPParser::parse_request_line(HTTPRequest& request, const string& input) const
{
  req_ptr = &request;
  url_ptr = &request.url;

  parse_info_t info = parse(input.data(), input.data() + input.size(), Request_Line);
  if (info.hit)
    return info.length;
  else
    return 0;
}

size_t HTTPParser::parse_host_header(HTTPRequest& request, const std::string& input) const
{
  req_ptr = &request;

  parse_info_t info = parse(input.data(), input.data() + input.size(), Host_Header);
  if (info.hit)
    return info.length;
  else
    return 0;
}

size_t HTTPParser::parse_if_modified_since_header(HTTPRequest& request, const std::string& input) const
{
  memset(&tm_date, 0, sizeof(tm_date));

  parse_info_t info = parse(input.data(), input.data() + input.size(), If_Modified_Since_Header);
  if (!info.hit)
    return 0;

  // Make sure the tm structure contains no nonsense.

  if (tm_date.tm_year < 1970 || tm_date.tm_hour > 23 || tm_date.tm_min > 59 || tm_date.tm_sec > 59)
    return 0;

  switch(tm_date.tm_mon)
  {
    case 0: case 2: case 4:
    case 6: case 7: case 9:
    case 11:
      if (tm_date.tm_mday > 31)
        return 0;
      break;
    case 1:
      if (tm_date.tm_year % 4 == 0 && tm_date.tm_year % 100 != 0)
      {
        if (tm_date.tm_mday > 29)
          return 0;
      }
      else
      {
        if (tm_date.tm_mday > 28)
          return 0;
      }
      break;
    case 3: case 5: case 8:
    case 10:
      if (tm_date.tm_mday > 30)
        return 0;
      break;
    default:
      throw logic_error("The month in HTTPParser::tm_date is screwed badly.");
  }

  // The date is fine. Now turn it into a time_t.

  tm_date.tm_year -= 1900;
  request.if_modified_since = mktime(&tm_date) - timezone;

  // Done.

  return info.length;
}

// The global parser instance.

HTTPParser const        http_parser;
