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

#ifndef SYSTEM_ERROR_HH_INCLUDED
#define SYSTEM_ERROR_HH_INCLUDED

#include <stdexcept>
#include <cerrno>
#include <string>
#include <cstring>

class system_error : public std::runtime_error
{
public:
  system_error() : runtime_error(str())
  {
  }

  explicit system_error(std::string const & msg)
      : runtime_error(msg + ": " + str())
  {
  }

private:
  static std::string str()
  {
    using namespace std;
    return string(strerror(errno));
  }
};

#endif // SYSTEM_ERROR_HH_INCLUDED
