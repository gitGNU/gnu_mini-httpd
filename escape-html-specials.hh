/*
 * Copyright (c) 2002 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef ESCAPE_HTML_SPECIALS_HH
#define ESCAPE_HTML_SPECIALS_HH

#include <string>

inline std::string escape_html_specials(const std::string& input)
    {
    std::string tmp = input;
    for (std::string::size_type pos = 0; pos <= tmp.size(); ++pos)
        {
        switch(tmp[pos])
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

#endif
