/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "system-error/system-error.hh"
#include "log.hh"
#include "file-cache.hh"
using namespace std;

namespace
    {
    struct fdsentry
	{
	fdsentry(int fd_arg) : fd(fd_arg) { }
	~fdsentry() { if (fd >= 0) close(fd); }
	operator int () const throw() { return fd; }
	int fd;
	};
    }

FileCache::FileCache() : total_size(0)
    {
    }

FileCache::~FileCache()
    {
    }

FileCache::cached_object::cached_object()
	: accesses(0)
    {
    }

FileCache::cached_object::cached_object(refcount_auto_ptr<char> buf, size_t len)
	: accesses(0)
    {
    data = buf;
    data_len = len;
    }

const FileCache::file_object FileCache::get_file(const char* name)
    {
    TRACE();
    objectmap_t::const_iterator i = objects.find(name);
    if (i != objects.end())
	{
	++(i->second.accesses);
	debug("CACHE HIT: File %s has been hit %d times.", name, i->second.accesses);
	return i->second;
	}

    debug("CACHE MISS for file %s.", name);
    return (objects[name] = load_file(name));
    }

FileCache::cached_object FileCache::load_file(const char* name)
    {
    TRACE();
    fdsentry fd = open(name, O_RDONLY, 0);
    if (fd < 0)
	throw system_error(string("Can't open file ") + name);

    off_t file_length = lseek(fd, 0, SEEK_END);
    if (file_length == -1)
	throw system_error(string("seek() failed in file ") + name);
    if (lseek(fd, 0, SEEK_SET) == -1)
	throw system_error(string("seek() failed in file ") + name);

    refcount_auto_ptr<char> buf = new char[file_length];

    for (off_t read_length = 0; read_length < file_length; )
	{
	ssize_t rc = read(fd, buf.get() + read_length, file_length - read_length);
	if (rc <= 0)
	    throw system_error(string("Reading file ") + name + " failed");
	else
	    read_length += rc;
	}

    return cached_object(buf, file_length);
    }
