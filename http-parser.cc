/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <string>
#include <stdexcept>

#include <iostream>
#include <unistd.h>
#include "system-error/system-error.hh"
#include "HTTPParser.hh"

using namespace std;

int main(int argc, char** argv)
try
    {
    // Create an instance of the HTTPParser.

    HTTPParser parser;

    // Read the request into memory.

#if 0
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
#else
    string buffer =
        "GET /?foo=bar HTTP/2341.41\r\n" \
        "Host: localhost:8080\r\n" \
        "User-Agent: Mozilla/5.0\r\n" \
        "        (X11; U; Linux i686; en-US; rv:0.9.9+) Gecko/20020305\r\n" \
        "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,video/x-mng," \
        "image/png,image/jpeg,image/gif;q=0.2,text/css,*/*;q=0.1\r\n" \
        "Accept-Language: en\r\n" \
        "Accept-Encoding: gzip, deflate, compress;q=0.9\r\n" \
        "Accept-Charset: ISO-8859-1, utf-8;q=0.66, *;q=0.66\r\n" \
        "Keep-Alive: 300\r\n" \
        "Connection: keep-alive\r\n" \
        "\r\n";
#endif

    // Parse it.

    if (!parser(buffer.data(), buffer.data() + buffer.size()))
        {
        cout << "Syntax error in request." << endl;
        return 1;
        }

    cout << "method.......: " << parser.HTTPRequest::method << endl
         << "major version: " << parser.HTTPRequest::major_version << endl
         << "minor version: " << parser.HTTPRequest::minor_version << endl
         << "host.........: " << parser.HTTPRequest::host << endl
         << "port.........: " << parser.HTTPRequest::port << endl
         << "path.........: " << parser.HTTPRequest::path << endl
         << "query........: " << parser.HTTPRequest::query << endl
        ;

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
