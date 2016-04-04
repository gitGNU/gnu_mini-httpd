/*
 * Copyright (c) 2010-2016 by Peter Simons <simons@cryp.to>.
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

#ifndef SYSTEM_ERROR_HH_INCLUDED
#define SYSTEM_ERROR_HH_INCLUDED

#include <boost/system/system_error.hpp>

struct system_error : boost::system::system_error
{
  system_error()
  : boost::system::system_error(errno, boost::system::system_category())
  {
  }

  explicit system_error(std::string const & msg)
  : boost::system::system_error(errno, boost::system::system_category(), msg)
  {
  }
};

#endif // SYSTEM_ERROR_HH_INCLUDED
