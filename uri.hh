/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef URI_HH
#define URI_HH

#include <string>
#include <stdexcept>
#include <boost/spirit/spirit_rule.hpp>

struct URI
    {
    std::string scheme, userinfo, host, opaque_part, ipaddress, port, path, query;
    std::string full;
    };

class uri_parser
    {
  public:
    typedef spirit::rule<const char*> rule_t;
    typedef spirit::range<char, char> range_t;
    typedef spirit::parse_info<char const*> parse_info_t;

    struct error : public std::runtime_error
        {
        error(const char* msg)
                : runtime_error(std::string("Input is not a valid URI: ") + msg)
            {
            }
        };

    uri_parser(URI& uri_param);

    void operator() (const char* buf) const
        {
        parse_info_t result = parse(buf, URI_reference);
        if (!result.full)
            throw error(buf);
        }

    range_t lowalpha, upalpha, digit;
    rule_t alpha, alphanum, hex, escaped, mark, unreserved, pchar,
        param, segment, path_segments, reserved, uric, query,
        fragment, port, IPv4address, toplabel, domainlabel, hostname,
        host, hostport, rel_segment, scheme, uric_no_slash, opaque_part,
        userinfo, server, reg_name, authority, abs_path, net_path,
        rel_path, hier_part, relativeURI, absoluteURI, URI_reference;

  private:
    URI& uri;
    };

#endif // !defined(URI_HH)
