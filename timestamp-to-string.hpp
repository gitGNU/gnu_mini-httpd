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

#ifndef TIME_TO_RFCDATE_HH
#define TIME_TO_RFCDATE_HH

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

#endif
