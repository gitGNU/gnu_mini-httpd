/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system-error.hh>
#include <log.hh>
#include "file-cache.hh"
using namespace std;

FileCache::FileCache() : total_size(0)
    {
    }

FileCache::~FileCache()
    {
    }

FileCache::buffer_object_t FileCache::get_file(const string& name)
    {
    TRACE();
    objectmap_t::const_iterator i = objects.find(name);
    if (i != objects.end())
	{
	++(i->second.accesses);
	debug("CACHE HIT: File %s has been hit %d times.", name.c_str(), i->second.accesses);
	return buffer_object_t(i->second.data, i->second.data_len);
	}
    debug("CACHE MISS for file %s.", name.c_str());
    buffer_object_t res = load_file(name);
    objects[name] = object(res.first, res.second);
    debug("File %s has been inserted into the cache. It's %d bytes long.", name.c_str(), res.second);
    return res;
    }

FileCache::buffer_object_t FileCache::load_file(const string& name)
    {
    TRACE();
    fdsentry fd = open(name.c_str(), O_RDONLY, 0);
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
    return buffer_object_t(buf,file_length);
    }
