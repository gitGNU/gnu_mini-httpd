/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <csignal>
#include "tcp-listener.hh"
#include "request-handler.hh"
#include "log.hh"
#include "config.hh"
using namespace std;

const configuration* config;
volatile sig_atomic_t got_terminate_sig = false;

static void set_sig_term(int)
    {
    got_terminate_sig = true;
    }

int main(int, char** argv)
try
    {
    // Create our configuration and place it in the global pointer.

    configuration real_config;
    config = &real_config;

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
