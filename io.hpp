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

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <boost/compatibility/cpp_c_headers/cstddef> // std::size_t, std::ptrdiff_t

// ----- Core Types -----------------------------------------------------------

using std::size_t;
using std::ptrdiff_t;

/**
 *  \brief Bytes may be signed or unsigned, we don't know.
 *
 *  All I/O deals with octets; none of the functions depend on this type to be
 *  signed or unsigned. Neither is it important whether that octet represents a
 *  character encoded in latin-1 or utf-8. These matters are firmly the problem
 *  of the upper layers. For purposes of I/O code, the only thing that matters
 *  is <code>sizeof(byte_t) == 1</code>.
 */
typedef char                                    byte_t;

/**
 *  \brief Standard iterator over continuous memory.
 */
typedef byte_t *                                byte_ptr;

/**
 *  \brief Standard iterator for immutable continous memory.
 */
typedef byte_t const *                          byte_const_ptr;

/**
 *  \brief Byte ranges are tuples of \c [begin,end) iterators.
 *
 *  A \c byte_range does not own the memory it references, these objects are
 *  cheap to copy. The object is implicitly convertible to \c bool: true
 *  signifies that the range is non-empty, false the opposite. The \c
 *  byte_range class also offers a full STL-container-like interface into the
 *  memory, using raw pointers as iterators.
 *
 *  \todo It would be nice if \c byte_range would have the same memory layout
 *        as the native I/O vector type. See also \c io_vector.
 */
typedef boost::iterator_range<byte_ptr>         byte_range;

/**
 *  \brief Standard allocator for I/O buffers.
 */
typedef std::allocator<byte_t>                  byte_allocator;

/**
 *  \brief A re-sizable buffer suitable for system I/O.
 *
 *  A \c byte_buffer does own the memory it represents, copying such an object
 *  is an expensive operation.
 *
 *  \note The original ISO C++ standard did not require \c std::vector to use
 *        continuous memory. This point was later revised in a Technical Report
 *        Unfortunately, a URL to that document is hard to find.
 */
typedef std::vector<byte_t, byte_allocator>     byte_buffer;

/**
 *  \brief A byte range type suitable for scatter I/O.
 *
 *  This type is essentially a \c byte_range without the STL-like interface. It
 *  does have the nice property, however, that arrays of this type can be used
 *  for I/O operations that involve non-continuous memory. The Unix man pages
 *  \c readv(2) and \c writev(2) provide further details.
 *
 *  \todo This is not really the native type. Boost.Asio does have a typedef
 *        for the native io_vector type, but it considers that a private
 *        implementation detail. Which is unfortunate, because without that
 *        type it is not possible to create a native scatter I/O array. Which
 *        means that the scatter array we pass for reading or writing has to
 *        be copied internally before it can be used. This subject needs
 *        further investigation.
 */
typedef boost::asio::const_buffer               io_vector;

/**
 *  \brief A dynamically re-sizable scatter I/O array.
 *
 *  Scatter I/O adds a layer of indirection to the way buffers are represented:
 *
 *  <pre>
 *    io_vector(0x00f00ba7, 24) -> "this is the first line\n"
 *    io_vector(0x00ba7f00, 21) -> "I am the second line\n"
 *    ...</pre>
 *
 *  Using a \c scatter_vector, it is possible to chain non-continuous memory
 *  chunks together and write them (or read into them) using a single system
 *  call. This approach is advantageous for applications that construct a write
 *  buffer from parts of the input buffer and parts from somewhere else.
 */
typedef std::vector<io_vector>                  scatter_vector;

/**
 *  \brief A scatter I/O vector for output.
 */
class output_buffer : private boost::noncopyable
{
  scatter_vector        _iovec;
  byte_buffer           _buf;
  byte_const_ptr        _base;

  struct fix_base;
  friend std::ostream & operator<< (std::ostream &, output_buffer const &);

public:
  output_buffer();

  bool empty() const;
  void consume(size_t);
  void append(byte_const_ptr, byte_const_ptr);
  void append(void const *, size_t);
  void flush();

  template <class Iter>
  void push_back(Iter b, Iter e);

  scatter_vector const & commit();
};

// ----- I/O Driver -----------------------------------------------------------

/**
 *  \brief Asynchronous stream buffer interface.
 */
struct async_streambuf : private boost::noncopyable
{
  typedef boost::shared_ptr<async_streambuf> pointer;

  virtual ~async_streambuf() { }

  /// \todo Can't use scatter_vector because of Asio limitations.
  virtual byte_range get_input_buffer() = 0;
  virtual scatter_vector const & get_output_buffer() = 0;

  virtual void append_input(size_t) = 0;
  virtual void drop_output(size_t) = 0;
};

/**
 *  \brief Asynchronous buffers are heap objects.
 */
typedef async_streambuf::pointer                stream_handler;

/**
 *  \brief todo
 */
typedef boost::asio::ip::tcp::socket            stream_socket;

/**
 *  \brief todo
 */
typedef boost::shared_ptr<stream_socket>        shared_socket;

/**
 *  \brief todo
 */
typedef boost::asio::ip::tcp::acceptor          stream_acceptor;

/**
 *  \brief todo
 */
typedef boost::shared_ptr<stream_acceptor>      shared_acceptor;

/**
 *  \brief Iterate a stream_handler over the input stream.
 */
void start_io( stream_handler const & f
             , shared_socket const &  sin
             , shared_socket const &  sout = shared_socket()
             );


#include <boost/bind.hpp>

/**
 *  \brief Accept TCP connections using a callback function.
 */
template <class socket_handler>
void start_tcp( stream_acceptor & acc
              , socket_handler    f   = socket_handler()
              , shared_socket     s   = shared_socket()
              )
{
  if (s) f(s);
  s.reset(new stream_socket( acc.io_service() ));
  acc.async_accept(*s, boost::bind(&start_tcp<socket_handler>, boost::ref(acc), f, s));
}

#endif // MINI_HTTPD_IO_HPP_INCLUDED
