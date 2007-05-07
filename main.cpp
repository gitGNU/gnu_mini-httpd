/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef BOOST_DISABLE_THREADS
#  include <boost/thread/thread.hpp>
#endif
#include <boost/scoped_ptr.hpp>
#include <csignal>
#include <boost/program_options.hpp>
#include <boost/test/prg_exec_monitor.hpp>
#include "sanity/system-error.hpp"
#include "http-daemon.hpp"
#include "io-driver.hpp"
#include "logging.hpp"

#ifndef PACKAGE_NAME
#  ifndef BOOST_DISABLE_THREADS
static char const PACKAGE_NAME[]    = "mini-httpd";
#  else
static char const PACKAGE_NAME[]    = "micro-httpd";
#  endif
#endif
#ifndef PACKAGE_VERSION
static char const PACKAGE_VERSION[] = "2007-05-07";
#endif

typedef boost::asio::io_service io_service;
static boost::scoped_ptr<io_service> the_io_service;

static void start_service()
{
#ifndef BOOST_DISABLE_THREADS
  TRACE() << "enter i/o service thread pool";
#else
  TRACE() << "enter i/o loop";
#endif
  the_io_service->run();
}

static void stop_service()
{
  TRACE() << "received signal";
  the_io_service->stop();
}

int cpp_main(int argc, char ** argv)
{
  using namespace std;
  namespace po = boost::program_options;
  //
  // Ignore relevant signals to avoid race conditions.
  //
  signal(SIGTERM, SIG_IGN);
  signal(SIGINT,  SIG_IGN);
  signal(SIGHUP,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  //
  // Seed the random number generator.
  //
  srand(time(0));
  //
  // stdout/stderr are accessed only through the C++ streams API.
  //
  ios::sync_with_stdio(false);
  //
  // Parse the command line: Generic Process and Daemon Options
  //
#ifndef BOOST_DISABLE_THREADS
  size_t                n_threads;
#endif
  string                change_root;
  uid_t                 uid;
  gid_t                 gid;
  vector<string>        listen_addrs;
  po::options_description meta_opts("Administrative Options");
  meta_opts.add_options()
    ( "help,h",                                                                                 "produce help message and exit" )
    ( "version,v",                                                                              "show program version and exit" )
    ( "no-detach,D",                                                                            "don't run in the background" )
#ifndef BOOST_DISABLE_THREADS
    ( "threads,j" ,        po::value<size_t>(&n_threads)->default_value(2u),                    "recommended value: number of CPUs"  )
#endif
    ( "change-root",       po::value<string>(&change_root),                                     "chroot(2) to this path after startup" )
    ( "uid",               po::value<uid_t>(&uid),                                              "setuid(2) to this id after startup" )
    ( "gid",               po::value<gid_t>(&gid),                                              "setgid(2) to this id after startup" )
    ( "listen",            po::value< vector<string> >(&listen_addrs),                          "listen on address:port"  )
    ;
  po::positional_options_description pos_opts;
  pos_opts.add("listen", -1);
  //
  // Parse the command line: HTTP Page Delivery and Logging
  //
  http::configuration & cfg( http::daemon::_config );
  po::options_description httpd_opts("HTTP Daemon Configuration");
  httpd_opts.add_options()
    ( "document-root",     po::value<string>(&cfg.document_root)->default_value("htdocs"),      "directory containing HTML documents" )
    ( "log-dir",           po::value<string>(&cfg.logfile_root)->default_value("htlogs"),       "write access logs to this directory" )
    ( "server-string",     po::value<string>(&cfg.server_string)->default_value(PACKAGE_NAME),  "set value of HTTP's \"Server:\" header" )
    ( "default-hostname",  po::value<string>(&cfg.default_hostname),                            "hostname to use for pre HTTP/1.1")
    ( "default-page",      po::value<string>(&cfg.default_page)->default_value("index.html"),   "filename of directory index page" )
    ;
  //
  // Run command line parser. Obvious errors are thrown as exceptions.
  //
  po::options_description opts
    ( string("Commandline Interface ") + PACKAGE_NAME + " " + PACKAGE_VERSION)
    ;
  opts.add(meta_opts).add(httpd_opts);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(opts).positional(pos_opts).run(), vm);
  po::notify(vm);
  //
  // Sanity checking ...
  //
  if (vm.count("help"))          { cout << opts << endl;                                     return 0; }
  if (vm.count("version"))       { cout << PACKAGE_NAME << " " << PACKAGE_VERSION << endl;   return 0; }
#ifndef BOOST_DISABLE_THREADS
  if (n_threads == 0u)           { cout << "no threads configured" << endl;                  return 1; }
#endif
  if (listen_addrs.empty())      { cout << "no listen addresses configured" << endl;         return 1; }
  if (cfg.default_page.empty())  { cout << "invalid argument --default-page=\"\"" << endl;   return 1; }
  if (cfg.document_root.empty()) { cout << "invalid argument --document-root=\"\"" << endl;  return 1; }
  bool const detach( !vm.count("no-detach") );
  //
  // Setup the system and log configuration.
  //
  init_logging(PACKAGE_NAME);
  the_io_service.reset(new io_service);
  INFO() << PACKAGE_NAME << " version " << PACKAGE_VERSION << " running "
#ifndef BOOST_DISABLE_THREADS
         << n_threads << " threads "
#endif
         << (detach ? "as daemon" : "on current tty")
         << " using config: " << cfg;
  //
  // Configure TCP listeners.
  //
  using namespace boost::asio::ip;
  typedef boost::shared_ptr<tcp_acceptor> shared_acceptor;
  typedef vector<shared_acceptor>         acceptor_array;
  acceptor_array                          acceptors;
  for (vector<string>::iterator i( listen_addrs.begin() ); i != listen_addrs.end(); ++i)
  {
    tcp::endpoint addr;
    string::size_type k( i->rfind(':') );
    switch (k)
    {
      case string::npos:
        TRACE() << "parse TCP port '" << *i << "'";
        addr = tcp::endpoint(tcp::v6(), atoi(i->c_str()));
        break;

      case 0u:
        TRACE() << "parse any host, TCP port '" << *i << "'";
        addr = tcp::endpoint(tcp::v6(), atoi(i->c_str() + 1u));
        break;

      default:
        TRACE() << "parse host '" << i->substr(0, k)
                << "', port '"    << i->substr(k + 1u)
                << "'";
        addr = tcp::endpoint( address::from_string(i->substr(0, k))
                            , atoi(i->substr(k + 1u).c_str())
                            );
    }
    INFO() << "listen on network address " << addr;
    shared_acceptor const acc( new tcp_acceptor(*the_io_service, addr) );
    acceptors.push_back(acc);
    tcp_driver< io_driver<http::daemon> >(*acc);
  }
  //
  // Drop all privileges.
  //
  if (change_root.empty())      TRACE() << "disabled change root";
  else
  {
    INFO() << "change root to " << change_root;
    if (chdir(change_root.c_str()) == -1 || chroot(".") == -1)
      throw system_error((string("chroot(\"")) + change_root + "\") failed");
  }
  if (!vm.count("gid"))         TRACE() << "disabled change group id";
  else
  {
    INFO() << "change process group id to " << gid;
    if (setgid(gid) == -1) throw system_error("setgid() failed");
  }
  if (!vm.count("uid"))         TRACE() << "disabled change user id";
  else
  {
    INFO() << "change process user id to " << uid;
    if (setuid(uid) == -1) throw system_error("setuid() failed");
  }
  //
  // Detach from terminal.
  //
  if (detach)
  {
    switch (fork())
    {
      case -1:
        throw system_error("fork() failed");
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
  //
  // Run the server.
  //
#ifndef BOOST_DISABLE_THREADS
  boost::thread_group pool;
  while(--n_threads) pool.create_thread( &start_service );
#endif
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  start_service();
  //
  // Terminate gracefully.
  //
  INFO() << "shutting down";
#ifndef BOOST_DISABLE_THREADS
  pool.join_all();
#endif
  acceptors.clear();
  the_io_service.reset();
  return 0;
}
