/*
 * Copyright (c) 2001-2016 Peter Simons <simons@cryp.to>
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

#ifndef URLDECODE_HH_INCLUDED
#define URLDECODE_HH_INCLUDED

#include <string>
#include <stdexcept>

/*
   Throwing an exception in case the URL contains a syntax error may
   seem a bit harsh, but consider that this should never happen as all
   URL stuff we see went through the HTTP parser, who would not let
   the URL pass if it contained an error.

   Hopefully.
*/

inline std::string urldecode(const std::string& input)
{
  std::string url = input;
  for (std::string::iterator i = url.begin(); i != url.end(); ++i)
  {
    if (*i == '+')
      *i = ' ';
    else if (*i == '%')
    {
      unsigned char c;
      std::string::size_type start = i - url.begin();

      if (++i == url.end())
        throw std::runtime_error("Invalid encoded character in URL!");

      if (*i >= '0' && *i <= '9')
        c = *i - '0';
      else if (*i >= 'a' && *i <= 'f')
        c = *i - 'a' + 10;
      else if (*i >= 'A' && *i <= 'F')
        c = *i - 'A' + 10;
      else
        throw std::runtime_error("Invalid encoded character in URL!");
      c = c << 4;

      if (++i == url.end())
        throw std::runtime_error("Invalid encoded character in URL!");

      if (*i >= '0' && *i <= '9')
        c += *i - '0';
      else if (*i >= 'a' && *i <= 'f')
        c += *i - 'a' + 10;
      else if (*i >= 'A' && *i <= 'F')
        c += *i - 'A' + 10;
      else
        throw std::runtime_error("Invalid encoded character in URL!");

      url.replace(start, 3, 1, c);
    }
  }
  return url;
}

#endif // URLDECODE_HH_INCLUDED
