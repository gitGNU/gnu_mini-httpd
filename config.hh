/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __CONFIG_HH__
#define __CONFIG_HH__

#include <string>

class configuration
    {
  public:
    configuration() { }
    ~configuration() { }

    static unsigned int network_read_timeout;
    static unsigned int network_write_timeout;
    static unsigned int file_read_timeout;
    static unsigned int file_read_buffer_size;
    static unsigned int network_read_buffer_size;
    static unsigned int buffer_too_empty_threshold;
    static std::string document_root;
    };
extern const configuration* config;

#endif
