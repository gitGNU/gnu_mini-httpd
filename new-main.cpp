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
#include <boost/log/extra/functions_ts.hpp>
#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
#  include <syslog.h>
#endif

static char const PACKAGE_NAME[]    = "micro-httpd";
static char const PACKAGE_VERSION[] = "2007-02-19";

// ----- Logging --------------------------------------------------------------

BOOST_DECLARE_LOG(logger_info)
BOOST_DEFINE_LOG(logger_info,  "info")
#define INFO()  BOOST_LOGL(logger_info, info)

BOOST_DECLARE_LOG_DEBUG(logger_debug)
BOOST_DEFINE_LOG(logger_debug, "debug")
#define TRACE() BOOST_LOGL(logger_debug, dbg)

static void line_writer(std::string const & /* prefix */, std::string const & msg)
{
#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
  syslog(LOG_INFO, "%s", msg.c_str());
#else
  std::cout << msg << std::endl;
#endif
}

#ifdef BOOST_HAS_PTHREADS
static void add_thread_id(std::string const & prefix, std::string & msg)
{
  using namespace std;
  char tid[64];
  int len( snprintf(tid, sizeof(tid), "[tid %lx] ", pthread_self()) );
  BOOST_ASSERT(static_cast<size_t>(len) < sizeof(tid) );
  msg.insert(msg.begin(), tid, tid + len);
}
#endif

static void init_logging()
{
  using namespace boost::logging;
  manipulate_logs("*")
    .add_appender( line_writer )
#if !defined(_POSIX_VERSION) || (_POSIX_VERSION < 200100)
    .add_modifier( prepend_time("$yyyy-$MM-$ddT$hh:$mm:$ss ") )
#endif
    ;
#ifdef BOOST_HAS_PTHREADS
  manipulate_logs("debug")
    .add_modifier( add_thread_id )
#endif
    ;
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
  size_t          capacity()   const    { return _buf.size(); }
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

  // \todo remove ASAP
  friend inline std::ostream & operator<< (std::ostream &, input_buffer const &);

  size_t flush()                // return free space
  {
    size_t const len( size() );
    size_t space( back_space() );
    if (front_gap() > std::max(len, space))
    {
      TRACE() << "force gap flushing: " << *this;
      flush_gap();
      space = back_space();
    }
    else if (space * 2u <= std::min(len, min_buf_size))
    {
      size_t const cap( std::max(min_buf_size, capacity() * 2u) );
      TRACE() << "reallocate to " << cap << " bytes: " << *this;
      realloc(cap);
      space = back_space();
    }
    BOOST_ASSERT(space);
    return space;
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

template <class Handler>
inline void tcp_driver( tcp_acceptor &  acc
                      , Handler         f = Handler()
                      , shared_socket   s = shared_socket()
                      )
{
  if (s) f.start(s);
  s.reset(new tcp_socket( acc.io_service() ));
  acc.async_accept(*s, boost::bind(&tcp_driver<Handler>, boost::ref(acc), f, s));
}

template <class Handler>        // bool (input_buffer &, size_t, output_buffer &)
struct io_driver
{
  struct context : private boost::noncopyable
  {
    shared_socket       insock, outsock;
    input_buffer        inbuf;
    output_buffer       outbuf;
    Handler             f;
  };

  typedef boost::shared_ptr<context> ctx_ptr;

  static void start( shared_socket const & sin
                   , shared_socket const & sout = shared_socket()
                   )
  {
    BOOST_ASSERT(sin);
    ctx_ptr ctx(new context);
    ctx->insock  = sin;
    ctx->outsock = sout ? sout : sin;
    start_read(ctx);
  }

  static void start_read(ctx_ptr ctx)
  {
    ctx->outbuf.reset();
    size_t const space( ctx->inbuf.flush() );
    BOOST_ASSERT(space);
    using boost::asio::placeholders::bytes_transferred;
    ctx->insock->async_read_some
      ( boost::asio::buffer(ctx->inbuf.end(), space)
      , boost::bind( &io_driver::new_input, ctx, bytes_transferred )
      );
  }

  static void new_input(ctx_ptr ctx, std::size_t i)
  {
    ctx->inbuf.append(i);
    bool const more_input( ctx->f(ctx->inbuf, i, ctx->outbuf) && i );
    scatter_vector const & outbuf( ctx->outbuf.commit() );
    if (more_input)
    {
      if (outbuf.empty())
        start_read(ctx);
      else
        boost::asio::async_write
          ( *ctx->outsock
          , outbuf
          , boost::bind( &io_driver::start_read, ctx )
          );
    }
    else
    {
      if (!outbuf.empty())
        boost::asio::async_write
          ( *ctx->outsock
          , outbuf
          , boost::bind( &io_driver::stop, ctx )
          );
    }
  }

  static void stop(ctx_ptr ctx)
  {
    ctx.reset();
  }
};

template <class Handler>        // size_t (byte_ptr, byte_ptr, size_t, output_buffer &)
struct stream_driver : public Handler
{
  typedef bool result_type;

  bool operator()(input_buffer & inbuf, size_t i, output_buffer & outbuf)
  {
    TRACE() << "stream handler entry: " << inbuf << ", new input = " << i;
    size_t k( Handler::operator()(inbuf.begin(), inbuf.end(), i, outbuf) );
    inbuf.consume(k);
    if (i)
      while (k && !inbuf.empty())
      {
        k = Handler::operator()(inbuf.begin(), inbuf.end(), std::min(i, inbuf.size()), outbuf);
        inbuf.consume(k);
      }
    TRACE() << "stream handler exit: " << inbuf << ", " << outbuf;
    return i;
  }
};

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

  std::size_t operator() (byte_const_ptr begin, byte_const_ptr end, std::size_t i, output_buffer & outbuf) const
  {
    BOOST_ASSERT(begin <= end);
    size_t const size( static_cast<std::size_t>(end - begin) );
    BOOST_ASSERT(i <= size);
    if (size)
    {
      i = !i ? size : static_cast<std::size_t>(rand()) % size;
      outbuf.append(begin, begin + i);
    }
    return i;
  }
};

// ----- test program ---------------------------------------------------------

#include <boost/program_options.hpp>

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

#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
#  ifndef LOG_PERROR
#    define LOG_PERROR 0
#  endif
  openlog(PACKAGE_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_DAEMON);
#endif
  init_logging();
  the_io_service.reset(new io_service);
  signal(SIGTERM, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGINT,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGHUP,  reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGQUIT, reinterpret_cast<sighandler_t>(&stop_service));
  signal(SIGPIPE, SIG_IGN);

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

  if (vm.count("help"))     { cout << opts << endl;                  return 0; }
  if (vm.count("version"))  { cout << PACKAGE_VERSION << endl;       return 0; }
  if (n_threads == 0u)      { cout << "invalid option: -j0" << endl; return 1; }
  bool const detach( !vm.count("no-detach") );

  // Configure the listeners.

  tcp_acceptor port2525(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2525));
  tcp_driver< io_driver<tracer> >(port2525);

  tcp_acceptor port2526(*the_io_service, tcp_endpoint(boost::asio::ip::tcp::v4(), 2526));
  tcp_driver< io_driver< stream_driver< tracer > > >(port2526);

  // Run the server.

  INFO() << "version " << PACKAGE_VERSION
         << " running " << n_threads << " threads "
         << (detach ? "as daemon" : "using current tty");

  boost::thread_group pool;
  while(--n_threads)
    pool.create_thread( boost::bind(&io_service::run, the_io_service.get()) );
  the_io_service->run();

  INFO() << "shutting down";

  pool.join_all();
  the_io_service.reset();

  return 0;
}
