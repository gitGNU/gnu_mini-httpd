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

#ifndef MINI_HTTPD_IO_HPP_INCLUDED
#define MINI_HTTPD_IO_HPP_INCLUDED

/**
 *  \file  io.hpp
 *  \brief High-level I/O driver interface.
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <vector>
#include <cstdlib>              // std::size_t, std::ptrdiff_t
#include <cstring>              // std::memmove()
#include "logging.hpp"

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
  void push_back(scatter_buffer const & iov) { _iovec.push_back(iov); }

  template <class Iter>
  void push_back(Iter b, Iter e)
  {
    size_t const old_len( _buf.size() );
    _buf.insert(_buf.end(), b, e);
    size_t const new_len( _buf.size() );
    BOOST_ASSERT(old_len <= new_len);
    if (old_len != new_len)
      push_back(scatter_buffer(byte_const_ptr(0) + old_len, new_len - old_len));
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

#endif // MINI_HTTPD_IO_HPP_INCLUDED
