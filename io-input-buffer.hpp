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

#ifndef MINI_HTTPD_IO_INPUT_BUFFER_HPP_INCLUDED
#define MINI_HTTPD_IO_INPUT_BUFFER_HPP_INCLUDED

#ifndef MINI_HTTPD_IO_HPP_INCLUDED
#  error "do not include this file directly -- include io.hpp"
#endif

/**
 *  \brief Magic constant lower limit for I/O buffer sizes.
 */
size_t const input_buffer::min_buf_size( 1024u );

/**
 *  \brief Debug-display an input_buffer's state.
 */
inline std::ostream & operator<< (std::ostream & os, input_buffer const & b)
{
  return os << "gap = "     << b.front_gap()
            << ", size = "  << b.size()
            << ", space = " << b.back_space();
}

/**
 *  \brief Construct an empty input_buffer with the given capacity.
 */
inline input_buffer::input_buffer(size_t cap) : _buf(cap)
{
  reset();
}

/**
 *  \brief Begin pointer of the underlying memory buffer.
 */
inline byte_ptr input_buffer::buf_begin()
{
  return &_buf[0];
}

/**
 *  \brief Begin pointer of the underlying, immutable memory buffer.
 */
inline byte_const_ptr input_buffer::buf_begin() const
{
  return &_buf[0];
}

/**
 *  \brief End pointer of the underlying memory buffer.
 */
inline byte_ptr input_buffer::buf_end()
{
  return &_buf[_buf.size()];
}

/**
 *  \brief End pointer of the underlying, immutable memory buffer.
 */
inline byte_const_ptr input_buffer::buf_end() const
{
  return &_buf[_buf.size()];
}

/**
 *  \brief Size of the the underlying memory buffer.
 */
inline size_t input_buffer::capacity() const
{
  return _buf.size();
}

/**
 *  \brief Unused space at buffer front.
 */
inline size_t input_buffer::front_gap() const
{
  return begin() - buf_begin();
}

/**
 *  \brief Unused space at buffer end.
 */
inline size_t input_buffer::back_space() const
{
  return buf_end() - end();
}

/**
 *  \brief Resize data buffer end to contain \p i additional bytes.
 */
inline void input_buffer::append(size_t i)
{
  reset(begin(), end() + i);
}

/**
 *  \brief Drop the first \p i bytes from the data buffer.
 */
inline void input_buffer::consume(size_t i)
{
  reset(begin() + i, end());
}

/**
 *  \brief Reset buffer into empty state.
 */
inline void input_buffer::reset()
{
  byte_range::operator= (byte_range(buf_begin(), buf_begin()));
}

/**
 *  \brief Reset the data buffer.
 */
inline void input_buffer::reset(byte_ptr b, byte_ptr e)
{
  BOOST_ASSERT(buf_begin() <= b && b <= e && e <= buf_end());
  if (b == e) reset();
  else        byte_range::operator= (byte_range(b, e));
}

/**
 *  \brief Reset the front gap.
 */
inline size_t input_buffer::flush_gap()
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

/**
 *  \brief Enlarge buffer.
 */
inline void input_buffer::realloc(size_t n)
{
  size_t const len( size() );
  BOOST_ASSERT(len < n);
  byte_buffer new_buf(n);
  std::memmove(&new_buf[0], begin(), len);
  _buf.swap(new_buf);
  reset(buf_begin(), buf_begin() + len);
}

/**
 *  \brief make space
 */
inline size_t input_buffer::flush()
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

#endif // MINI_HTTPD_IO_INPUT_BUFFER_HPP_INCLUDED
