/*
 * Copyright (C) 2001 by Peter Simons <simons@computer.org>.
 * All rights reserved.
 */

#ifndef __SYSTEM_ERROR_HH__
#define __SYSTEM_ERROR_HH__

#include <stdexcept>
#include <cerrno>
#include <string>

struct system_error : public runtime_error
    {
    explicit system_error(const string& msg)
	    : runtime_error(msg + ": " + strerror(errno))
	{
	}
    };

#endif
