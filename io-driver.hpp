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
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include "logging.hpp"

/**
 *  \brief todo
 */
template <class Handler>
class io_driver<Handler>::context : public async_streambuf
{
  input_buffer        inbuf;
  output_buffer       outbuf;

public:
  shared_socket       insock, outsock;
  Handler             f;

  context() { inbuf.realloc(1024u); }

  byte_range get_input_buffer()
  {
    return byte_range(inbuf.end(), inbuf.buf_end());
  }
  scatter_vector const & get_output_buffer()
  {
    return outbuf.commit();
  }
  void append_input(size_t i)
  {
    inbuf.append(i);
    f(inbuf, outbuf, i);
  }
  void drop_output(size_t /* driver uses "blocking" writes */)
  {
    outbuf.flush();
    f(inbuf, outbuf, true);
    if (outbuf.empty())
      inbuf.flush();
  }
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
  run(ctx, true);
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::run(ctx_ptr ctx, bool more_input)
{
  scatter_vector const & outbuf( ctx->get_output_buffer() );
  if (outbuf.empty())           // no output data available
  {
    if (more_input)             // read more input
    {
      byte_range inbuf( ctx->get_input_buffer() );
      if (!inbuf.empty()) // !inbuf.empty()
      {
        using boost::asio::placeholders::bytes_transferred;
        ctx->insock->async_read_some
          ( boost::asio::buffer(inbuf.begin(), inbuf.size())
          , boost::bind( &io_driver<Handler>::handle_read, ctx, bytes_transferred )
          );
      }
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
inline void io_driver<Handler>::handle_read(ctx_ptr ctx, size_t i)
{
  ctx->append_input(i);
  run(ctx, i);
}

/**
 *  \brief todo
 */
template <class Handler>
inline void io_driver<Handler>::handle_write(ctx_ptr ctx)
{
  ctx->drop_output(12u); /// \todo Use write_some?
  run(ctx, true);
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
