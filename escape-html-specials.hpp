/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
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
