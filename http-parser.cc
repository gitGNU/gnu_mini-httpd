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

    // Parse it.

    HTTPRequest request;
    if (parser(buffer.data(), buffer.data() + buffer.size(), request))
        cout << "The request is correct." << endl;
    else
        cout << "Syntax error in request." << endl;

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
