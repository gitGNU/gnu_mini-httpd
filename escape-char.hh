/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef ESCAPE_CHAR_HH
#define ESCAPE_CHAR_HH

#include <string>

inline std::string escape_char(const std::string& buffer, char evil)
    {
    std::string tmp = buffer;
    for (std::string::size_type pos = 0; pos < tmp.size(); ++pos)
        {
        if (tmp[pos] == evil)
            {
            tmp.insert(pos, 1, '\\');
            ++pos;
            }
        }
    return tmp;
    }

#endif
