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

#ifndef ESCAPE_HTML_SPECIALS_HH_INCLUDED
#define ESCAPE_HTML_SPECIALS_HH_INCLUDED

#include <string>

inline std::string escape_html_specials(const std::string& input)
{
  std::string tmp = input;
  for (std::string::size_type pos = 0; pos <= tmp.size(); ++pos)
  {
    switch (tmp[pos])
    {
      case '<':
        tmp.replace(pos, 1, "&lt;");
        pos += 3;
        break;
      case '>':
        tmp.replace(pos, 1, "&gt;");
        pos += 3;
        break;
      case '&':
        tmp.replace(pos, 1, "&amp;");
        pos += 4;
        break;
    }
  }
  return tmp;
}

#endif // ESCAPE_HTML_SPECIALS_HH_INCLUDED
