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
    for(std::string::iterator i = url.begin(); i != url.end(); ++i)
        {
        if (*i == '+')
            *i = ' ';
        else if(*i == '%')
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
