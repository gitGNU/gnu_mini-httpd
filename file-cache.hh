/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __FILE_CACHE_HH__
#define __FILE_CACHE_HH__

#include <string>
#include <map>
#include <unistd.h>
#include "refcount_auto_ptr/refcount_auto_ptr.hh"

class FileCache
    {
  public:
    explicit FileCache();
    ~FileCache();

    typedef std::pair<refcount_auto_ptr<char>,size_t> buffer_object_t;
    buffer_object_t get_file(const std::string& name);

  private:
    FileCache(const FileCache&);
    FileCache& operator= (const FileCache&);

    buffer_object_t load_file(const std::string& name);

    struct fdsentry
	{
	fdsentry(int fd_arg) : fd(fd_arg) { }
	~fdsentry() { if (fd >= 0) close(fd); }
	operator int () const throw() { return fd; }
	int fd;
	};

    struct object
	{
	explicit object()
		: data(0), data_len(0), accesses(0)
	    {
	    }
	explicit object(refcount_auto_ptr<char> buf, size_t len)
		: data(buf), data_len(len), accesses(0)
	    {
	    }
	refcount_auto_ptr<char> data;
	size_t data_len;
	mutable size_t accesses;
	};
    typedef std::map<std::string,object> objectmap_t;
    objectmap_t objects;
    size_t total_size;
    };

extern FileCache* cache;

#endif //!defined(__FILE_CACHE_HH__)
