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

#include "io.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include "logging.hpp"

namespace
{
  struct io_context : private boost::noncopyable
  {
    stream_handler      f;
    shared_socket       insock, outsock;
  };

  typedef boost::shared_ptr<io_context> ctx_ptr;

  void run(ctx_ptr ctx, bool more_input);

  void handle_read(ctx_ptr ctx, size_t i)
  {
    ctx->f->append_input(i);
    run(ctx, i);
  }

  void handle_write(ctx_ptr ctx)
  {
    ctx->f->drop_output(12u); /// \todo Use write_some?
    run(ctx, true);
  }

  void stop(ctx_ptr ctx)
  {
    ctx.reset();
  }

  void run(ctx_ptr ctx, bool more_input)
  {
    scatter_vector const & outbuf( ctx->f->get_output_buffer() );
    if (outbuf.empty())           // no output data available
    {
      if (more_input)             // read more input
      {
        byte_range inbuf( ctx->f->get_input_buffer() );
        if (!inbuf.empty()) // !inbuf.empty()
        {
          using boost::asio::placeholders::bytes_transferred;
          ctx->insock->async_read_some
            ( boost::asio::buffer(inbuf.begin(), inbuf.size())
            , boost::bind( &handle_read, ctx, bytes_transferred )
            );
        }
      }
    }
    else                          // perform async write
      boost::asio::async_write
        ( *ctx->outsock
        , outbuf
        , boost::bind( more_input ? &handle_write : &stop, ctx )
        );
  }
}

void start_io( stream_handler const & f
             , shared_socket const &  sin
             , shared_socket const &  sout
             )
{
  BOOST_ASSERT(f); BOOST_ASSERT(sin);
  ctx_ptr ctx(new io_context);
  ctx->insock  = sin;
  ctx->outsock = sout ? sout : sin;
  ctx->f       = f;
  run(ctx, true);
}
