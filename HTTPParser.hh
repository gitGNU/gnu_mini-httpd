/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPPARSER_HH
#define HTTPPARSER_HH

#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/utility/chset.hpp>

class HTTPParser
    {
  public:
    typedef char                                  value_type;
    typedef const value_type*                     iterator_type;
    typedef spirit::chlit<value_type>             chlit_type;
    typedef spirit::range<value_type, value_type> range_type;
    typedef spirit::rule<iterator_type>           rule_type;
    typedef spirit::parse_info<iterator_type>     parse_info_type;

    HTTPParser();
    bool operator() (iterator_type begin, iterator_type end)
        {
        return parse(begin, end, Request).full;
        }

    chlit_type SP, CR, LF, HT;
    range_type CHAR;
    rule_type CTL, separators, reserved, mark,Request, Request_Line,
        Method, CRLF, Request_URI, HTTP_Version,
        absoluteURI, hier_part, net_path, abs_path, scheme, authority,
        opaque_part, query, path_segments, segment, param, pchar,
        escaped, server, userinfo, hostport, reg_name, unreserved,
        host, hostname, domainlabel, toplabel, IPv4address, port,
        uric_no_slash, uric, general_header, quoted_pair,
        token, quoted_string, qdtext, request_header, TEXT,
        entity_header, message_body, entity_body, Connection, LWS,
        connection_token, Date, HTTP_date, Pragma, pragma_directive,
        extension_pragma, Trailer, Transfer_Encoding, Upgrade,
        field_name, transfer_coding, transfer_extension, parameter,
        attribute, value, rfc1123_date, rfc850_date, asctime_date,
        wkday, date1, date2, date3, month, time, weekday, product,
        product_version, Host, extension_header, message_header,
        field_value, field_content, extension_method;
    };

#endif // !defined(HTTPPARSER_HH)
