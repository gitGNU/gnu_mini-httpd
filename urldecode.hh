/*
 * Copyright (c) 2000 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <string>
#include <stdexcept>

inline void urldecode(std::string& url)
    {
    for(std::string::iterator i = url.begin(); i != url.end(); ++i)
        {
        if (*i == '+')
            *i = ' ';
        else if(*i == '%')
            {
            unsigned char c;
            std::string::size_type start = i - url.begin();

            if (++i == url.end())
                throw std::runtime_error("Invalid encoded character in url!");

            if (*i >= '0' && *i <= '9')
                c = *i - '0';
            else if (*i >= 'a' && *i <= 'f')
                c = *i - 'a' + 10;
            else if (*i >= 'A' && *i <= 'F')
                c = *i - 'A' + 10;
            else
                throw std::runtime_error("Invalid encoded character in url!");
            c = c << 8;

            if (++i == url.end())
                throw std::runtime_error("Invalid encoded character in url!");

            if (*i >= '0' && *i <= '9')
                c += *i - '0';
            else if (*i >= 'a' && *i <= 'f')
                c += *i - 'a' + 10;
            else if (*i >= 'A' && *i <= 'F')
                c += *i - 'A' + 10;
            else
                throw std::runtime_error("Invalid encoded character in url!");

            url.replace(start, 3, 1, c);
            }
        }
    }
