/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __CONFIG_HH__
#define __CONFIG_HH__

#include <ext/hash_map>

class configuration
    {
  public:
    // Construction and Destruction.
    configuration();
    ~configuration();

    // Timeouts.
    static unsigned int network_read_timeout;
    static unsigned int network_write_timeout;
    static unsigned int file_read_timeout;

    // Buffer sizes.
    static unsigned int io_buffer_size;

    // Paths.
    static char* document_root;

    // Miscellaneous.
    static char* default_content_type;

    // Content-type mapping.
    const char* get_content_type(const char* filename) const;

  private:
    configuration(const configuration&);
    configuration& operator= (const configuration&);

    struct eqstr
	{
	bool operator()(const char* lhs, const char* rhs) const
	    { return strcasecmp(lhs, rhs) == 0; }
	};
    typedef std::hash_map<const char*,const char*,std::hash<const char*>,eqstr> map_t;
    map_t content_types;
    };
extern const configuration* config;

#endif
