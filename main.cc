/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <csignal>
#include <ctime>
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

    // Set global variables with our timezone information.

    tzset();

    // Start-up scheduler and listener, drop root priviledges, and
    // start running.

    bool using_accurate_poll_interval = true;
    scheduler sched;
    TCPListener<RequestHandler> listener(sched, config->http_port);
    setgid(config->setgid_group);
    setuid(config->setuid_user);
    info("httpd starting up: listen port = %u, user id = %u, group id %u",
         config->http_port, getuid(), getgid());
    while(!got_terminate_sig && !sched.empty())
        {
	sched.schedule();

        if (RequestHandler::instances > config->hard_poll_interval_threshold)
            {
            if (using_accurate_poll_interval)
                {
                debug(("We have %d active connections: Switching to a hard poll interval of %d seconds.",
                       RequestHandler::instances, config->hard_poll_interval));
                sched.set_poll_interval(config->hard_poll_interval * 1000);
                using_accurate_poll_interval = false;
                }
            }
        else
            {
            if (!using_accurate_poll_interval)
                {
                debug(("We have %d active connections: Switching back to an accurate poll interval.",
                       RequestHandler::instances));
                sched.use_accurate_poll_interval();
                using_accurate_poll_interval = true;
                }
            }
        }

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
