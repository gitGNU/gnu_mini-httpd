/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
using namespace std;

#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/attr/semantics.hpp>
using namespace spirit;

void print(char const* first, char const* last)
    {
    cout << string(first, last) << endl;
    }

struct URL
    {
    string scheme;
    string host;
    string port;
    string path;
    string query;
    };

inline std::ostream& operator<< (std::ostream& os, const URL& url)
    {
    os << "scheme = '" << url.scheme << "'; "
       << "host = '"   << url.host   << "'; "
       << "port = '"   << url.port   << "'; "
       << "path = '"   << url.path   << "'; "
       << "query = '"  << url.query  << "'";
    return os;
    }

int main(int argc, char** argv)
try
    {
    typedef rule<const char*> rule_t;
    typedef range<char, char> range_t;
    typedef parse_info<char const*> parse_info_t;

    URL url;

    range_t lowalpha('a', 'z'), upalpha('a', 'z'), digit('0', '9');

    rule_t alpha         = lowalpha | upalpha;
    rule_t alphanum      = alpha | digit;
    rule_t hex           = range_t('a', 'f') | range_t('A', 'F') | digit;
    rule_t escaped       = '%' >> hex >> hex;
    rule_t mark          = ch_p('-') | '_' | '.' | '!' | '~' | '*' | '\'' | '(' | ')';
    rule_t unreserved    = alphanum | mark;
    rule_t pchar         = unreserved | escaped | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    rule_t param         = *pchar;
    rule_t segment       = *pchar >> *( ';' >> param );
    rule_t path_segments = segment >> *( '/' >> segment );
    rule_t reserved      = ch_p(';') | '/' | '?' | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    rule_t uric          = reserved | unreserved | escaped;
    rule_t query         = (*uric)[ref(url.query)];
    rule_t fragment      = *uric;
    rule_t port          = *digit;
    rule_t IPv4address   = +digit >> '.' >> +digit >> '.' >> +digit >> '.' >> +digit;
    rule_t toplabel      = alpha >> *( !ch_p('-') >> alphanum );
    rule_t domainlabel   = alphanum >> *( !ch_p('-') >> alphanum );
    rule_t hostname      = *( domainlabel >> '.' ) >> toplabel >> !ch_p('.');
    rule_t host          = hostname | IPv4address;
    rule_t hostport      = host[ref(url.host)] >> !( ':' >> port[ref(url.port)] );
    rule_t rel_segment   = +( unreserved | escaped | ';' | '@' | '&' | '=' | '+' | '$' | ',' );
    rule_t scheme        = alpha >> *( alpha | digit | '+' | '-' | '.' );
    rule_t uric_no_slash = unreserved | escaped | ';' | '?' | ':' | '@' | '&' | '=' | '+' | '$' | ',';
    rule_t opaque_part   = uric_no_slash >> *uric;
    rule_t userinfo      = *( unreserved | escaped | ';' | ':' | '&' | '=' | '+' | '$' | ',' );
    rule_t server        = !( !( userinfo >> '@' ) >> hostport );
    rule_t reg_name      = +( unreserved | escaped | '$' | ',' | ';' | ':' | '@' | '&' | '=' | '+' );
    rule_t authority     = server | reg_name;
    rule_t abs_path      = ( '/' >> path_segments )[ref(url.path)];
    rule_t net_path      = str_p("//") >> authority >> !abs_path;
    rule_t rel_path      = rel_segment >> !abs_path;
    rule_t hier_part     = ( net_path | abs_path ) >> !( '?' >> query );
    rule_t relativeURI   = ( net_path | abs_path | rel_path ) >> !( '?' >> query );
    rule_t absoluteURI   = scheme[ref(url.scheme)] >> ':' >> ( hier_part | opaque_part );
    rule_t URI_reference = !( absoluteURI | relativeURI ) >> !( '#' >> fragment );

    SPIRIT_DEBUG_RULE(lowalpha);
    SPIRIT_DEBUG_RULE(upalpha);
    SPIRIT_DEBUG_RULE(digit);
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

    if (argc < 2)
        {
        cerr << "Command line syntax error." << endl;
        return 1;
        }
    else
        cout << "Parsing '" << argv[1] << "' ... " << endl;

    parse_info_t result = parse(argv[1], URI_reference);
    if (result.full)
        cout << url << endl;
    else
        cout << "Failure";
    cout << endl;

    return 0;
    }
catch(const exception& e)
    {
    cerr << "Caught exception: " << e.what() << endl;
    return 1;
    }
catch(...)
    {
    cerr << "Caught unknown exception; aborting." << endl;
    return 1;
    }
