/*
 * Copyright (c) 2001 by Peter Simons <simons@cryp.to>.
 * All rights reserved.
 */

#include <stdexcept>
#include <csignal>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "tcp-listener.hh"
#include "RequestHandler.hh"
#include "log.hh"
#include "config.hh"
using namespace std;

const configuration* config;
volatile sig_atomic_t got_terminate_sig = false;

static void set_sig_term(int)
    {
    got_terminate_sig = true;
    }

int main(int argc, char** argv)
try
    {
    // Create our configuration and place it in the global pointer.

    configuration real_config(argc, argv);
    config = &real_config;

    // Install signal handler.

    signal(SIGTERM, (sighandler_t)&set_sig_term);
    signal(SIGINT, (sighandler_t)&set_sig_term);
    signal(SIGHUP, (sighandler_t)&set_sig_term);
    signal(SIGQUIT, (sighandler_t)&set_sig_term);
    signal(SIGPIPE, SIG_IGN);

    // Start-up scheduler and listener.

    bool using_accurate_poll_interval = true;
    scheduler sched;
    TCPListener<RequestHandler> listener(sched, config->http_port);

    // Change root to our sandbox.

    if (!config->chroot_directory.empty())
        {
        if (chdir(config->chroot_directory.c_str()) == -1 || chroot(".") == -1)
            throw system_error(string("Can't change root to '") + config->chroot_directory + "'");
        }

    // Drop super user priviledges.

    if (geteuid() == 0)
        {
        if (!config->setgid_group.empty())
            {
            if (setgid(config->setgid_group) == -1)
                throw system_error("setgid() failed");
            }
        if (!config->setuid_user.empty())
            if (setuid(config->setuid_user) == -1)
                throw system_error("setuid() failed");
        }

#if 0
    // Detach from terminal.

    if (config->detach)
        {
        switch (fork())
            {
            case -1:
                throw system_error("can't fork()");
            case 0:
                setsid();
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
                break;
            default:
                return 0;
            }
        }
#endif

    // Log some helpful information.

    info("%s %s starting up: listen port = %u, user id = %u, group id = %u, chroot = '%s', " \
         "default hostname = '%s'", PACKAGE_NAME, PACKAGE_VERSION,
         config->http_port, getuid(), getgid(), config->chroot_directory.c_str(),
         config->default_hostname.c_str());

    // Run ...

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

    info("httpd shutting down.");
    return 0;
    }
catch(const configuration::no_error&)
    {
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
