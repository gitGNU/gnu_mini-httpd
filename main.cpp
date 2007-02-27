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

#include <stdexcept>
#include <csignal>
#include <ctime>                // must have tzset(3)
#include <iostream>
#include <boost/log/log_impl.hpp>
#include <boost/log/functions.hpp>
#include <boost/scoped_ptr.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "sanity/system-error.hpp"
#include "ioxx/tcp-acceptor.hpp"
#include "ioxx/logging.hpp"
#include "http-daemon.hpp"

using namespace std;
using namespace ioxx;

namespace
{
  void write_to_cerr(std::string const &, std::string const & msg)
  {
    cerr << msg << endl;
  }

  void init_logging()
  {
    using namespace boost::logging;
    manipulate_logs("*")
      .add_modifier(&prepend_prefix)
      .add_appender(&write_to_cerr);
    flush_log_cache();
  }

  volatile sig_atomic_t got_terminate_sig = false;

  void set_sig_term(int)
  {
    got_terminate_sig = true;
  }
}

int main(int argc, char** argv)
try
{
  using namespace http;

  ios::sync_with_stdio(false);
  init_logging();

  // Initialize the global variables telling us our time zone.

  tzset();

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

  boost::scoped_ptr<probe> probe( make_probe_poll() );
  if (!probe) throw system_error("no probe implementation");
  socket listen_port( add_tcp_service<http::daemon>(*probe, config->http_port) );

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

  BOOST_LOGL(::http::logging::misc, info)
    << "mini-httpd 2007-02-27 starting up"
    << ": listen port = "  << config->http_port
    << ", user id = "      << ::getuid()
    << ", group id = "     << ::getgid()
    << ", chroot = "       << config->chroot_directory
    << ", hostname = "     << config->default_hostname
    ;

  // Run ...

  while(!got_terminate_sig && probe->active())  probe->run_once();

  // Exit gracefully.

  BOOST_LOGL(::http::logging::misc, info) << "mini-httpd shutting down";
  return 0;
}
catch(http::configuration::no_error)
{
  return 0;
}
