/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPPARSER_HH
#define HTTPPARSER_HH

#include <string>
#include <map>
#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/utility/chset.hpp>

struct HTTPRequest
    {
    typedef std::map<std::string,std::string> header_map_t;

    std::string method;
    unsigned int major_version, minor_version;
    std::string host;
    unsigned int port;
    std::string path;
    std::string query;

    header_map_t header_map;
    };

class HTTPParser : public HTTPRequest
    {
  public:
    HTTPParser();
    bool operator() (const char* begin, const char* end) const;

  private:
    typedef char                                  value_type;
    typedef const value_type*                     iterator_type;
    typedef spirit::chlit<value_type>             chlit_type;
    typedef spirit::range<value_type, value_type> range_type;
    typedef spirit::chset<value_type>             chset_type;
    typedef spirit::rule<iterator_type>           rule_type;
    typedef spirit::parse_info<iterator_type>     parse_info_type;

    chlit_type SP, CR, LF, HT;
    range_type CHAR;
    rule_type CRLF, LWS, Method, mark, reserved, unreserved, escaped,
        pchar, param, segment, path_segments, abs_path, domainlabel,
        toplabel, hostname, IPv4address, host, uric, query,
        Request_URI, HTTP_Version, Request_Line, CTL, separators, token,
        connection_token, Connection, weekday, month, wkday, time, date1,
        date2, date3, rfc1123_date, rfc850_date, asctime_date, HTTP_date,
        Date, quoted_pair, qdtext, quoted_string, value, attribute, parameter,
        transfer_extension, transfer_coding, Transfer_Encoding,
        general_header, Host, request_header, TEXT, field_content,
        field_value, field_name, message_header, entity_body, message_body,
        extension_header, entity_header, Request;

    std::string name, data;
    void msghead_committer(const char* begin, const char* end)
        {

        }
    };

#endif // !defined(HTTPPARSER_HH)
