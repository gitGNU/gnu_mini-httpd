/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
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

#ifndef TIME_TO_RFCDATE_HH_INCLUDED
#define TIME_TO_RFCDATE_HH_INCLUDED

#include <stdexcept>
#include <string>
#include <ctime>

inline std::string time_to_rfcdate(time_t t)
{
  char buffer[64];
  size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
  if (len == 0 || len >= sizeof(buffer))
    throw std::length_error("strftime() failed because an internal buffer is too small!");
  return buffer;
}

inline std::string time_to_logdate(time_t t)
{
  char buffer[64];
  size_t len = strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", localtime(&t));
  if (len == 0 || len >= sizeof(buffer))
    throw std::length_error("strftime() failed because the internal buffer is too small");
  return buffer;
}

#endif // TIME_TO_RFCDATE_HH_INCLUDED
