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

#ifndef MINI_HTTPD_SYSTEM_ERROR_HPP_INCLUDED
#define MINI_HTTPD_SYSTEM_ERROR_HPP_INCLUDED

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

#endif // MINI_HTTPD_SYSTEM_ERROR_HPP_INCLUDED
