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

#ifndef MINI_HTTPD_IO_HPP_INCLUDED
#  error "do not include this file directly -- include io.hpp"
#endif

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
  start_read(ctx);
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::start_read(ctx_ptr ctx)
{
  ctx->outbuf.reset();
  size_t const space( ctx->inbuf.flush() );
  BOOST_ASSERT(space);
  using boost::asio::placeholders::bytes_transferred;
  ctx->insock->async_read_some
    ( boost::asio::buffer(ctx->inbuf.end(), space)
    , boost::bind( &io_driver<Handler>::new_input, ctx, bytes_transferred )
    );
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::new_input(ctx_ptr ctx, std::size_t i)
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
        , boost::bind( &io_driver<Handler>::start_read, ctx )
        );
  }
  else
  {
    if (!outbuf.empty())
      boost::asio::async_write
        ( *ctx->outsock
        , outbuf
        , boost::bind( &io_driver<Handler>::stop, ctx )
        );
  }
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::stop(ctx_ptr ctx)
{
  ctx.reset();
}

#endif // MINI_HTTPD_IO_DRIVER_HPP_INCLUDED
