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
    //
    static unsigned int read_block_size;
    static unsigned int min_buffer_fill_size;
    static unsigned int max_buffer_fill_size;
    //
    static std::string document_root;
    //
    static int syslog_facility;
    };
extern const configuration* config;

#endif
