/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "HTTPParser.hh"
#include <boost/spirit/attr/semantics.hpp>

using namespace std;
using namespace spirit;

HTTPParser http_parser;

HTTPParser::HTTPParser()
        : CHAR (0, 127),
          HT   (9),
          LF   (10),
          CR   (13),
          SP   (32)
    {
    CRLF          = CR >> LF;
    mark          = chset_t("-_.!~*'()");
    reserved      = chset_t(";/?:@&=+$,");
    unreserved    = alnum_p | mark;
    escaped       = '%' >> xdigit_p >> xdigit_p;
    pchar         = unreserved | escaped | chset_t(":@&=+$,");
    param         = *pchar;
    segment       = *pchar >> *( ';' >> param );
    segments      = segment >> *( '/' >> segment );
    abs_path      = ( '/' >> segments );
    domainlabel   = alnum_p >> *( !ch_p('-') >> alnum_p );
    toplabel      = alpha_p >> *( !ch_p('-') >> alnum_p );
    hostname      = *( domainlabel >> '.' ) >> toplabel >> !ch_p('.');
    IPv4address   = (+digit_p >> '.' ).repeat(3) >> +digit_p;
    Host          = hostname | IPv4address;
    Method        = str_p("GET") | "HEAD";
    uric          = reserved | unreserved | escaped;
    Query         = *uric;
    http_URL      = nocase_d["http://"] >> Host[ref(res_host)] >> !( ':' >> uint_p[ref(res_port)] )
                    >> !( abs_path[ref(res_path)] >> !( '?' >> Query[ref(res_query)] ) );
    Request_URI   = http_URL | abs_path[ref(res_path)] >> !( '?' >> Query[ref(res_query)] );
    HTTP_Version  = nocase_d["http/"] >> uint_p[ref(res_major_version)]
                    >> '.' >> uint_p[ref(res_minor_version)];
    Request_Line  = Method[ref(res_method)] >> SP >> Request_URI >> SP >> HTTP_Version >> CRLF;
    CTL           = range_t(0, 31) | chlit_t(127);
    TEXT          = anychar_p - CTL;
    separators    = chset_t("()<>@,;:\\\"/[]?={}\x20\x09");
    token         = +( CHAR - ( CTL | separators ) );
    LWS           = !CRLF >> +( SP | HT );
    quoted_pair   = '\\' >> CHAR;
    qdtext        = anychar_p - '"';
    quoted_string = ( '"' >> *(qdtext | quoted_pair ) >> '"' );
    field_content = +TEXT | ( token | separators | quoted_string );
    field_value   = *( field_content | LWS );
    field_name    = token;
    Header        = ( field_name[ref(res_name)] >> *LWS >> ":" >> *LWS >> !( field_value[ref(res_data)] ) ) >> CRLF;
    Host_Header   = Host[ref(res_host)] >> !( ":" >> uint_p[ref(res_port)] );
    weekday       = "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday";
    wkday         = "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun";
    month.add       ("Jan", 0)("Feb", 1)("Mar", 2)("Apr", 3)("May", 4)("Jun", 5)
                    ("Jul", 6)("Aug", 7)("Sep", 8)("Oct", 9)("Nov", 10)("Dec", 11);
    time          = uint_p[ref(res_date.tm_hour)] >> ":" >> uint_p[ref(res_date.tm_min)] >>
                    ":" >> uint_p[ref(res_date.tm_sec)];
    date1         = uint_p[ref(res_date.tm_mday)] >> SP >> month[st_data(res_date.tm_mon)] >>
                    SP >> uint_p[ref(res_date.tm_year)];
    date2         = uint_p[ref(res_date.tm_mday)] >> "-" >> month[st_data(res_date.tm_mon)] >>
                    "-" >> uint_p[ref(res_date.tm_year)];
    date3         = month[st_data(res_date.tm_mon)] >> SP >>
                    ( uint_p[ref(res_date.tm_mday)] | ( SP >> uint_p[ref(res_date.tm_mday)] ) );
    rfc1123_date  = wkday >> "," >> SP >> date1 >> SP >> time >> SP >> "GMT";
    rfc850_date   = weekday >> "," >> SP >> date2 >> SP >> time >> SP >> "GMT";
    asctime_date  = wkday >> SP >> date3 >> SP >> time >> SP >> uint_p[ref(res_date.tm_year)];
    HTTP_date     = rfc1123_date | rfc850_date | asctime_date;
    If_Modified_Since_Header = HTTP_date;

#if 0
    value           = token | quoted_string;
    attribute       = token;
    parameter       = attribute >> *LWS >> "=" >> *LWS >> value;
    transfer_extension = token >> *( *LWS >> ";" >> *LWS >> parameter );
    transfer_coding    = "chunked" | transfer_extension;
    Transfer_Encoding  = str_p("Transfer_Encoding") >> *LWS >> ":" >>
                         ( *LWS >> transfer_coding >> *( *LWS >> "," >> *LWS >> transfer_coding ) );
#endif
    }
