/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef HTTPREQUEST_HH
#define HTTPREQUEST_HH

#include <boost/optional.hpp>
#include <string>
#include <ctime>

// This class contains the relevant information in an (HTTP) URL.

struct URL
{
  std::string                      host;
  boost::optional<unsigned int>    port;
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
  boost::optional<unsigned int>    port;
  std::string                      connection;
  std::string                      keep_alive;
  boost::optional<time_t>          if_modified_since;
  std::string                      user_agent;
  std::string                      referer;
  boost::optional<unsigned int>    status_code;
  boost::optional<size_t>          object_size;
};

#endif // !defined(HTTPREQUEST_HH)
