/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <string>
#include <csignal>

using namespace std;
#include "tcp-listener.hh"
#include "httpd.hh"
#include "log.hh"
#include "config.hh"

const configuration* config;
bool got_terminate_sig = false;

void set_sig_term(int)
    {
    info("Received signal; shutting down.");
    got_terminate_sig = true;
    }

int main(int, char** argv)
try
    {
    // Create our configuration.

    config = new configuration;

    // Install signal handler.

    signal(SIGTERM, &set_sig_term);
    signal(SIGINT, &set_sig_term);
    signal(SIGHUP, &set_sig_term);
    signal(SIGQUIT, &set_sig_term);
    signal(SIGPIPE, SIG_IGN);

    // Start-up scheduler and listener.

    scheduler sched;
    TCPListener<RequestHandler> listener(sched, 8080);
    while(!got_terminate_sig && !sched.empty())
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
