/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
#include "tcp-listener.hh"
#include "httpd.hh"

int main(int, char** argv)
try
    {
    scheduler sched;
    TCPListener<RequestHandler> listener(sched, 8080);
    while(!sched.empty())
	sched.schedule();
    return 0;
    }
catch(const exception& e)
    {
    cerr << "Caught exception: " << e.what() << endl;
    return 1;
    }
catch(...)
    {
    cerr << "Caught unknown exception." << endl;
    return 1;
    }
