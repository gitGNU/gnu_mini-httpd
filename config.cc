/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "config.hh"

#define kb *1024
#define sec *1

// Timeouts.
unsigned int configuration::network_read_timeout        = 30 sec;
unsigned int configuration::network_write_timeout       = 30 sec;
unsigned int configuration::file_read_timeout           =  0 sec;

// Buffer sizes.
unsigned int configuration::file_read_buffer_size       = 16 kb;
unsigned int configuration::network_read_buffer_size    =  1 kb;
unsigned int configuration::buffer_too_empty_threshold  =  4 kb;

// Paths.
std::string configuration::document_root                = DOCUMENT_ROOT;
