/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "uri.hh"
#include <boost/spirit/attr/semantics.hpp>

using namespace std;
using namespace spirit;

uri_parser::uri_parser(URI& uri_param)
        : lowalpha ('a', 'z'), upalpha('a', 'z'), digit('0', '9'), uri(uri_param)
    {
    alpha         = lowalpha | upalpha;
    alphanum      = alpha | digit;
    hex           = range_t('a', 'f') | range_t('A', 'F') | digit;
    escaped       = '%' >> hex >> hex;
    mark          = ch_p('-') | '_' | '.' | '!' | '~' | '*' | '\'' | '(' | ')';
    unreserved    = alphanum | mark;
    pchar         = unreserved | escaped | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    param         = *pchar;
    segment       = *pchar >> *( ';' >> param );
    path_segments = segment >> *( '/' >> segment );
    reserved      = ch_p(';') | '/' | '?' | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    uric          = reserved | unreserved | escaped;
    query         = (*uric)[ref(uri.query)];
    fragment      = *uric;
    port          = *digit;
    IPv4address   = +digit >> '.' >> +digit >> '.' >> +digit >> '.' >> +digit;
    toplabel      = alpha >> *( !ch_p('-') >> alphanum );
    domainlabel   = alphanum >> *( !ch_p('-') >> alphanum );
    hostname      = domainlabel >> '.' >> hostname |  toplabel >> !ch_p('.');
    host          = hostname[ref(uri.host)] | IPv4address[ref(uri.ipaddress)];
    hostport      = host >> !( ':' >> port[ref(uri.port)] );
    rel_segment   = +( unreserved | escaped | ';' | '@' | '&' | '=' | '+' | '$' | ',' );
    scheme        = alpha >> *( alpha | digit | '+' | '-' | '.' );
    uric_no_slash = unreserved | escaped | ';' | '?' | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    opaque_part   = uric_no_slash >> *uric;
    userinfo      = *( unreserved | escaped | ';' | ':' | '&' | '=' | '+' | '$' | ',' );
    server        = !( !( userinfo[ref(uri.userinfo)] >> '@' ) >> hostport );
    reg_name      = +( unreserved | escaped | '$' | ',' | ';' | ':' | '@' | '&' | '=' | '+' );
    authority     = server | reg_name;
    abs_path      = ( '/' >> path_segments )[ref(uri.path)];
    net_path      = str_p("//") >> authority >> !abs_path;
    rel_path      = ( rel_segment >> !abs_path )[ref(uri.path)];
    hier_part     = ( net_path | abs_path ) >> !( '?' >> query );
    relativeURI   = ( net_path | abs_path | rel_path ) >> !( '?' >> query );
    absoluteURI   = scheme[ref(uri.scheme)] >> ':' >> ( hier_part | opaque_part[ref(uri.opaque_part)] );
    URI_reference = ( !( absoluteURI | relativeURI ) >> !( '#' >> fragment ) )[ref(uri.full)];

    SPIRIT_DEBUG_RULE(alpha);
    SPIRIT_DEBUG_RULE(alphanum);
    SPIRIT_DEBUG_RULE(hex);
    SPIRIT_DEBUG_RULE(escaped);
    SPIRIT_DEBUG_RULE(mark);
    SPIRIT_DEBUG_RULE(unreserved);
    SPIRIT_DEBUG_RULE(pchar);
    SPIRIT_DEBUG_RULE(param);
    SPIRIT_DEBUG_RULE(segment);
    SPIRIT_DEBUG_RULE(path_segments);
    SPIRIT_DEBUG_RULE(reserved);
    SPIRIT_DEBUG_RULE(uric);
    SPIRIT_DEBUG_RULE(query);
    SPIRIT_DEBUG_RULE(fragment);
    SPIRIT_DEBUG_RULE(port);
    SPIRIT_DEBUG_RULE(IPv4address);
    SPIRIT_DEBUG_RULE(toplabel);
    SPIRIT_DEBUG_RULE(domainlabel);
    SPIRIT_DEBUG_RULE(hostname);
    SPIRIT_DEBUG_RULE(host);
    SPIRIT_DEBUG_RULE(hostport);
    SPIRIT_DEBUG_RULE(rel_segment);
    SPIRIT_DEBUG_RULE(scheme);
    SPIRIT_DEBUG_RULE(uric_no_slash);
    SPIRIT_DEBUG_RULE(opaque_part);
    SPIRIT_DEBUG_RULE(userinfo);
    SPIRIT_DEBUG_RULE(server);
    SPIRIT_DEBUG_RULE(reg_name);
    SPIRIT_DEBUG_RULE(authority);
    SPIRIT_DEBUG_RULE(abs_path);
    SPIRIT_DEBUG_RULE(net_path);
    SPIRIT_DEBUG_RULE(rel_path);
    SPIRIT_DEBUG_RULE(hier_part);
    SPIRIT_DEBUG_RULE(relativeURI);
    SPIRIT_DEBUG_RULE(absoluteURI);
    SPIRIT_DEBUG_RULE(URI_reference);
    }
