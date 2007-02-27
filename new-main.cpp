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

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/range.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <csignal>
#include <cstdlib>              // std::size_t, std::ptrdiff_t
#include <cstring>              // std::memmove()
#include <boost/test/prg_exec_monitor.hpp>
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

// ----- Memory Buffers -------------------------------------------------------

using std::size_t;
using std::ptrdiff_t;

typedef char                                    byte;
typedef byte *                                  byte_ptr;
typedef byte const *                            byte_const_ptr;
typedef boost::iterator_range<byte_ptr>         byte_range;
typedef boost::iterator_range<byte_const_ptr>   byte_const_range;

typedef std::allocator<byte>                    byte_allocator;
typedef std::vector<byte, byte_allocator>       io_buffer;
typedef boost::shared_ptr<io_buffer>            shared_buffer;

static size_t const min_buf_size( 1024u );

class page : public byte_range
{
  io_buffer     _buf;

  page(page const &);
  page & operator= (page const &);

public:
  explicit page(size_t cap = 0u) : _buf(cap) { reset(); }

  byte_ptr        buf_begin()           { return &_buf[0]; }
  byte_const_ptr  buf_begin() const     { return &_buf[0]; }
  byte_ptr        buf_end()             { return &_buf[_buf.size()]; }
  byte_const_ptr  buf_end()   const     { return &_buf[_buf.size()]; }
  size_t          gap()       const     { return begin() - buf_begin(); }
  size_t          space()     const     { return buf_end() - end();  }
  bool            full()      const     { return buf_end() == end(); }

  void reset()
  {
    byte_range::operator=(byte_range(buf_begin(), buf_begin()));
  }

  void reset(byte_ptr b, byte_ptr e)
  {
    BOOST_ASSERT(buf_begin() <= b && b <= e && e <= buf_end());
    if (b == e) reset();
    else        byte_range::operator=(byte_range(b, e));
  }

  void pop_front(size_t i)  { reset(begin() + i, end()); }
  void push_back(size_t i)  { reset(begin(), end() + i); }

  void realloc(size_t n)
  {
    size_t const gap_( gap() );
    size_t const size_( size() );
    BOOST_ASSERT(gap_ + size_ < n);
    _buf.resize(n);
    byte_ptr const begin_( &_buf[gap_] );
    byte_ptr const end_( begin_ + size_ );
    reset(begin_, end_);
  }

  size_t flush_gap()
  {
    byte_ptr const buf_beg( buf_begin() );
    byte_ptr const data_beg( begin() );
    BOOST_ASSERT(buf_beg <= data_beg);
    size_t const gap_len( data_beg - buf_beg );
    if (gap_len)
    {
      size_t const data_len( size() );
      std::memmove(buf_beg, data_beg, data_len);
      reset(buf_beg, buf_beg + data_len);
    }
    return gap_len;
  }
};

inline std::ostream & operator<< (std::ostream & os, page const & buf)
{
  return os << "gap = " << buf.gap()
            << ", size = " << buf.size()
            << ", space = " << buf.space() ;
}

typedef boost::shared_ptr<page>         shared_page;

// ----- Central Services -----------------------------------------------------

typedef boost::asio::io_service         io_service;

typedef boost::asio::ip::tcp::socket    tcp_socket;
typedef boost::shared_ptr<tcp_socket>   shared_socket;

typedef boost::asio::ip::tcp::acceptor  tcp_acceptor;
typedef boost::shared_ptr<tcp_acceptor> shared_acceptor;

typedef boost::asio::ip::tcp::endpoint  tcp_endpoint;

template <class Handler>
inline void tcp_driver( tcp_acceptor & acc
                      , Handler        f = Handler()
                      , shared_socket  s = shared_socket()
                      )
{
  if (s) f(s);
  s.reset( new tcp_socket(acc.io_service()) );
  acc.async_accept(*s, boost::bind(&tcp_driver<Handler>, boost::ref(acc), f, s));
}

template <class Handler>
struct input_driver
{
  typedef void result_type;

  void operator()( shared_socket  s
                 , shared_page    iob   = shared_page(new page(min_buf_size))
                 , Handler        f     = Handler()
                 , size_t         i     = 0u
                 , bool           fresh = true
                 )      const
  {
    BOOST_ASSERT(s);
    BOOST_ASSERT(iob);
    if (!fresh)
    {
      iob->push_back(i);
      if (!f(iob, i) || !i)
        return;
    }
    if (iob->full())
      if (iob->flush_gap() == 0u)
        iob->realloc(std::max(min_buf_size, iob->size() * 2u));
    BOOST_ASSERT(!iob->full());
    using boost::asio::placeholders::bytes_transferred;
    s->async_read_some
      ( boost::asio::buffer(iob->end(), iob->space())
      , boost::bind( *this, s, iob, f, bytes_transferred, false )
      );
  }
};

template <class Handler>
struct stream_driver
{
  typedef bool result_type;

  bool operator()(shared_page & iob, size_t i, Handler f = Handler()) const
  {
    iob->pop_front( f(iob->begin(), iob->end(), i) );
    return i;
  }
};

template <class Handler>
struct slurp
{
  typedef bool result_type;

  bool operator() (shared_page & iob, std::size_t i, Handler f = Handler()) const
  {
    BOOST_ASSERT(iob);
    if (!i) f(iob);
    return i;
  }
};

// ----- handlers -------------------------------------------------------------

struct tracer
{
  void operator() (shared_socket const & s) const
  {
    TRACE() << "We just accepted (and ignored) a connection.";
  }

  bool operator() (shared_page & iob, std::size_t i) const
  {
    BOOST_ASSERT(iob);
    if (i)
    {
      iob->pop_front( (*this)(iob->begin(), iob->end(), i) );
      return true;
    }
    else
    {
      TRACE() << "page contains " << iob->size() << " bytes at time of eof";
      return false;
    }
  }

  std::size_t operator() (byte_const_ptr begin, byte_const_ptr end, std::size_t i) const
  {
    BOOST_ASSERT(begin <= end);
    BOOST_ASSERT(i <= static_cast<std::size_t>(end - begin));
    size_t const drop( static_cast<std::size_t>(rand()) % (end - begin));
    TRACE() << "streambuf: read " << i << " bytes, dropping " << drop;;
    return drop;
  }

  void operator() (shared_page const & iob) const
  {
    BOOST_ASSERT(iob);
    TRACE() << "slurped buffer: " << *iob;
  }
};

// ----- test program ---------------------------------------------------------

static boost::scoped_ptr<io_service> the_io_service;

static void stop_service()
{
  TRACE() << "received signal";
  the_io_service->stop();
}

int cpp_main(int argc, char ** argv)
{
  using namespace std;

  // Setup the system.

  srand(time(0));
  ios::sync_with_stdio(false);
  init_logging();
  the_io_service.reset(new io_service);
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

  // Start the server.

  tcp_acceptor port2525(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2525));
  tcp_driver<tracer>(port2525);

  tcp_acceptor port2526(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2526));
  tcp_driver< input_driver< stream_driver< tracer > > >(port2526);

  tcp_acceptor port2527(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2527));
  tcp_driver< input_driver<tracer> >(port2527);

  tcp_acceptor port2528(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2528));
  tcp_driver< input_driver< slurp<tracer> > >(port2528);

  boost::thread t1(boost::bind(&io_service::run, the_io_service.get()));
  boost::thread t2(boost::bind(&io_service::run, the_io_service.get()));
  boost::thread t3(boost::bind(&io_service::run, the_io_service.get()));
  the_io_service->run();

  // Shut down gracefully.

  t1.join(); t2.join(); t3.join();
  the_io_service.reset();

  return 0;
}
