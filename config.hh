/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __CONFIG_HH__
#define __CONFIG_HH__

#include <string>
#include <map>

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
    static unsigned int read_block_size;
    static unsigned int min_buffer_fill_size;
    static unsigned int max_buffer_fill_size;

    // Paths.
    static std::string document_root;

    // Miscellaneous.
    static char* default_content_type;

    // Content-type mapping.
    const char* get_content_type(const std::string& filename) const;

  private:
    configuration(const configuration&);
    configuration& operator= (const configuration&);

    struct ltstr
	{
	bool operator()(const std::string& lhs, const std::string& rhs) const
	    { return strcasecmp(lhs.c_str(), rhs.c_str()) < 0; }
	};
    std::map<std::string,std::string,ltstr> content_types;
    };
extern const configuration* config;

#endif
