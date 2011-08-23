/*
 * Copyright (c) 2001-2011 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPPARSER_HH_INCLUDED
#define HTTPPARSER_HH_INCLUDED

#include <string>
#include <ctime>
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_chset.hpp>
#include <boost/spirit/include/classic_symbols.hpp>

#include "HTTPRequest.hh"

namespace spirit = boost::spirit::classic;

class HTTPParser
{
public:
  explicit HTTPParser();

  // This function will check whether the provided string is a
  // complete HTTP header, taking continuation lines into account
  // and all.

  static bool have_complete_header_line(const std::string& input);

  // Does the given request allow a persistent connection?

  static bool supports_persistent_connection(const HTTPRequest& request);

  // Parse an HTTP request line.

  size_t parse_request_line(HTTPRequest& request, const std::string& input) const;

  // Split an HTTP header into the header's name and data part.

  size_t parse_header(std::string& name, std::string& data, const std::string& input) const;

  // Parse various headers.

  size_t parse_host_header(HTTPRequest& request, const std::string& input) const;
  size_t parse_if_modified_since_header(HTTPRequest& request, const std::string& input) const;

private:                      // Don't copy me.
  HTTPParser(const HTTPParser&);
  HTTPParser& operator= (const HTTPParser&);

private:
  typedef char                             value_t;
  typedef const value_t*                   iterator_t;
  typedef spirit::chlit<value_t>           chlit_t;
  typedef spirit::range<value_t>           range_t;
  typedef spirit::chset<value_t>           chset_t;
  typedef spirit::rule<>                   rule_t;
  typedef spirit::symbols<int, value_t>    symbol_t;
  typedef spirit::parse_info<iterator_t>   parse_info_t;

  range_t CHAR;
  chlit_t HT, LF, CR, SP;
  rule_t CRLF, mark, reserved, unreserved, escaped, pchar,
  param, segment, segments, abs_path, domainlabel,
  toplabel, hostname, IPv4address, Host, Method,
  uric, Query, http_URL, Request_URI, HTTP_Version,
  Request_Line, CTL, TEXT, separators, token, LWS,
  quoted_pair, qdtext, quoted_string, field_content,
  field_value, field_name, Header, Host_Header,
  date1, date2, date3, time, rfc1123_date, rfc850_date,
  asctime_date, HTTP_date, If_Modified_Since_Header;
  symbol_t weekday, month, wkday;

private:
  // Because the parsers dereference pointers at construction time,
  // we need an layer of indirection to assign the parser's results
  // to the variables the caller has given us.

  mutable std::string* name_ptr;
  mutable std::string* data_ptr;
  mutable URL*         url_ptr;
  mutable HTTPRequest* req_ptr;
  mutable struct tm    tm_date;
};

extern const HTTPParser http_parser;

#endif // HTTPPARSER_HH_INCLUDED
