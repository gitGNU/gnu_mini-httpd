/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>

#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/attr/semantics.hpp>

using namespace std;
using namespace spirit;

class URI
    {
  public:
    URI()                { }
    URI(const char* buf) { init(buf); }
    ~URI()               { }

  private:
    string scheme_s;
    string host_s;
    string ipaddress_s;
    string port_s;
    string path_s;
    string query_s;

    typedef rule<const char*> rule_t;
    typedef range<char, char> range_t;
    typedef parse_info<char const*> parse_info_t;

    void init(const char* buf)
        {
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
        rule_t query         = (*uric)[ref(query_s)];
        rule_t fragment      = *uric;
        rule_t port          = *digit;
        rule_t IPv4address   = +digit >> '.' >> +digit >> '.' >> +digit >> '.' >> +digit;
        rule_t toplabel      = alpha >> *( !ch_p('-') >> alphanum );
        rule_t domainlabel   = alphanum >> *( !ch_p('-') >> alphanum );
        rule_t hostname      = domainlabel >> '.' >> hostname |  toplabel >> !ch_p('.');
        rule_t host          = hostname[ref(host_s)] | IPv4address[ref(ipaddress_s)];
        rule_t hostport      = host >> !( ':' >> port[ref(port_s)] );
        rule_t rel_segment   = +( unreserved | escaped | ';' | '@' | '&' | '=' | '+' | '$' | ',' );
        rule_t scheme        = alpha >> *( alpha | digit | '+' | '-' | '.' );
        rule_t uric_no_slash = unreserved | escaped | ';' | '?' | ':' | '@' | '&' | '=' | '+' | '$' | ',';
        rule_t opaque_part   = uric_no_slash >> *uric;
        rule_t userinfo      = *( unreserved | escaped | ';' | ':' | '&' | '=' | '+' | '$' | ',' );
        rule_t server        = !( !( userinfo >> '@' ) >> hostport );
        rule_t reg_name      = +( unreserved | escaped | '$' | ',' | ';' | ':' | '@' | '&' | '=' | '+' );
        rule_t authority     = server | reg_name;
        rule_t abs_path      = ( '/' >> path_segments )[ref(path_s)];
        rule_t net_path      = str_p("//") >> authority >> !abs_path;
        rule_t rel_path      = rel_segment >> !abs_path;
        rule_t hier_part     = ( net_path | abs_path ) >> !( '?' >> query );
        rule_t relativeURI   = ( net_path | abs_path | rel_path ) >> !( '?' >> query );
        rule_t absoluteURI   = scheme[ref(scheme_s)] >> ':' >> ( hier_part | opaque_part );
        rule_t URI_reference = !( absoluteURI | relativeURI ) >> !( '#' >> fragment );

        parse_info_t result = parse(buf, URI_reference);
        if (!result.full)
            throw std::runtime_error(string("Input is not a valid URI: ") + buf);
        }

    };

#if 0
inline std::ostream& operator<< (std::ostream& os, const URL& url)
    {
    os << "scheme = '"    << url.scheme    << "'; "
       << "host = '"      << url.host      << "'; "
       << "ipaddress = '" << url.ipaddress << "'; "
       << "port = '"      << url.port      << "'; "
       << "path = '"      << url.path      << "'; "
       << "query = '"     << url.query     << "'";
    return os;
    }
#endif

int main(int argc, char** argv)
try
    {


    if (argc < 2)
        {
        cerr << "Command line syntax error." << endl;
        return 1;
        }
    else
        cout << "Parsing '" << argv[1] << "' ... " << endl;

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
