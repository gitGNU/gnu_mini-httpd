/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <unistd.h>
#include "system-error/system-error.hh"
#include "request-handler.hh"
#include "config.hh"
using namespace std;

size_t RequestHandler::myread(int fd, void* buf, size_t size)
    {
    TRACE();
    ssize_t rc = read(fd, buf, size);
    if (rc < 0)
	{
	if (errno == EAGAIN || errno == EINTR)
	    return 0;
	else
	    throw system_error("read() failed with an error");
	}
    return rc;
    }

size_t RequestHandler::mywrite(int fd, const void* buf, size_t size)
    {
    TRACE();
    ssize_t rc = write(fd, buf, size);
    if (rc < 0)
	{
	if (errno == EAGAIN || errno == EINTR)
	    return 0;
	else
	    throw system_error("write() failed with an error");
	}
    return rc;
    }
