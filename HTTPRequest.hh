/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPREQUEST_HH
#define HTTPREQUEST_HH

#include <string>
#include <ctime>
#include "resetable-variable.hh"

// This class contains the relevant information in an (HTTP) URL.

struct URL
    {
    std::string                      host;
    resetable_variable<unsigned int> port;
    std::string                      path;
    std::string                      query;
    };


// This class contains all relevant information in an HTTP request.

struct HTTPRequest
    {
    time_t                           start_up_time;
    std::string                      method;
    URL                              url;
    unsigned int                     major_version;
    unsigned int                     minor_version;
    std::string                      host;
    resetable_variable<unsigned int> port;
    std::string                      connection;
    std::string                      keep_alive;
    resetable_variable<time_t>       if_modified_since;
    std::string                      user_agent;
    std::string                      referer;
    resetable_variable<unsigned int> status_code;
    resetable_variable<size_t>       object_size;
    };

#endif // !defined(HTTPREQUEST_HH)
