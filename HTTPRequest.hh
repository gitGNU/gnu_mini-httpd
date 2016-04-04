/*
 * Copyright (c) 2001-2016 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPREQUEST_HH_INCLUDED
#define HTTPREQUEST_HH_INCLUDED

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

#endif // HTTPREQUEST_HH_INCLUDED
