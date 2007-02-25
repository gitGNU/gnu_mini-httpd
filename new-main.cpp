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

#include <csignal>
#include <cstdlib>
#include <functional>
#include <boost/test/prg_exec_monitor.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>

#include <boost/log/log_impl.hpp>
#include <boost/log/functions.hpp>

// ----- Logging --------------------------------------------------------------

BOOST_DECLARE_LOG_DEBUG(logger)
BOOST_DEFINE_LOG(logger, "httpd.new-main")
#define TRACE() BOOST_LOGL(logger, dbg)
#define INFO()  BOOST_LOGL(logger, info)

static void write_to_cerr(std::string const & prefix, std::string const & msg)
{
  std::cerr << '[' << prefix << "] " << msg << std::endl;
}

static void init_logging()
{
  using namespace boost::logging;
  manipulate_logs("*").add_appender(&write_to_cerr);
  flush_log_cache();
}

// ----- Central Services -----------------------------------------------------

typedef std::vector<char>               io_buffer;
typedef boost::shared_ptr<io_buffer>    shared_buffer;

typedef boost::asio::io_service         io_service;
typedef boost::shared_ptr<io_service>   shared_service;

typedef boost::asio::ip::tcp::socket    tcp_socket;
typedef boost::shared_ptr<tcp_socket>   shared_socket;

typedef boost::asio::ip::tcp::acceptor  tcp_acceptor;
typedef boost::asio::ip::tcp::endpoint  tcp_endpoint;

template <class Handler>
inline void drive_tcp( tcp_acceptor &   acc
                     , Handler          f = Handler()
                     , shared_socket    s = shared_socket()
                     )
{
  if (s) f(s);
  s.reset( new tcp_socket(acc.io_service()) );
  acc.async_accept(*s, boost::bind(&drive_tcp<Handler>, boost::ref(acc), f, s));
}

template <class Handler>
struct drive_input
{
  typedef void result_type;

  void operator()( shared_socket  s
                 , Handler        f   = Handler()
                 , shared_buffer  iob = shared_buffer(new io_buffer(1024u))
                 , std::size_t    gap = 0u
                 , std::size_t    len = 0u
                 , std::size_t    i   = 0u
                 , bool           ini = true
                 )
    const
  {
    BOOST_ASSERT(iob->size());
    BOOST_ASSERT(gap + len + i <= iob->size());
    if (!ini && !i) { TRACE() << "eof"; return; }
    if (i)
    {
      TRACE() << i << " new bytes of input available, run handler";
      std::size_t const n( f( &(*iob)[gap], &(*iob)[gap + len + i], i) );
      TRACE() << "handler consumed " << n << " bytes";
      len += i;
      BOOST_ASSERT(n <= len);
      len -= n;
      gap = len ? gap + n : 0u;
      BOOST_ASSERT(gap + len <= iob->size());
    }
    TRACE() << "buffer gap = " << gap << ", len = " << len << ", cap = " << iob->size();
    if (gap + len == iob->size())
    {
      TRACE() << "buffer is full";
      if (gap)
      {
        TRACE() << "flush gap";
        std::copy( &(*iob)[gap], &(*iob)[gap + len], &(*iob)[0] );
        gap = 0u;
      }
      else
      {
        iob->reserve(iob->size() * 2);
        iob->resize(iob->capacity());
        TRACE() << "re-sized buffer to " << iob->size() << " bytes";
      }
    }
    BOOST_ASSERT(gap + len < iob->size());
    TRACE() << "read at most " << (iob->size() - gap - len) << " bytes";
    s->async_read_some
      ( boost::asio::buffer(&(*iob)[gap + len], iob->size() - gap - len)
      , boost::bind( *this, s, f, iob, gap, len
                   , boost::asio::placeholders::bytes_transferred
                   , false
                   )
      );
  }
};

// ----- TCP Session ----------------------------------------------------------

class session
{
  tcp_socket            _sock;
  std::vector<char>     _buf;

public:
  session(io_service & ios) : _sock(ios), _buf(1024u)
  {
    TRACE() << "init session";
  }

  ~session()
  {
    TRACE() << "close session";
  }

  tcp_socket & socket()     { return _sock; }

  void start()
  {
    _sock.async_read_some
      ( boost::asio::buffer(_buf, _buf.size())
      , boost::bind( &session::handle_read, this
                   , boost::asio::placeholders::error
                   , boost::asio::placeholders::bytes_transferred
                   )
      );
  }

  void handle_read(boost::system::error_code const & error, size_t bytes_transferred)
  {
    if (!error)
    {
      boost::asio::async_write(_sock,
          boost::asio::buffer(_buf, bytes_transferred),
          boost::bind(&session::handle_write, this,
            boost::asio::placeholders::error));
    }
    else
    {
      delete this;
    }
  }

  void handle_write(const boost::system::error_code& error)
  {
    if (!error)
    {
      _sock.async_read_some(boost::asio::buffer(_buf, _buf.size()),
          boost::bind(&session::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      delete this;
    }
  }
};

class server
{
public:
  server(io_service & ios, short port)
    : io_service_(ios), acceptor_(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
  {
    TRACE() << __PRETTY_FUNCTION__;
    session* new_session = new session(io_service_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
  }

  ~server()
  {
    TRACE() << __PRETTY_FUNCTION__;
  }

  void handle_accept(session* new_session,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_session->start();
      new_session = new session(io_service_);
      acceptor_.async_accept(new_session->socket(),
          boost::bind(&server::handle_accept, this, new_session,
            boost::asio::placeholders::error));
    }
    else
    {
      delete new_session;
    }
  }

private:
  io_service &          io_service_;
  tcp_acceptor          acceptor_;
};

io_service * the_io_service;

static void stop_service()
{
  TRACE() << "received signal";
  the_io_service->stop();
}

struct trace_accept
{
  void operator() (shared_socket const & s) const
  {
    TRACE() << "We just accepted a connection.";
  }
};

struct trace_input
{
  std::size_t operator() (char const * begin, char const * end, std::size_t i) const
  {
    BOOST_ASSERT(begin < end);
    BOOST_ASSERT(i);
    BOOST_ASSERT(i <= static_cast<std::size_t>(end - begin));
    TRACE() << "We just received " << i << " bytes of input";
    return static_cast<std::size_t>(rand()) % (end - begin);
  }
};

int cpp_main(int argc, char ** argv)
{
  using namespace std;

  // Make sure usage is correct.

  if (argc != 2)
  {
    INFO() << "Usage: " << argv[0] << " <port>";
    return 1;
  }

  // Setup the system.

  srand(time(0));
  ios::sync_with_stdio(false);
  init_logging();

  io_service global_ios;
  the_io_service = &global_ios;
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

  // Start the server.

  tcp_acceptor port2525(global_ios, tcp_endpoint(boost::asio::ip::tcp::v4(), 2525));
  drive_tcp<trace_accept>(port2525);

  tcp_acceptor port2526(global_ios, tcp_endpoint(boost::asio::ip::tcp::v4(), 2526));
  drive_tcp< drive_input<trace_input> >(port2526);

  server s(global_ios, atoi(argv[1]));
  boost::thread t1(boost::bind(&io_service::run, the_io_service));
  boost::thread t2(boost::bind(&io_service::run, the_io_service));
  boost::thread t3(boost::bind(&io_service::run, the_io_service));
  global_ios.run();
  t1.join(); t2.join(); t3.join();

  // Shut down gracefully.

  return 0;
}
