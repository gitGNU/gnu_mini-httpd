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
          SP   (32),
          commit_month(*this)
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
#if 0
symbols<> monthlist;
monthlist.add
    ("Jan",  0)
    ("Feb",  1)
    ("Mar",  2)
    ("Apr",  3)
    ("May",  4)
    ("Jun",  5)
    ("Jul",  6)
    ("Aug",  7)
    ("Sep",  8)
    ("Oct",  9)
    ("Nov", 10)
    ("Dec", 11);

month = monthlist[ref(date.tm_mon)];

    weekday         = str_p("Monday") | "Tuesday" | "Wednesday" | "Thursday" | "Friday" | "Saturday" | "Sunday";
    month           = str_p("Jan") | "Feb" | "Mar" | "Apr" | "May" | "Jun"
                    | "Jul" | "Aug" | "Sep" | "Oct" | "Nov" | "Dec";
#endif

    weekday         = "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday";
    wkday           = str_p("Mon") | "Tue" | "Wed" | "Thu" | "Fri" | "Sat" | "Sun";

    month           = "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec";

    time            = digit_p.repeat(2) >> ":" >> digit_p.repeat(2) >> ":" >> digit_p.repeat(2);
    date1           = digit_p.repeat(2) >> SP >> month >> SP >> digit_p.repeat(4);
    date2           = digit_p.repeat(2) >> "-" >> month >> "-" >> digit_p.repeat(2);
    date3           = month >> SP >> ( digit_p.repeat(2) | ( SP >> digit_p ) );
    rfc1123_date    = wkday >> "," >> SP >> date1 >> SP >> time >> SP >> "GMT";
    rfc850_date     = weekday >> "," >> SP >> date2 >> SP >> time >> SP >> "GMT";
    asctime_date    = wkday >> SP >> date3 >> SP >> time >> SP >> digit_p.repeat(4);
    HTTP_date       = rfc1123_date | rfc850_date | asctime_date;
    If_Modified_Since_Header = HTTP_date;

#if 0
    CRLF            = CR >> LF;

    LWS             = !CRLF >> +( SP | HT );

    mark            = chset_type("-_.!~*'()");

    reserved        = chset_type(";/?:@&=+$,");

    unreserved      = alnum_p | mark;

    escaped         = '%' >> xdigit_p >> xdigit_p;

    pchar           = unreserved | escaped | chset_type(":@&=+$,");

    param           = *pchar;

    segment         = *pchar >> *( ';' >> param );

    path_segments   = segment >> *( '/' >> segment );

    abs_path        = '/' >> path_segments;

    domainlabel     = alnum_p >> *( !ch_p('-') >> alnum_p );

    toplabel        = alpha_p >> *( !ch_p('-') >> alnum_p );

    hostname        = *( domainlabel >> '.' ) >> toplabel >> !ch_p('.');

    IPv4address     = (+digit_p >> '.' ).repeat(3) >> +digit_p;

    host            = hostname | IPv4address;

    uric            = reserved | unreserved | escaped;

    query           = *uric;

    Request_URI    = !( str_p("http://") >> host[ref(HTTPRequest::host)] >> !( ":" >> uint_p[ref(HTTPRequest::port)] ) ) >>
                     !( abs_path[ref(HTTPRequest::path)] >> !( "?" >> query[ref(HTTPRequest::query)] ) );

    HTTP_Version    = str_p("HTTP") >> '/' >>
                      uint_p[ref(HTTPRequest::major_version)] >> '.' >>
                      uint_p [ref(HTTPRequest::minor_version)];

    Method          = str_p("GET")
                      | "HEAD"
                      | "POST";

    Request_Line    = Method[ref(HTTPRequest::method)] >> SP >> Request_URI >> SP >> HTTP_Version >> CRLF;

    CTL             = range<>(0, 31) | chlit<>(127);

    separators      = chset_type("()<>@,;:\\\"/[]?={}\x20\x09");

    token           = +( CHAR - ( CTL | separators ) );

    connection_token = token;

    Connection      = str_p("Connection") >> *LWS >> ":" >>
                      ( *LWS >> connection_token >> *( *LWS >> "," >> *LWS >> connection_token ) );




    Date            = str_p("Date") >> *LWS >> ":" >> *LWS >> HTTP_date;

    quoted_pair     = '\\' >> CHAR;

    qdtext          = anychar_p - '"';

    quoted_string   = ( '"' >> *(qdtext | quoted_pair ) >> '"' );

    value           = token | quoted_string;

    attribute       = token;

    parameter       = attribute >> *LWS >> "=" >> *LWS >> value;

    transfer_extension = token >> *( *LWS >> ";" >> *LWS >> parameter );

    transfer_coding    = "chunked" | transfer_extension;

    Transfer_Encoding  = str_p("Transfer_Encoding") >> *LWS >> ":" >>
                         ( *LWS >> transfer_coding >> *( *LWS >> "," >> *LWS >> transfer_coding ) );

    general_header  = Connection | Date | Transfer_Encoding;


    request_header  = Host;

    TEXT            = anychar_p - CTL;

    field_content   = +TEXT | ( token | separators | quoted_string );

    field_value     = *( field_content | LWS );

    field_name      = token;

    message_header  = ( field_name[ref(name)] >> *LWS >> ":" >> *LWS >> !( field_value[ref(data)] ) )
                      [&HTTPParser::msghead_committer];

    entity_body     = *anychar_p;

    message_body    = entity_body;

    extension_header = message_header;

    entity_header   = extension_header;

    Request         = Request_Line
                      >> *(( general_header | request_header | entity_header ) >> CRLF )
                      >> CRLF
                      >> !message_body;
#endif
    }


void HTTPParser::commit_month_t::operator() (iterator_t begin, iterator_t end) const
    {
    }
