/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPPARSER_HH
#define HTTPPARSER_HH

#include <string>
#include <ctime>
#include <boost/spirit/spirit_core.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/symbols/symbols.hpp>

#include "HTTPRequest.hh"

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

#endif // !defined(HTTPPARSER_HH)
