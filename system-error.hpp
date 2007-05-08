/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef SANITY_SYSTEM_ERROR_HH_INCLUDED
#define SANITY_SYSTEM_ERROR_HH_INCLUDED

#include <boost/system/system_error.hpp>
#include <cerrno>
#include <iosfwd>

struct system_error : public boost::system::system_error
{
  system_error()
    : boost::system::system_error(errno, boost::system::errno_ecat)
  {
  }

  explicit system_error(std::string const & msg)
    : boost::system::system_error(errno, boost::system::errno_ecat, msg)
  {
  }
};

inline std::ostream & operator<< (std::ostream & os, system_error const & err)
{
  return os << err.what();
}

#endif // SANITY_SYSTEM_ERROR_HH_INCLUDED
