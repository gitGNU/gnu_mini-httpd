/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include "HTTPParser.hh"
#include <boost/spirit/attr/semantics.hpp>

using namespace std;
using namespace spirit;

/*
  OK, what comes next is pure madness. These classes solve an
  interesting problem with the Spirit parser framework: I want the
  parsers to assign the results directly to the variables given to me
  by the caller. Unfortunately, I don't know these locations when the
  parsers are constructed, hence I have to use pointers to the
  variables for assignment.

  So far so good.

  To make matters a bit complicated, though, I cannot use pointers,
  because the pointers are set when the user calls the parser, but the
  pointers will be dereferenced at _construction_ time -- they
  aren't set yet at construction time.

  So what I do is to use these assigner classes, which will store the
  location of the pointers and dereference it when the assignment
  takes place. Effectively, these classes are proxies.

  Because I use two types of pointers -- ordinary pointers and
  pointers to member variables --, I need to proxy classes: var_assign
  and member_assign. Both classes are instantiated in the parser
  through the overloaded assign() function that returns the
  appropriate class, depending on the parameter list.

  Of course that would have been too easy.

  The variables I assign to are mostly of type resetable_variable<>,
  and because the compiler won't get the data types right (what is not
  its fault, I admit), I need to define several partially specialized
  templates for the various types of resetable_variable<> I use.

  It's probably best not to read this altogether. I only pray that I
  won't have to understand what I did here in a few weeks, because
  there's a bug somewhere. In fact, I'd be surprised if this stuff
  worked at all. At the time I write this comment, I can compile this
  source module, but I never actually used that parser yet. Keep your
  fingers crossed!
*/

// Proxy class that will assign a parser result via a pointer to classT.

template<typename classT>
class var_assign
    {
  public:
    var_assign(classT** i) : instance(i) { }
    void operator() (const classT& val) const
        {
        *instance = val;
        }
  private:
    classT** instance;
    };

// This specialized version for std::string is necessary because the
// parser will give me a pointer to the begin and the end of the
// string, not a std::string directly.

template<>
class var_assign<string>
    {
  public:
    var_assign(string** i) : instance(i) { }
    void operator() (const char*& first, const char*& last) const
        {
        **instance = string(first, last - first);
        }
  private:
    string** instance;
    };

// Proxy class that will assign a parser result via a pointer to a
// member variable and a pointer to the class instance.

template<typename classT, typename memberT>
class member_assign
    {
  public:
    member_assign(classT** i, memberT classT::* m)
            : instance(i), member(m)
        {
        }
    void operator() (const memberT& val) const
        {
        (*instance)->*member = val;
        }
  private:
    classT** instance;
    memberT classT::* member;
    };

// This specialized version can assign any memberT type to
// resetable_variable<memberT>.

template<typename classT, typename memberT>
class member_assign< classT, resetable_variable<memberT> >
    {
  public:
    member_assign(classT** i, resetable_variable<memberT> classT::* m)
            : instance(i), member(m)
        {
        }
    void operator() (const memberT& val) const
        {
        (*instance)->*member = val;
        }
  private:
    classT** instance;
    resetable_variable<memberT> classT::* member;
    };

// One more specialized version for a memberT of std::string.

template<typename classT>
class member_assign<classT, string>
    {
  public:
    member_assign(classT** i, string classT::* m)
            : instance(i), member(m)
        {
        }
    void operator() (const char*& first, const char*& last) const
        {
        (*instance)->*member = string(first, last - first);
        }
  private:
    classT** instance;
    string classT::* member;
    };

// Function expression that will return an apropriate instance of
// var_assign.

template<typename classT>
var_assign<classT> assign(classT** dst)
    {
    return var_assign<classT>(dst);
    }

// Function expression that will return an apropriate instance of
// member_assign.

template<typename classT, typename memberT>
member_assign<classT,memberT> assign(classT** dst, memberT classT::* var)
    {
    return member_assign<classT,memberT>(dst, var);
    }

// Even though this class looks like another proxy class, it isn't.
// This is basically another version of the ref() functor, but this one
// uses another interface in operator(). I need this one because the
// symbol tables use another parameter syntax to return the result.

template <typename classT>
class ref_assigner
    {
  public:
    ref_assigner(classT& r) : ref(r) { }
    void operator()(const char*&, const char*, const classT& val) const
        {
        ref = val;
        }
    classT& ref;
    };

// Function expression that will return an apropriate instance of
// ref_assign.

template <typename classT>
ref_assigner<classT> assign(classT& r)
    {
    return ref_assigner<classT>(r);
    }

// That was easy, wasn't it? Finally ... the almighty parser. I wonder
// whether the code had been much more complicated, had I written it
// by hand entirely instead of using Spirit at all. :-)

HTTPParser::HTTPParser()
        : CHAR (0, 127),
          HT   (9),
          LF   (10),
          CR   (13),
          SP   (32),
          name_ptr(0),
          data_ptr(0),
          url_ptr(0),
          req_ptr(0)
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
    Method        = token;
    uric          = reserved | unreserved | escaped;
    Query         = *uric;
    http_URL      = nocase_d["http://"]
                    >> Host[assign(&url_ptr, &URL::host)]
                    >> !( ':' >> uint_p[assign(&url_ptr, &URL::port)] )
                    >> !( abs_path[assign(&url_ptr, &URL::path)]
                    >> !( '?' >> Query[assign(&url_ptr, &URL::query)] ) );
    Request_URI   = http_URL | abs_path[assign(&url_ptr, &URL::path)]
                    >> !( '?' >> Query[assign(&url_ptr, &URL::query)] );
    HTTP_Version  = nocase_d["http/"] >> uint_p[assign(&req_ptr, &HTTPRequest::major_version)]
                    >> '.' >> uint_p[assign(&req_ptr, &HTTPRequest::minor_version)];
    Request_Line  = Method[assign(&req_ptr, &HTTPRequest::method)] >> SP >> Request_URI >> SP >> HTTP_Version >> CRLF;
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
    Header        = ( field_name[assign(&name_ptr)] >> *LWS >> ":" >> *LWS
                    >> !( field_value[assign(&data_ptr)] ) ) >> CRLF;
    Host_Header   = Host[assign(&req_ptr, &HTTPRequest::host)]
                    >> !( ":" >> uint_p[assign(&req_ptr, &HTTPRequest::port)] );
    weekday       = "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday";
    wkday         = "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun";
    month.add       ("Jan", 0)("Feb", 1)("Mar", 2)("Apr", 3)("May", 4)("Jun", 5)
                    ("Jul", 6)("Aug", 7)("Sep", 8)("Oct", 9)("Nov", 10)("Dec", 11);
    time          = uint_p[ref(tm_date.tm_hour)] >> ":" >> uint_p[ref(tm_date.tm_min)] >>
                    ":" >> uint_p[ref(tm_date.tm_sec)];
    date1         = uint_p[ref(tm_date.tm_mday)] >> SP >> month[assign(tm_date.tm_mon)] >>
                    SP >> uint_p[ref(tm_date.tm_year)];
    date2         = uint_p[ref(tm_date.tm_mday)] >> "-" >> month[assign(tm_date.tm_mon)] >>
                    "-" >> uint_p[ref(tm_date.tm_year)];
    date3         = month[assign(tm_date.tm_mon)] >> SP >>
                    ( uint_p[ref(tm_date.tm_mday)] | ( SP >> uint_p[ref(tm_date.tm_mday)] ) );
    rfc1123_date  = wkday >> "," >> SP >> date1 >> SP >> time >> SP >> "GMT";
    rfc850_date   = weekday >> "," >> SP >> date2 >> SP >> time >> SP >> "GMT";
    asctime_date  = wkday >> SP >> date3 >> SP >> time >> SP >> uint_p[ref(tm_date.tm_year)];
    HTTP_date     = rfc1123_date | rfc850_date | asctime_date;
    If_Modified_Since_Header = HTTP_date;

    // Initialize the global variables telling us our time zone and
    // stuff. We'll need that to turn the GMT dates in the headers to
    // local time.

    tzset();
    }

bool HTTPParser::have_complete_header_line(const string& input)
    {
    std::string::const_iterator p;
    for (p = input.begin(); input.end() - p > 2; ++p)
        {
        if (*p == '\r' && p[1] == '\n' && !(p[2] == '\t' || p[2] == ' '))
            break;
        }
    if (input.end() - p > 1)
        return true;
    else
        return false;
    }

bool HTTPParser::supports_persistent_connection(const HTTPRequest& request)
    {
    if (strcasecmp(request.connection.c_str(), "close") == 0)
        return false;
    if (strcasecmp(request.connection.c_str(), "keep-alive") == 0 && !request.keep_alive.empty())
        return true;
    if (request.major_version >= 1 && request.minor_version >= 1)
        return true;
    else
        return false;
    }

size_t HTTPParser::parse_header(string& name, string& data, const string& input) const
    {
    name_ptr = &name;
    data_ptr = &data;

    parse_info_t info = parse(input.data(), input.data() + input.size(), Header);
    if (info.match)
        return info.length;
    else
        return 0;
    }

size_t HTTPParser::parse_request_line(HTTPRequest& request, const string& input) const
    {
    req_ptr = &request;
    url_ptr = &request.url;

    parse_info_t info = parse(input.data(), input.data() + input.size(), Request_Line);
    if (info.match)
        return info.length;
    else
        return 0;
    }

size_t HTTPParser::parse_host_header(HTTPRequest& request, const std::string& input) const
    {
    req_ptr = &request;

    parse_info_t info = parse(input.data(), input.data() + input.size(), Host_Header);
    if (info.match)
        return info.length;
    else
        return 0;
    }

size_t HTTPParser::parse_if_modified_since_header(HTTPRequest& request, const std::string& input) const
    {
    memset(&tm_date, 0, sizeof(tm_date));

    parse_info_t info = parse(input.data(), input.data() + input.size(), If_Modified_Since_Header);
    if (!info.match)
        return 0;

    // Make sure the tm structure contains no nonsense.

    if (tm_date.tm_year < 1970 || tm_date.tm_hour > 23 || tm_date.tm_min > 59 || tm_date.tm_sec > 59)
        return 0;

    switch(tm_date.tm_mon)
        {
        case 0: case 2: case 4:
        case 6: case 7: case 9:
        case 11:
            if (tm_date.tm_mday > 31)
                return 0;
            break;
        case 1:
            if (tm_date.tm_year % 4 == 0 && tm_date.tm_year % 100 != 0)
                {
                if (tm_date.tm_mday > 29)
                    return 0;
                }
            else
                {
                if (tm_date.tm_mday > 28)
                    return 0;
                }
            break;
        case 3: case 5: case 8:
        case 10:
            if (tm_date.tm_mday > 30)
                return 0;
            break;
        default:
            throw logic_error("The month in HTTPParser::tm_date is screwed badly.");
        }

    // The date is fine. Now turn it into a time_t.

    tm_date.tm_year -= 1900;
    request.if_modified_since = mktime(&tm_date) - timezone;

    // Done.

    return info.length;
    }

// And here comes the global parser instance.

const HTTPParser http_parser;
