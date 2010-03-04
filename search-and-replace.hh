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

#ifndef SEARCH_AND_REPLACE_HH
#define SEARCH_AND_REPLACE_HH

#include <stdexcept>
#include <string>

inline std::string search_and_replace(const std::string& input,
                                      const std::string& search,
                                      const std::string& replace,
                                      const bool case_sensitive = false)
    {
    if (search.empty())
        throw std::invalid_argument("search_and_replace() called with empty search string");

    std::string tmp;
    for (const char* p = input.data(); p != input.data() + input.size(); )
        {
        int rc;
        if (case_sensitive)
            rc = strncmp(p, search.data(), search.size());
        else
            rc = strncasecmp(p, search.data(), search.size());
        if (rc == 0)
            {
            tmp.append(replace);
            p += search.size();
            }
        else
            {
            tmp.append(p, 1);
            ++p;
            }
        }
    return tmp;
    }

#endif // !defined(SEARCH_AND_REPLACE_HH)
