/* -*- mode: c++; eval: (c-set-offset 'statement-cont `c-lineup-math); -*-
 *
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "HTTPParser.hh"
#include <boost/spirit/utility/numerics.hpp>
#include <boost/spirit/attr/semantics.hpp>

using namespace std;
using namespace spirit;

class assign
    {
  public:
    assign(string& dst_) : dst(dst_) { }
    void operator() (const char* begin, const char* end) const
        {
        dst = string(begin, end);
        }
  private:
    string& dst;
    };

HTTPParser::HTTPParser()
        : SP   (32),
          CR   (13),
          LF   (10),
          HT   (9),
          CHAR (0, 127),
          req(0)
    {
    Request         = Request_Line
                      >> *(( general_header | request_header | entity_header ) >> CRLF )
                      >> CRLF
                      >> !message_body;

    Request_Line    = Method[ref(req->method)] >> SP >> Request_URI >> SP >> HTTP_Version >> CRLF;

    Method          = str_p("OPTIONS")
                      | "GET"
                      | "HEAD"
                      | "POST"
                      | "PUT"
                      | "DELETE"
                      | "TRACE"
                      | "CONNECT"
                      | extension_method;

    extension_method = token;

    HTTP_Version    = str_p("HTTP") >> '/' >> uint_p[ref(req->major_version)] >>
                      '.' >> uint_p[ref(req->minor_version)];

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

    qdtext          = anychar_p - '"';

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

    TEXT            = anychar_p - CTL;

    CRLF            = CR >> LF;

    LWS             = !CRLF >> +( SP | HT );

    CTL             = range<>(0, 31) | chlit<>(127);

    separators      = chset<>("()<>@,;:\\\"/[]?={}\x20\x09");

    reserved        = chset<>(";/?:@&=+$,");

    mark            = chset<>("-_.!~*'()");

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
    field_content   = +TEXT | ( token | separators | quoted_string );

    message_body    = entity_body;
    entity_body     = *anychar_p;
    }
