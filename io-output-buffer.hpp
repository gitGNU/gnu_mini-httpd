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

#ifndef MINI_HTTPD_IO_OUTPUT_BUFFER_HPP_INCLUDED
#define MINI_HTTPD_IO_OUTPUT_BUFFER_HPP_INCLUDED

#ifndef MINI_HTTPD_IO_HPP_INCLUDED
#  error "do not include this file directly -- include io.hpp"
#endif

/**
 *  \internal Commit the buffer address.
 */
struct output_buffer::fix_base
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

/**
 *  \brief todo
 */
inline void output_buffer::reset()
{
  _iovec.clear();
  _buf.clear();
}

/**
 *  \brief todo
 */
inline scatter_vector const & output_buffer::commit()
{
  std::for_each(_iovec.begin(), _iovec.end(), fix_base(*this));
  return _iovec;
}

/**
 *  \brief todo
 */
inline void output_buffer::push_back(io_vector const & iov)
{
  _iovec.push_back(iov);
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
    push_back(io_vector(byte_const_ptr(0) + old_len, new_len - old_len));
}

/**
 *  \brief todo
 */
inline std::ostream & operator<< (std::ostream & os, output_buffer const & b)
{
  return os << "iovecs = "     << b._iovec.size()
            << ", buffered = "   << b._buf.size();
}

#endif // MINI_HTTPD_IO_OUTPUT_BUFFER_HPP_INCLUDED
