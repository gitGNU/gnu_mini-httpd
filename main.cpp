/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#include "http-daemon.hpp"

#include <boost/test/prg_exec_monitor.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdexcept>
#include <csignal>
#include <ctime>                // must have tzset(3)
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "sanity/system-error.hpp"

static char const PACKAGE_NAME[]    = "mini-httpd";
static char const PACKAGE_VERSION[] = "2007-05-04";

typedef boost::asio::io_service io_service;
static boost::scoped_ptr<io_service> the_io_service;

static void start_service()
{
  TRACE() << "enter i/o service thread pool";
  the_io_service->run();
}

static void stop_service()
{
  TRACE() << "received signal";
  the_io_service->stop();
}

int cpp_main(int argc, char** argv)
try
{
  using namespace std;
  using namespace http;

  // Set up the system.

  ios::sync_with_stdio(false);
  srand(time(0));
  tzset();
  init_logging(PACKAGE_NAME);

  // Create our configuration and place it in the global pointer.

  configuration real_config(argc, argv);
  config = &real_config;

  // Install signal handler.

  the_io_service.reset(new io_service);
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

  // Start-up scheduler and listener.

  using namespace boost::asio::ip;
  tcp::acceptor mini_httpd(*the_io_service, tcp::endpoint(tcp::v6(), config->http_port));
  tcp_driver< io_driver<http::daemon> >(mini_httpd);

  // Change root to our sandbox.

  if (!config->chroot_directory.empty())
  {
    if (chdir(config->chroot_directory.c_str()) == -1 || chroot(".") == -1)
      throw system_error((string("Can't change root to '") + config->chroot_directory + "'").c_str());
  }

  // Drop super user privileges.

  if (geteuid() == 0)
  {
    if (config->setgid_group && setgid(*config->setgid_group) == -1)
      throw system_error("setgid() failed");

    if (config->setuid_user && setuid(*config->setuid_user) == -1)
      throw system_error("setuid() failed");
  }

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

  // Log some helpful information.

  INFO()
    << PACKAGE_NAME << " " << PACKAGE_VERSION << " starting up"
    << ": tcp port = "  << config->http_port
    << ", user id = "   << ::getuid()
    << ", group id = "  << ::getgid()
    << ", chroot = "    << config->chroot_directory
    << ", hostname = "  << config->default_hostname
    << ", log dir = "   << ( config->logfile_directory.empty()
                           ? "disabled"
                           : config->logfile_directory.c_str()
                           )
    ;

  // Run ...

  start_service();

  // Terminate gracefully.

  INFO() << "mini-httpd shutting down";
  the_io_service.reset();

  return 0;
}
catch(http::configuration::no_error)
{
  return 0;
}
