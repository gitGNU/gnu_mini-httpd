/* -*- mode: c++; eval: (c-set-offset 'statement-cont `c-lineup-math); -*-
 *
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <string>
#include <stdexcept>
#include <boost/spirit/spirit_rule.hpp>
#include <boost/spirit/utility/chset.hpp>

using namespace std;
using namespace spirit;

class HTTPRequest
    {
  public:
    struct parse_error : public std::runtime_error
        {
        parse_error() : runtime_error("Input is not a valid HTTP request.") { }
        };

    typedef const char*             iterator_t;
    typedef rule<iterator_t>        rule_t;
    typedef parse_info<iterator_t>  parse_info_t;

    HTTPRequest(iterator_t begin, iterator_t end)
            : SP         (32),
              CR         (13),
              LF         (10),
              HT         (9),
              CHAR       (0, 127),
              CTL        ("\x00-\x1F\x7F"),
              separators ("()<>@,;:\\\"/[]?={}\x20\x09"),
              reserved   (";/?:@&=+$,"),
              mark       ("-_.!~*'()")
        {
        Request         = Request_Line
                          >> *(( general_header | request_header | entity_header ) >> CRLF )
                          >> CRLF
                          >> !message_body;

        Request_Line    = Method >> SP >> Request_URI >> SP >> HTTP_Version >> CRLF;

        Method          = str_p("OPTIONS") | "GET" | "HEAD" | "POST"
                          | "PUT" | "DELETE" | "TRACE" | "CONNECT"
                          | extension_method;

        extension_method = token;

        HTTP_Version    = str_p("HTTP") >> '/' >> +digit_p >> '.' >> +digit_p;

        Request_URI     = '*' | absoluteURI | abs_path | authority;

        absoluteURI     = scheme >> ':' >> ( hier_part | opaque_part );

        hier_part       = ( net_path | abs_path ) >> !( '?' >> query );

        net_path        = "//" >> authority >> !abs_path;

        abs_path        = '/' >> path_segments;

        path_segments   = segment >> *( '/' >> segment );

        segment         = *pchar >> *( ';' >> param );

        param           = *pchar;

        pchar           = unreserved | escaped | chset<>(":@&=+$,");

        unreserved      = alnum_p | mark;

        escaped         = '%' >> xdigit_p >> xdigit_p;

        server          = !( !( userinfo >> '@' ) >> hostport );

        userinfo        = *( unreserved | escaped | chset<>(";:&=+$,") );

        hostport        = host >> !( ':' >> port );

        host            = hostname | IPv4address;

        hostname        = *( domainlabel >> '.' ) >> toplabel >> !ch_p('.');

        domainlabel     = alnum_p >> *( !ch_p('-') >> alnum_p );

        toplabel        = alpha_p >> *( !ch_p('-') >> alnum_p );

        IPv4address     = (+digit_p >> '.' ).repeat(3) >> +digit_p;

        port            = *digit_p;

        scheme          = alpha_p >> *( alpha_p | digit_p | chset<>("+-.") );

        authority       = server | reg_name;

        reg_name        = +( unreserved | escaped | chset<>("'$,;:@&=+") );

        opaque_part     = uric_no_slash >> *uric;

        query           = *uric;

        uric_no_slash   = unreserved | escaped | chset<>(";?:@&=+$");

        uric            = reserved | unreserved | escaped;

        token           = +( CHAR - ( CTL | separators ) );

        quoted_string   = ( '"' >> *(qdtext | quoted_pair ) >> '"' );

        qdtext          = TEXT - '"';

        TEXT            = anychar_p - CTL;

        quoted_pair     = '\\' >> CHAR;

        general_header  = Connection | Date | Pragma | Trailer | Transfer_Encoding | Upgrade ;

        Connection      = str_p("Connection") >> *LWS >> ":" >>
                          ( *LWS >> connection_token >> *( *LWS >> "," >> *LWS >> connection_token ) );

        connection_token = token;

        Pragma           = str_p("Pragma") >> *LWS >> ":" >>
                           ( *LWS >> pragma_directive >> *( *LWS >> "," >> *LWS >> pragma_directive ) );

        pragma_directive = "no-cache" | extension_pragma;

        extension_pragma = token >> !( *LWS >> "=" >> *LWS >> ( token | quoted_string ) );

        Trailer         = str_p("Trailer") >> *LWS >> ":" >>
                          ( *LWS >> field_name >> *( *LWS >> "," >> *LWS >> field_name ) );

        field_name      = token;

        Transfer_Encoding  = str_p("Transfer_Encoding") >> *LWS >> ":" >>
                             ( *LWS >> transfer_coding >> *( *LWS >> "," >> *LWS >> transfer_coding ) );

        transfer_coding    = "chunked" | transfer_extension;

        transfer_extension = token >> *( *LWS >> ";" >> *LWS >> parameter );

        parameter       = attribute >> *LWS >> "=" >> *LWS >> value;

        attribute       = token;

        value           = token | quoted_string;

        Date            = str_p("Date") >> *LWS >> ":" >> *LWS >> HTTP_date;

        HTTP_date       = rfc1123_date | rfc850_date | asctime_date;

        rfc1123_date    = wkday >> "," >> SP >> date1 >> SP >> time >> SP >> "GMT";

        rfc850_date     = weekday >> "," >> SP >> date2 >> SP >> time >> SP >> "GMT";

        asctime_date    = wkday >> SP >> date3 >> SP >> time >> SP >> digit_p.repeat(4);

        date1           = digit_p.repeat(2) >> SP >> month >> SP >> digit_p.repeat(4);

        date2           = digit_p.repeat(2) >> "-" >> month >> "-" >> digit_p.repeat(2);

        date3           = month >> SP >> ( digit_p.repeat(2) | ( SP >> digit_p ));

        time            = digit_p.repeat(2) >> ":" >> digit_p.repeat(2) >> ":" >> digit_p.repeat(2);

        wkday           = str_p("Mon") | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun";

        weekday         = str_p("Monday") | "Tuesday" | "Wednesday"
                          | "Thursday" | "Friday" | "Saturday" | "Sunday";

        month           = str_p("Jan") | "Feb" | "Mar" | "Apr" | "May" | "Jun"
                          | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec";

        Upgrade         = "Upgrade" >> *LWS >> ":" >> ( *LWS >> product >> *( *LWS >> "," >> *LWS >> product ) );

        product         = token >> *LWS >> !( "/" >> *LWS >> product_version );

        product_version = token;

        CRLF            = CR >> LF;
        LWS             = !CRLF >> +( SP | HT );

        request_header  = Host; /* | Accept
                                   | Accept_Charset
                                   | Accept_Encoding
                                   | Accept_Language
                                   | Authorization
                                   | Expect
                                   | From
                                   | If_Match
                                   | If_Modified_Since
                                   | If_None_Match
                                   | If_Range
                                   | If_Unmodified_Since
                                   | Max_Forwards
                                   | Proxy_Authorization
                                   | Range
                                   | Referer
                                   | TE
                                   | User_Agent
                                */

        Host            = str_p("Host") >> *LWS >> ":" >> *LWS >> host >> !( ":" >> port );

        entity_header   = extension_header; /* | Allow
                                               | Content_Encoding
                                               | Content_Language
                                               | Content_Length
                                               | Content_Location
                                               | Content_MD5
                                               | Content_Range
                                               | Content_Type
                                               | Expires
                                               | Last_Modified
                                            */
        extension_header = message_header;

        message_header  = field_name >> *LWS >> ":" >> *LWS >> !field_value;
        field_name      = token;
        field_value     = *( field_content | LWS );
        field_content   = *TEXT | *( token | separators | quoted_string );

        message_body    = entity_body;
        entity_body     = *anychar_p;

        SPIRIT_DEBUG_RULE(Request);
        SPIRIT_DEBUG_RULE(Request_Line);
        SPIRIT_DEBUG_RULE(Method);
        SPIRIT_DEBUG_RULE(CRLF);
        SPIRIT_DEBUG_RULE(Request_URI);
        SPIRIT_DEBUG_RULE(HTTP_Version);
        SPIRIT_DEBUG_RULE(query);
        SPIRIT_DEBUG_RULE(general_header);
        SPIRIT_DEBUG_RULE(quoted_pair);
        SPIRIT_DEBUG_RULE(token);
        SPIRIT_DEBUG_RULE(separators);
        SPIRIT_DEBUG_RULE(quoted_string);
        SPIRIT_DEBUG_RULE(qdtext);
        SPIRIT_DEBUG_RULE(TEXT);
        SPIRIT_DEBUG_RULE(request_header);
        SPIRIT_DEBUG_RULE(entity_header);
        SPIRIT_DEBUG_RULE(message_body);
        SPIRIT_DEBUG_RULE(entity_body);
        SPIRIT_DEBUG_RULE(Connection);
        SPIRIT_DEBUG_RULE(connection_token);
        SPIRIT_DEBUG_RULE(Date);
        SPIRIT_DEBUG_RULE(HTTP_date);
        SPIRIT_DEBUG_RULE(Pragma);
        SPIRIT_DEBUG_RULE(pragma_directive);
        SPIRIT_DEBUG_RULE(extension_pragma);
        SPIRIT_DEBUG_RULE(Trailer);
        SPIRIT_DEBUG_RULE(Transfer_Encoding);
        SPIRIT_DEBUG_RULE(Upgrade);
        SPIRIT_DEBUG_RULE(field_name);
        SPIRIT_DEBUG_RULE(transfer_coding);
        SPIRIT_DEBUG_RULE(transfer_extension);
        SPIRIT_DEBUG_RULE(parameter);
        SPIRIT_DEBUG_RULE(attribute);
        SPIRIT_DEBUG_RULE(value);
        SPIRIT_DEBUG_RULE(rfc1123_date);
        SPIRIT_DEBUG_RULE(rfc850_date);
        SPIRIT_DEBUG_RULE(asctime_date);
        SPIRIT_DEBUG_RULE(wkday);
        SPIRIT_DEBUG_RULE(date1);
        SPIRIT_DEBUG_RULE(date2);
        SPIRIT_DEBUG_RULE(date3);
        SPIRIT_DEBUG_RULE(month);
        SPIRIT_DEBUG_RULE(time);
        SPIRIT_DEBUG_RULE(weekday);
        SPIRIT_DEBUG_RULE(product);
        SPIRIT_DEBUG_RULE(product_version);
        SPIRIT_DEBUG_RULE(Host);
        SPIRIT_DEBUG_RULE(extension_header);
        SPIRIT_DEBUG_RULE(message_header);
        SPIRIT_DEBUG_RULE(field_value);
        SPIRIT_DEBUG_RULE(field_content);

        parse_info_t result = parse(begin, end, Request);
        if (!result.full)
            throw parse_error();
        }

    string method;

  private:
    chlit<> SP, CR, LF, HT;
    range<> CHAR;
    chset<> CTL, separators, reserved, mark;
    rule_t Request, Request_Line, Method, CRLF, Request_URI, HTTP_Version,
        absoluteURI, hier_part, net_path, abs_path, scheme, authority,
        opaque_part, query, path_segments, segment, param, pchar,
        escaped, server, userinfo, hostport, reg_name, unreserved,
        host, hostname, domainlabel, toplabel, IPv4address, port,
        uric_no_slash, uric, general_header, quoted_pair,
        token, quoted_string, qdtext, TEXT, request_header,
        entity_header, message_body, entity_body, Connection, LWS,
        connection_token, Date, HTTP_date, Pragma, pragma_directive,
        extension_pragma, Trailer, Transfer_Encoding, Upgrade,
        field_name, transfer_coding, transfer_extension, parameter,
        attribute, value, rfc1123_date, rfc850_date, asctime_date,
        wkday, date1, date2, date3, month, time, weekday, product,
        product_version, Host, extension_header, message_header,
        field_value, field_content, extension_method;
    };



#include <iostream>
#include <unistd.h>
#include "system-error/system-error.hh"

int main(int argc, char** argv)
try
    {
    string buffer;
    char partbuf[1*1024];
    ssize_t rc;
    for (rc = read(STDIN_FILENO, partbuf, sizeof(partbuf));
	 rc > 0;
	 rc = read(STDIN_FILENO, partbuf, sizeof(partbuf)))
	{
	buffer.append(partbuf, rc);
	}
    if (rc < 0)
	throw system_error("Failed to read mail from standard input");

    HTTPRequest request(buffer.data(), buffer.data() + buffer.size());

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
