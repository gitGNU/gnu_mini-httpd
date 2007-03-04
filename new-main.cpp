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

// ----- Core Types -----------------------------------------------------------

using std::size_t;
using std::ptrdiff_t;

static size_t const min_buf_size( 1024u );

typedef char                                    byte;
typedef std::allocator<byte>                    byte_allocator;
typedef byte_allocator::pointer                 byte_ptr;
typedef byte_allocator::const_pointer           byte_const_ptr;
typedef std::vector<byte, byte_allocator>       io_vector;

typedef boost::iterator_range<byte_ptr>         byte_range;
typedef boost::iterator_range<byte_const_ptr>   byte_const_range;

// ----- Input ----------------------------------------------------------------

class input_buffer : public byte_range
{
  io_vector _buf;

  input_buffer(input_buffer const &);
  input_buffer & operator= (input_buffer const &);

public:
  input_buffer(size_t cap = 0u) : _buf(cap)    { reset(); }

  byte_ptr        buf_begin()           { return &_buf[0]; }
  byte_const_ptr  buf_begin()  const    { return &_buf[0]; }
  byte_ptr        buf_end()             { return &_buf[_buf.size()]; }
  byte_const_ptr  buf_end()    const    { return &_buf[_buf.size()]; }
  size_t          front_gap()  const    { return begin() - buf_begin(); }
  size_t          back_space() const    { return buf_end() - end(); }

  void reset()
  {
    byte_range::operator= (byte_range(buf_begin(), buf_begin()));
  }

  void reset(byte_ptr b, byte_ptr e)
  {
    BOOST_ASSERT(buf_begin() <= b && b <= e && e <= buf_end());
    if (b == e) reset();
    else        byte_range::operator= (byte_range(b, e));
  }

  void consume(size_t i) { reset(begin() + i, end()); }
  void append(size_t i)  { reset(begin(), end() + i); }

  size_t flush_gap()
  {
    size_t const gap( front_gap() );
    if (gap)
    {
      byte_ptr const base( buf_begin() );
      size_t const len( size() );
      std::memmove(base, begin(), len);
      reset(base, base + len);
    }
    return gap;
  }

  void realloc(size_t n)
  {
    size_t const len( size() );
    BOOST_ASSERT(len < n);
    io_vector new_buf(n);
    std::memmove(&new_buf[0], begin(), len);
    _buf.swap(new_buf);
    reset(buf_begin(), buf_begin() + len);
  }
};

inline std::ostream & operator<< (std::ostream & os, input_buffer const & b)
{
  return os << "gap = "     << b.front_gap()
            << ", size = "  << b.size()
            << ", space = " << b.back_space();
}

// ----- Output ---------------------------------------------------------------

typedef boost::asio::const_buffer   scatter_buffer;
typedef std::vector<scatter_buffer> scatter_vector;

class output_buffer : private boost::noncopyable
{
  scatter_vector        _iovec;
  io_vector             _buf;

  struct fix_base
  {
    byte_const_ptr const end;
    ptrdiff_t const      fix;

    fix_base(output_buffer & iob) : end( byte_const_ptr(0) + iob._buf.size() )
                                 , fix( &iob._buf[0] - byte_const_ptr(0) )
    {
    }

    void operator() (boost::asio::const_buffer & iov) const
    {
      byte_const_ptr const  b( boost::asio::buffer_cast<byte_const_ptr>(iov) );
      size_t const          len( boost::asio::buffer_size(iov) );
      if (b + len <= end)
        iov = boost::asio::const_buffer(b + fix, len);
    }
  };

public:
  void append(scatter_buffer const & iov) { _iovec.push_back(iov); }

  template <class Iter>
  void append(Iter b, Iter e)
  {
    size_t const old_len( _buf.size() );
    _buf.insert(_buf.end(), b, e);
    size_t const new_len( _buf.size() );
    BOOST_ASSERT(old_len <= new_len);
    if (old_len != new_len)
      append(scatter_buffer(byte_const_ptr(0) + old_len, new_len - old_len));
  }

  scatter_vector const & commit()
  {
    std::for_each(_iovec.begin(), _iovec.end(), fix_base(*this));
    return _iovec;
  }

  void reset()
  {
    _iovec.clear();
    _buf.clear();
  }

  friend inline std::ostream & operator<< (std::ostream & os, output_buffer const & b)
  {
    return os << "iovecs = "     << b._iovec.size()
              << ", buffered = "   << b._buf.size();
  }
};

// ----- I/O Driver -----------------------------------------------------------

typedef boost::asio::io_service                 io_service;
typedef boost::asio::ip::tcp::socket            tcp_socket;
typedef boost::asio::ip::tcp::endpoint          tcp_endpoint;
typedef boost::asio::ip::tcp::acceptor          tcp_acceptor;

typedef boost::shared_ptr<tcp_socket>           shared_socket;
typedef boost::shared_ptr<tcp_acceptor>         shared_acceptor;
typedef boost::shared_ptr<io_vector>            shared_vector;
typedef boost::shared_ptr<input_buffer>         shared_input_buffer;
typedef boost::shared_ptr<output_buffer>        shared_output_buffer;

template <class Handler>        // void (shared_socket &)
inline void tcp_driver( tcp_acceptor & acc
                      , Handler        f = Handler()
                      , shared_socket  s = shared_socket()
                      )
{
  if (s) f.start(s);
  s.reset(new tcp_socket( acc.io_service() ));
  acc.async_accept(*s, boost::bind(&tcp_driver<Handler>, boost::ref(acc), f, s));
}

template <class Handler>        // bool (input_buffer &, size_t, output_buffer &)
class io_driver : public Handler
{
  shared_socket         _insock, _outsock;
  shared_input_buffer   _inbuf;
  shared_output_buffer  _outbuf;

public:
  io_driver() { }

  typedef void result_type;

  void start(shared_socket const sin) { start(sin, sin); }

  void start(shared_socket const & sin, shared_socket const & sout)
  {
    TRACE() << "initialize io_driver";
    BOOST_ASSERT(sin && sout);
    _insock = sin;
    _outsock = sout;
    _inbuf.reset(new input_buffer(min_buf_size));
    _outbuf.reset(new output_buffer);
    start_read();
  }

  void stop() { *this = io_driver(); }

  void start_read()
  {
    size_t space( _inbuf->back_space() );
    if (!space)
    {
      if (!_inbuf->flush_gap())
        _inbuf->realloc(std::max(min_buf_size, _inbuf->size() * 2u));
      space = _inbuf->back_space();
      BOOST_ASSERT(space);
    }
    TRACE() << "start reading for " << space << " bytes";
    using boost::asio::placeholders::bytes_transferred;
    _insock->async_read_some
      ( boost::asio::buffer(_inbuf->end(), space)
      , boost::bind( &io_driver::new_input, *this, bytes_transferred )
      );
  }

  void new_input(std::size_t i)
  {
    TRACE() << "process " << i << " new input bytes";
    _outbuf->reset();
    _inbuf->append(i);
    bool const more_input( Handler::operator()(*_inbuf, i, *_outbuf) && i );
    scatter_vector const & outbuf( _outbuf->commit() );
    if (more_input)
    {
      if (outbuf.empty()) start_read();
      else                start_write(outbuf);
    }
    else
    {
      if (outbuf.empty()) stop();
      else                last_write(outbuf);
    }
  }

  void start_write(scatter_vector const & buf)
  {
    TRACE() << "start writing " << buf.size() << " iovecs";
    boost::asio::async_write(*_outsock, buf, boost::bind(&io_driver::start_read, *this));
  }

  void last_write(scatter_vector const & buf)
  {
    _insock.reset();
    TRACE() << "flush " << buf.size() << " iovecs";
    boost::asio::async_write(*_outsock, buf, boost::bind(&io_driver::stop, *this));
  }
};

// template <class Handler>        // size_t (byte_ptr, byte_ptr, size_t)
// struct stream_driver
// {
//   typedef bool result_type;
//
//   bool operator()(input_buffer & inbuf, size_t i, Handler f = Handler()) const
//   {
//     TRACE() << "stream handler entry: " << inbuf << ", new input = " << i;
//     size_t k( f(inbuf.begin(), inbuf.end(), i) );
//     inbuf.consume(k);
//     if (i)
//       while (k && !inbuf.empty())
//       {
//         k = f(inbuf.begin(), inbuf.end(), std::min(i, inbuf.size()));
//         inbuf.consume(k);
//       }
//     TRACE() << "stream handler exit: " << inbuf;
//     return i;
//   }
// };
//
// template <class Handler>        // void (shared_input_buffer &)
// struct slurp
// {
//   typedef bool result_type;
//
//   bool operator() (input_buffer & inbuf, std::size_t i, Handler f = Handler()) const
//   {
//     if (!i) f(inbuf);
//     return i;
//   }
// };

// ----- handlers -------------------------------------------------------------

struct tracer
{
  bool operator() (input_buffer & inbuf, std::size_t i, output_buffer & outbuf) const
  {
    size_t const size( inbuf.size() );
    BOOST_ASSERT(i <= size);
    if (size)
    {
      size_t const drop( !i ? size : static_cast<std::size_t>(rand()) % size );
      outbuf.append(boost::asio::buffer(inbuf.begin(), drop));
      inbuf.consume(drop);
    }
    TRACE() << "input_buffer handler: " << inbuf << "; outbuf " << outbuf;
    return i;
  }

  std::size_t operator() (byte_const_ptr begin, byte_const_ptr end, std::size_t i) const
  {
    BOOST_ASSERT(begin <= end);
    size_t const size( static_cast<std::size_t>(end - begin) );
    BOOST_ASSERT(i <= size);
    if (size)   i = static_cast<std::size_t>(rand()) % size;
    else        BOOST_ASSERT(!i);
    TRACE() << "range handler: drop " << i;
    return i;
  }

  void operator() (input_buffer const & inbuf) const
  {
    TRACE() << "slurped buffer: " << inbuf;
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

  INFO() << "new-mini-httpd 2007-03-04 starting up";

//   tcp_acceptor port2526(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2526));
//   tcp_driver< io_driver< stream_driver< tracer > > >(port2526);

  tcp_acceptor port2527(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2527));
  tcp_driver< io_driver<tracer> >(port2527);

//   tcp_acceptor port2528(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2528));
//   tcp_driver< io_driver< slurp<tracer> > >(port2528);

  boost::thread t1(boost::bind(&io_service::run, the_io_service.get()));
  boost::thread t2(boost::bind(&io_service::run, the_io_service.get()));
  boost::thread t3(boost::bind(&io_service::run, the_io_service.get()));
  the_io_service->run();

  INFO() << "new-mini-httpd 2007-03-04 shutting down";

  // Shut down gracefully.

  t1.join(); t2.join(); t3.join();
  the_io_service.reset();

  return 0;
}
