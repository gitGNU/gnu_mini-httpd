/*
 * Copyright (C) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#ifndef __SYSTEM_ERROR_HH__
#define __SYSTEM_ERROR_HH__

#include <stdexcept>
#include <cerrno>
#include <string>

struct system_error : public std::runtime_error
    {
    explicit system_error(const std::string& msg)
	    : std::runtime_error(msg + ": " + strerror(errno))
	{
	}
    };

#endif
