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
#include "config.hh"

const configuration* config;

int main(int, char** argv)
try
    {
    // Create our configuration.

    config = new configuration;

    // Start-up scheduler and listener.

    scheduler sched;
    TCPListener<RequestHandler> listener(sched, 8080);
    while(!sched.empty())
	sched.schedule();

    // Exit gracefully.

    delete config;
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
