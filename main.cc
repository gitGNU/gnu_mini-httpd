/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <string>

using namespace std;
#include "tcp-listener.hh"
#include "httpd.hh"
#include "log.hh"

int main(int, char** argv)
try
    {
    // Start-up scheduler and listener.

    scheduler sched;
    TCPListener<RequestHandler> listener(sched, 8080);
    while(!sched.empty())
	sched.schedule();

    // Exit gracefully.

    return 0;
    }
catch(const exception& e)
    {
    error("Caught exception: %s", e.what());
    return 1;
    }
catch(...)
    {
    error("Caught unknown exception.");
    return 1;
    }
