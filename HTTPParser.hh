/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPPARSER_HH
#define HTTPPARSER_HH

#include <string>
#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/utility/chset.hpp>

class HTTPParser
    {
  public:
    typedef char                             value_t;
    typedef const value_t*                   iterator_t;
    typedef spirit::chlit<value_t>           chlit_t;
    typedef spirit::range<value_t, value_t>  range_t;
    typedef spirit::chset<value_t>           chset_t;
    typedef spirit::rule<iterator_t>         rule_t;
    typedef spirit::parse_info<iterator_t>   parse_info_t;

    HTTPParser();

    std::string res_method, res_host, res_path, res_query;
    unsigned int res_port;
    std::string res_name, res_data;

    range_t CHAR;
    chlit_t HT, LF, CR, SP;

    rule_t CRLF;
    rule_t mark, reserved, unreserved, escaped, pchar,
        param, segment, segments, abs_path, domainlabel,
        toplabel, hostname, IPv4address, Host, Method,
        uric, Query, http_URL, Request_URI, HTTP_Version,
        Request_Line, CTL, TEXT, separators, token, LWS,
        quoted_pair, qdtext, quoted_string, field_content,
        field_value, field_name, Header, Host_Header;

    static bool have_complete_header_line(std::string::const_iterator begin, std::string::const_iterator end)
        {
        std::string::const_iterator p;
        for (p = begin; end - p > 2; ++p)
            {
            if (*p == '\r' && p[1] == '\n' && !(p[2] == '\t' || p[2] == ' '))
                break;
            }
        if (end - p > 1)
            return true;
        else
            return false;
        }
    };

extern HTTPParser http_parser;

#endif // !defined(HTTPPARSER_HH)
