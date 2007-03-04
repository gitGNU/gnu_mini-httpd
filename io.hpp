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
 *
 *  mini-httpd separates all I/O code from the actual HTTP state machine. This
 *  module provides a fairly high-level I/O driver that can run arbitrary
 *  handler functions of type
 *
 *    <pre>  bool (input_buffer &, size_t, output_buffer &)</pre>
 *
 *  as an asynchronous TCP service. I/O is driven by input events and the
 *  driver alternates between a "read" and "write" state until aborted. Every
 *  time new data arrives on the socket, the driver triggers the callback with
 *  the input buffer and passes the number of newly appended bytes as a
 *  parameter. The value 0u signifies that the input stream has ended and is to
 *  be interpreted as a shut-down signal, saying "write all remaining output
 *  now". The handler function can also abort the I/O driver prior to EOF by
 *  returning false. If the handler function returns true, the driver goes into
 *  "write" state until the entire contents of the \c output_buffer has been
 *  written before going back into "read" state.
 *
 *  An \c input_buffer is a continous memory buffer that -- from the handler
 *  function's perspective -- can be interpreted as a window into the input
 *  stream. The handler function can consume data from the front of buffer to
 *  make space for further input. It can also choose not to consume anything
 *  and to defer processing until more input has arrived. In that case, the I/O
 *  driver enlarges the buffer dynamically to allow further reading if
 *  necessary. A handler function that never consumes input until EOF
 *  essentially slurps the entire stream into one continous memory buffer
 *  (which might involve fairly expensive copying).
 *
 *  The \c output_buffer is a scatter I/O vector of continous memory ranges.
 *  Data can be written into this buffer either statically or dynamically. To
 *  write a string "statically" simply means to append it's begin and end
 *  address to the output buffer. Consequently, the existance of the object
 *  that owns this memory must be guaranteed until the time the output buffer
 *  has been written (which is well beyond the handler function's stack frame).
 *  In case of a buffered write, the output buffer takes ownership of the data
 *  being by copying, which is less efficient but very convenient.
 *
 *  As a consequence of the driver's mode of operation, the contents of the \c
 *  input_buffer is always guaranteed to exist for the life-time of a write
 *  state. No input takes place until the write is done, so \c input_buffer
 *  will not change until then. Input data can always be re-used for output
 *  without copying.
 *
 *  In terms of performance, it is in the handler's best interest to consume
 *  the buffer eagerly. To encourage that, an iteration strategy termed \c
 *  stream_driver exists as an adaptor template. A stream driver can run a
 *  handler function of this type:
 *
 *    <pre>  size_t (byte_const_ptr, byte_const_ptr, size_t i, output_buffer &)</pre>
 *
 *  A stream handler can be thought of as a lexer or a tokenizer. As a matter
 *  of fact, its signature is very similar to the one of \c
 *  boost::spirit::parse(). The integer value a stream handler returns
 *  signifies how many bytes it has consumed from the front of the input
 *  window. Returning 0u signifies blocking, and will result in the handler to
 *  be called again when more input has been read.
 */

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/assert.hpp>
#include <vector>
#include <cstring>              // std::memmove()
#include <cstdlib>              // std::size_t, std::ptrdiff_t
#include "logging.hpp"

// ----- Core Types -----------------------------------------------------------

using std::size_t;
using std::ptrdiff_t;

/// \brief Magic constant lower limit for I/O buffer sizes.
static size_t const min_buf_size( 1024u );

/// \brief Bytes may be signed or unsigned, we don't know.
typedef char                                    byte;

/// \brief The standard iterator for continous memory.
typedef byte *                                  byte_ptr;

/// \brief The standard iterator for continous, immutable memory.
typedef byte const *                            byte_const_ptr;

/// \brief The standard allocator for I/O buffers.
typedef std::allocator<byte>                    byte_allocator;

/**
 *  \brief A re-sizable buffer suitable for system I/O.
 *
 *  The original ISO standard did not guarantee that \c std::vector
 *  would use continous memory internally, but this point was revised in
 *  a technical report later.
 */
typedef std::vector<byte, byte_allocator>       io_vector;

/**
 *  \brief A range is a pair of iterators.
 *
 *  Ranges are cheap to copy, they do not own the memory they refer to.
 *  At some point future, it might be nice to change \c byte_range to
 *  the system's native \c iovec type, so that it can be used for
 *  scatter I/O without copying, but as of now the Boost.Asio library
 *  wouldn't be able to take advantage of this fact anyway.
 */
typedef boost::iterator_range<byte_ptr>         byte_range;

/**
 *  \brief The native scatter I/O vector type.
 *
 *  \todo Well, this is not really the native type. Boost.Asio does nave a
 *        typedef for the native type, but it considers it an implementation
 *        detail, so apparently users are forced to have their buffers copied
 *        to adapt them to the real type before they are written. It doesn't
 *        make any sense. This subject needs further investigation. Asking on
 *        the mailing list made no difference so far.
 */
typedef boost::asio::const_buffer               scatter_buffer;

/**
 *  \brief A native vector of scatter I/O vectors.
 *  \todo  Some day, maybe.
 */
typedef std::vector<scatter_buffer>             scatter_vector;

// ----- Input ----------------------------------------------------------------

/**
 *  \brief A dynamically re-sizable stream buffer for input.
 */
class input_buffer : public byte_range
{
  io_vector _buf;

  input_buffer(input_buffer const &);
  input_buffer & operator= (input_buffer const &);

public:
  explicit input_buffer(size_t cap = 0u);

  byte_ptr        buf_begin();
  byte_const_ptr  buf_begin()   const;
  byte_ptr        buf_end();
  byte_const_ptr  buf_end()     const;

  size_t capacity()   const;
  size_t front_gap()  const;
  size_t back_space() const;

  void   append(size_t i);
  void   consume(size_t i);

  void   reset();
  void   reset(byte_ptr b, byte_ptr e);
  size_t flush();

  size_t flush_gap();
  void   realloc(size_t n);
};

#include "io-input-buffer.hpp"

// ----- Output ---------------------------------------------------------------

/**
 *  \brief A scatter I/O vector for output.
 */
class output_buffer : private boost::noncopyable
{
  scatter_vector        _iovec;
  io_vector             _buf;

  struct fix_base;
  friend std::ostream & operator<< (std::ostream &, output_buffer const &);

public:
  output_buffer() { }

  void reset();

  scatter_vector const & commit();

  void push_back(scatter_buffer const & iov);

  template <class Iter> void push_back(Iter b, Iter e);
};

#include "io-output-buffer.hpp"

// ----- I/O Driver -----------------------------------------------------------

typedef boost::asio::ip::tcp::socket            tcp_socket;
typedef boost::shared_ptr<tcp_socket>           shared_socket;

/**
 *  \brief Iterate a given callback function over the input stream.
 */
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


/**
 *  \brief Iterate a tokenizer function over the input stream.
 */
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

typedef boost::asio::ip::tcp::acceptor tcp_acceptor;

/**
 *  \brief Accept TCP connections using a callback function.
 */
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

#endif // MINI_HTTPD_IO_HPP_INCLUDED
