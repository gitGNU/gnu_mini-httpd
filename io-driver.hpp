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

#ifndef MINI_HTTPD_IO_DRIVER_HPP_INCLUDED
#define MINI_HTTPD_IO_DRIVER_HPP_INCLUDED

#include "io-input-buffer.hpp"
#include "io-output-buffer.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include "logging.hpp"

/**
 *  \brief todo
 */
template <class Handler>
struct io_driver<Handler>::context : private boost::noncopyable
{
  shared_socket       insock, outsock;
  input_buffer        inbuf;
  output_buffer       outbuf;
  Handler             f;
};

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::start( shared_socket const & sin
                                     , shared_socket const & sout
                                     )
{
  BOOST_ASSERT(sin);
  ctx_ptr ctx(new context);
  ctx->insock  = sin;
  ctx->outsock = sout ? sout : sin;
  run(ctx, ctx->f(ctx->inbuf, ctx->outbuf, true));
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::run(ctx_ptr ctx, bool more_input)
{
  scatter_vector const & outbuf( ctx->outbuf.commit() );
  if (outbuf.empty())           // no output data available
  {
    if (more_input)             // read more input
    {
      size_t const space( ctx->inbuf.flush() );
      BOOST_ASSERT(space);
      using boost::asio::placeholders::bytes_transferred;
      ctx->insock->async_read_some
        ( boost::asio::buffer(ctx->inbuf.end(), space)
        , boost::bind( &io_driver<Handler>::handle_read, ctx, bytes_transferred )
        );
    }
  }
  else                          // perform async write
    boost::asio::async_write
      ( *ctx->outsock
      , outbuf
      , boost::bind( more_input ? &io_driver<Handler>::handle_write
                                : &io_driver<Handler>::stop
                   , ctx
                   )
      );
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::handle_read(ctx_ptr ctx, std::size_t i)
{
  ctx->inbuf.append(i);
  run(ctx, ctx->f(ctx->inbuf, ctx->outbuf, i != 0u) && i);
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::handle_write(ctx_ptr ctx)
{
  ctx->outbuf.flush();
  run(ctx, ctx->f(ctx->inbuf, ctx->outbuf, true));
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::stop(ctx_ptr ctx)
{
  ctx.reset();
}


/**
 *  \brief Iterate a tokenizer function over the input stream.
 */
template <class Handler>
inline bool stream_driver<Handler>::operator()(input_buffer & inbuf, size_t i, output_buffer & outbuf)
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

/**
 *  \brief Accept TCP connections using a callback function.
 */
template <class Handler>
inline void tcp_driver( stream_acceptor & acc
                      , Handler           f
                      , shared_socket     s
                      )
{
  if (s) f.start(s);
  s.reset(new stream_socket( acc.io_service() ));
  acc.async_accept(*s, boost::bind(&tcp_driver<Handler>, boost::ref(acc), f, s));
}

#endif // MINI_HTTPD_IO_DRIVER_HPP_INCLUDED
