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

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <csignal>
#include "io.hpp"
#include "sanity/system-error.hpp"

// ----- I/O Handlers ---------------------------------------------------------

struct tracer
{
  bool operator() (input_buffer & inbuf, std::size_t i, output_buffer & outbuf) const
  {
    size_t const size( inbuf.size() );
    BOOST_ASSERT(i <= size);
    if (size)
    {
      size_t const drop( !i ? size : static_cast<std::size_t>(rand()) % size );
      outbuf.append(inbuf.begin(), drop);
      inbuf.consume(drop);
    }
    TRACE() << "input_buffer handler: " << inbuf << "; outbuf " << outbuf;
    return i;
  }

  std::size_t operator() (byte_const_ptr begin, byte_const_ptr end, std::size_t i, output_buffer & outbuf) const
  {
    BOOST_ASSERT(begin <= end);
    size_t const size( static_cast<std::size_t>(end - begin) );
    BOOST_ASSERT(i <= size);
    if (size)
    {
      i = !i ? size : static_cast<std::size_t>(rand()) % size;
      outbuf.push_back(begin, begin + i);
    }
    return i;
  }
};

// ----- test program ---------------------------------------------------------

#include <boost/program_options.hpp>
#include <boost/test/prg_exec_monitor.hpp>

static char const PACKAGE_NAME[]    = "micro-httpd";
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

int cpp_main(int argc, char ** argv)
{
  using namespace std;
  srand(time(0));
  ios::sync_with_stdio(false);

  // Parse the command line.

  namespace po = boost::program_options;
  typedef boost::uint16_t               portnum_t;
  typedef vector<portnum_t>             portnum_array;
  typedef portnum_array::iterator       portnum_array_iterator;

  po::options_description meta_opts("Administrative Options");
  meta_opts.add_options()
    ( "help,h",      "produce help message and exit" )
    ( "version,v",   "show program version and exit" )
    ;

  size_t        n_threads;
  portnum_array listen_ports;
  string        change_root;
  string        logdir;
  string        server_string;
  int           uid;
  int           gid;
  string        default_hostname;
  string        document_root;
  string        default_page;

  po::options_description httpd_opts("Daemon Configuration");
  httpd_opts.add_options()
    ( "threads,j" ,        po::value<size_t>(&n_threads)->default_value(1u),                  "recommended value: number of CPUs"  )
    ( "no-detach,D",                                                                          "do not run in the background" )
    ( "port",              po::value< vector<portnum_t> >(&listen_ports),                     "listen on TCP port(s)"  )
    ( "change-root",       po::value<string>(&change_root),                                   "chroot(2) to this path after startup" )
    ( "uid",               po::value<int>(&uid),                                              "setgit(2) to this id after startup" )
    ( "gid",               po::value<int>(&gid),                                              "setgit(2) to this id after startup" )
    ( "document-root",     po::value<string>(&document_root)->default_value("htdocs"),        "directory containing HTML documents" )
    ( "log-dir",           po::value<string>(&logdir)->default_value("logs"),                 "write access logs to this directory" )
    ( "server-string",     po::value<string>(&server_string)->default_value(PACKAGE_NAME),    "set value of HTTP's \"Server:\" header" )
    ( "default-hostname",  po::value<string>(&default_hostname)->default_value("localhost"),  "hostname to use for pre HTTP/1.1")
    ( "default-page",      po::value<string>(&default_page)->default_value("index.html"),     "filename of directory index page" )
    ;

  po::options_description opts
    ( string("Commandline Interface ") + PACKAGE_NAME + " " + PACKAGE_VERSION)
    ;
  opts.add(meta_opts).add(httpd_opts);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
  po::notify(vm);

  if (vm.count("help"))      { cout << opts << endl;                                     return 0; }
  if (vm.count("version"))   { cout << PACKAGE_NAME << " " << PACKAGE_VERSION << endl;   return 0; }
  if (n_threads == 0u)       { cout << "no threads configured" << endl;                  return 1; }
  if (listen_ports.empty())  { cout << "no listen ports configured" << endl;             return 1; }
  if (default_page.empty())  { cout << "invalid argument --default-page=\"\"" << endl;   return 1; }
  if (document_root.empty()) { cout << "invalid argument --document-root=\"\"" << endl;  return 1; }
  bool const detach( !vm.count("no-detach") );

  // Setup the system.

  init_logging(PACKAGE_NAME);
  the_io_service.reset(new io_service);
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

  INFO()  << "version " << PACKAGE_VERSION
          << " running " << n_threads << " threads "
          << (detach ? "as daemon" : "on current tty");
#define TRACE_CONFIG_VAR(v) BOOST_LOGL(httpd_logger_debug, info) << TRACE_VAR(v)
  TRACE_CONFIG_VAR(change_root);
  TRACE_CONFIG_VAR(logdir);
  TRACE_CONFIG_VAR(server_string);
  TRACE_CONFIG_VAR(uid);
  TRACE_CONFIG_VAR(gid);
  TRACE_CONFIG_VAR(default_hostname);
  TRACE_CONFIG_VAR(document_root);
  TRACE_CONFIG_VAR(default_page);
#undef TRACE_CONFIG_VAR

  // Change root to our sandbox.

  if (change_root.empty())      INFO() << "change root disabled";
  else
  {
    INFO() << "change root to " << change_root;
    if (chdir(change_root.c_str()) == -1 || chroot(".") == -1)
      throw system_error((string("chroot(\"")) + change_root + "\") failed");
  }
#if 0
  // Drop super user privileges.

  if (geteuid() == 0)
  {
    if (config->setgid_group && setgid(*config->setgid_group) == -1)
      throw system_error("setgid() failed");

    if (config->setuid_user && setuid(*config->setuid_user) == -1)
      throw system_error("setuid() failed");
  }
#endif
  // Configure TCP listeners.

  using namespace boost::asio::ip;
  typedef boost::shared_ptr<tcp_acceptor> shared_acceptor;
  typedef vector<shared_acceptor>         acceptor_array;
  acceptor_array                          acceptors;
  for (portnum_array_iterator i( listen_ports.begin() ); i != listen_ports.end(); ++i)
  {
    tcp::endpoint const addr(tcp::v6(), *i);
    INFO() << "listening on address " << addr;
    shared_acceptor const acc( new tcp_acceptor(*the_io_service, addr) );
    acceptors.push_back(acc);
    tcp_driver< io_driver< stream_driver<tracer > > >(*acc);
  }

  // Detach from terminal.

  if (detach)
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

  // Run the server.

  boost::thread_group pool;
  while(--n_threads) pool.create_thread( &start_service );
  start_service();

  // Terminate gracefully.

  INFO() << "shutting down";

  pool.join_all();
  acceptors.clear();
  the_io_service.reset();

  return 0;
}
