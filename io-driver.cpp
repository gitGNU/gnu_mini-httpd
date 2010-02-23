/*
 * Copyright (c) 2010 by Peter Simons <simons@cryp.to>.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
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

  void handle_write(ctx_ptr ctx, size_t i)
  {
    ctx->f->drop_output(i);
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
        if (!inbuf.empty())
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
    {
      if (more_input)
        ctx->outsock->async_write_some
          ( outbuf
          , boost::bind( &handle_write
                       , ctx
                       , boost::asio::placeholders::bytes_transferred
                       )
          );
      else
        boost::asio::async_write(*ctx->outsock, outbuf, boost::bind(&stop, ctx));
    }
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
