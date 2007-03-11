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

// ----- I/O Socket -----------------------------------------------------------

class io_stream : private boost::noncopyable
{
public:
  tcp_socket    _sock;
  input_buffer  _inbuf;
  output_buffer _outbuf;

  bool          _more_input;
  bool          _reading;
  bool          _writing;

  io_stream(boost::asio::io_service & ios)
    : _sock(ios), _more_input(false), _reading(false), _writing(false)
  {
  }

  virtual ~io_stream()          { }
  virtual void initialize()     { _more_input = true; run(); }

  void run()
  {
    TRACE_VAR3(_more_input, _reading, _writing);
    TRACE_VAR2(_inbuf, _outbuf);
    if (_more_input)
    {
      if (!_writing)                                            consume_input();
      if (!_writing && !_outbuf.empty())                        start_writing();
      if (!_reading && !_writing && !_inbuf.back_space())       _inbuf.flush();
      if (!_reading && _inbuf.back_space())                     start_reading();
    }
    else
    {
      BOOST_ASSERT(!_reading);
      if (!_writing)
      {
        if (!_outbuf.empty())   start_writing();
        else                    terminate();
      }
    }
  }

  virtual void terminate()
  {
    TRACE_VAR2(_inbuf, _outbuf);
    BOOST_ASSERT(!_more_input);
    BOOST_ASSERT(!_reading);
    BOOST_ASSERT(!_writing);
    delete this;
  }

  virtual void start_reading()
  {
    TRACE_VAR1(_inbuf);
    BOOST_ASSERT(_more_input);
    BOOST_ASSERT(!_reading);
    size_t const space( _inbuf.back_space() );
    BOOST_ASSERT(space);
    using boost::asio::placeholders::bytes_transferred;
    _sock.async_read_some
      ( boost::asio::buffer(_inbuf.end(), space)
      , boost::bind( &io_stream::handle_read, this, bytes_transferred )
      );
    _reading = true;
  }

  virtual void handle_read(size_t i)
  {
    TRACE_VAR2(i, _inbuf);
    BOOST_ASSERT(_more_input);
    BOOST_ASSERT(_reading);
    _reading = false;
    _more_input = (i != 0u);
    _inbuf.append(i);
    run();
  }

  virtual void start_writing()
  {
    TRACE_VAR1(_outbuf);
    BOOST_ASSERT(!_writing);
    scatter_vector const & iov( _outbuf.commit() );
    BOOST_ASSERT(!iov.empty());
    using boost::asio::placeholders::bytes_transferred;
    _sock.async_write_some
      ( iov
      , boost::bind( &io_stream::handle_write, this, bytes_transferred )
      );
    _writing = true;
  }

  virtual void handle_write(size_t i)
  {
    TRACE_VAR2(i, _outbuf);
    BOOST_ASSERT(_writing);
    BOOST_ASSERT(i > 0u);
    _writing = false;
    _outbuf.consume(i);
    if (_outbuf.empty()) _outbuf.flush();
    run();
  }

  virtual void consume_input()
  {
    _outbuf.append(_inbuf.begin(), _inbuf.end());
    _inbuf.consume(_inbuf.size());
  }
};



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
static char const PACKAGE_VERSION[] = "2007-02-19";

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

  po::options_description meta_opts("Administrative Options");
  meta_opts.add_options()
    ( "help,h",      "produce help message and exit" )
    ( "version,v",   "show program version and exit" )
    ;

  size_t n_threads;
  po::options_description httpd_opts("Daemon Configuration");
  httpd_opts.add_options()
    ( "threads,j" , po::value<size_t>(&n_threads)->default_value(1u), "recommended value: number of CPUs"  )
    ( "no-detach,D",                                                  "do not run in the background" )
    ;

  po::options_description opts
    ( string("Commandline Interface ") + PACKAGE_NAME + " " + PACKAGE_VERSION)
    ;
  opts.add(meta_opts).add(httpd_opts);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(opts).run(), vm);
  po::notify(vm);

  if (vm.count("help"))     { cout << opts << endl;                    return 0; }
  if (vm.count("version"))  { cout << PACKAGE_VERSION << endl;         return 0; }
  if (n_threads == 0u)      { cout << "no threads configured" << endl; return 1; }
  bool const detach( !vm.count("no-detach") );

  // Setup the system.

  init_logging(PACKAGE_NAME);
  the_io_service.reset(new io_service);
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

  INFO() << "version " << PACKAGE_VERSION
         << " running " << n_threads << " threads "
         << (detach ? "as daemon" : "on current tty");

  // Configure TCP listeners.

  using namespace boost::asio::ip;

  tcp::acceptor port2525(*the_io_service, tcp::endpoint(tcp::v6(), 2525));
  tcp_driver< io_driver<tracer> >(port2525);

  tcp::acceptor port2526(*the_io_service, tcp::endpoint(tcp::v6(), 2526));
  tcp_driver< io_driver< stream_driver< tracer > > >(port2526);

  io_stream s( *the_io_service );

  // Run the server.

  boost::thread_group pool;
  while(--n_threads) pool.create_thread( &start_service );
  start_service();

  // Terminate gracefully.

  INFO() << "shutting down";

  pool.join_all();
  the_io_service.reset();

  return 0;
}
