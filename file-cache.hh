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

    struct file_object
	{
	refcount_auto_ptr<char> data;
	size_t data_len;
	};
    const file_object get_file(const char* name);

  private:
    FileCache(const FileCache&);
    FileCache& operator= (const FileCache&);

    struct cached_object : public file_object
	{
	cached_object();
	cached_object(refcount_auto_ptr<char> buf, size_t len);

	mutable size_t accesses;
	};
    cached_object load_file(const char* name);

    typedef std::map<std::string,cached_object> objectmap_t;
    objectmap_t objects;
    size_t total_size;
    };

extern FileCache* cache;

#endif //!defined(__FILE_CACHE_HH__)
