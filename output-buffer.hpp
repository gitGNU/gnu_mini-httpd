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

#ifndef MINI_HTTPD_OUTPUT_BUFFER_HPP_INCLUDED
#define MINI_HTTPD_OUTPUT_BUFFER_HPP_INCLUDED

#include "io.hpp"

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

/**
 *  \internal Commit the buffer address.
 */
struct output_buffer::fix_base
{
  byte_const_ptr const begin;
  byte_const_ptr const end;
  ptrdiff_t const      fix;

  fix_base(output_buffer & iob) : begin( iob._base )
                                , end( begin + iob._buf.size() )
                                , fix( &iob._buf[0] - begin )
  {
    iob._base = &iob._buf[0];
  }

  void operator() (boost::asio::const_buffer & iov) const
  {
    byte_const_ptr const  b( boost::asio::buffer_cast<byte_const_ptr>(iov) );
    size_t const          len( boost::asio::buffer_size(iov) );
    if (begin <= b && b + len <= end)
      iov = boost::asio::const_buffer(b + fix, len);
  }
};

/**
 *  \brief todo
 */
inline output_buffer::output_buffer() : _base(0)
{
}

/**
 *  \brief todo
 */
inline bool output_buffer::empty() const
{
  BOOST_ASSERT( !_iovec.empty() || _buf.empty() );
  return _iovec.empty();
}

/**
 *  \brief todo
 *  \todo use paged_advance()
 */
inline void output_buffer::consume(size_t n)
{
  for (scatter_vector::iterator i( _iovec.begin() ); n; /**/)
  {
    BOOST_ASSERT(i != _iovec.end());
    size_t const len( boost::asio::buffer_size(*i) );
    BOOST_ASSERT(len);
    if (len <= n)
    {
      n -= len;
      ++i;
      if (!n)
        _iovec.erase(_iovec.begin(), i);
    }
    else
    {
      byte_const_ptr const b( boost::asio::buffer_cast<byte_const_ptr>(*i) );
      *i = io_vector(b + n, len - n);
      _iovec.erase(_iovec.begin(), i);
      break;
    }
  }
  if (_iovec.empty()) flush();
}

/**
 *  \brief todo
 */
inline void output_buffer::flush()
{
  _iovec.resize(0u);
  _buf.resize(0u);
}

/**
 *  \brief todo
 */
inline scatter_vector const & output_buffer::commit()
{
  if (!_buf.empty())
    std::for_each(_iovec.begin(), _iovec.end(), fix_base(*this));
  return _iovec;
}

/**
 *  \brief todo
 */
inline void output_buffer::append(byte_const_ptr b, byte_const_ptr e)
{
  BOOST_ASSERT(b < e);
  _iovec.push_back(io_vector(b, e - b));
}

/**
 *  \brief todo
 */
inline void output_buffer::append(void const * base, size_t len)
{
  BOOST_ASSERT(len);
  _iovec.push_back(io_vector(base, len));
}

/**
 *  \brief todo
 */
template <class Iter>
inline void output_buffer::push_back(Iter b, Iter e)
{
  size_t const old_len( _buf.size() );
  _buf.insert(_buf.end(), b, e);
  size_t const new_len( _buf.size() );
  BOOST_ASSERT(old_len <= new_len);
  if (old_len != new_len)
    append(_base + old_len, new_len - old_len);
}

/**
 *  \brief todo
 */
inline std::ostream & operator<< (std::ostream & os, output_buffer const & b)
{
  return os << "iovecs = "     << b._iovec.size()
            << ", buffered = " << b._buf.size();
}

#endif // MINI_HTTPD_OUTPUT_BUFFER_HPP_INCLUDED
