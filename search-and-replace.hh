/*
 * Copyright (c) 2002 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
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
