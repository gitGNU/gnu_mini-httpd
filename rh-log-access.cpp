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

#include "RequestHandler.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sanity/system-error.hpp>
#include <boost/algorithm/string/replace.hpp>
#include "timestamp-to-string.hpp"
#include "config.hpp"

using namespace std;

inline void escape_quotes(string & input)
{
  boost::algorithm::replace_all(input, "\"", "\\\"");
}

void RequestHandler::log_access()
{
  if (!request.status_code)
  {
    HERROR() "Can't write access log entry because there is no status code.";
    return;
  }

  string logfile( config->logfile_directory + "/" );
  if (request.host.empty())     logfile += "no-hostname";
  else                          logfile += request.host + "-access";

  // Convert the object size now, so that we can write a string.
  // That's necessary, because in some cases we write "-" rather
  // than a number.

  char object_size[32];
  if (!request.object_size)
    strcpy(object_size, "-");
  else
  {
    int len = snprintf(object_size, sizeof(object_size), "%u", *request.object_size);
    if (len < 0 || len > static_cast<int>(sizeof(object_size)))
      HERROR() "internal error: snprintf() exceeded buffer size while formatting log entry";
  }
  // Open it and write the entry.

  ofstream os(logfile.c_str(), ios_base::out); // no ios_base::trunc
  if (!os.is_open())
    throw system_error((string("Can't open logfile '") + logfile + "'"));

  escape_quotes(request.url.path);
  escape_quotes(request.referer);
  escape_quotes(request.user_agent);

  // "%s - - [%s] \"%s %s HTTP/%u.%u\" %u %s \"%s\" \"%s\"\n"
  os << "TODO" << " - - [" << time_to_logdate(request.start_up_time) << "]"
     << " \"" << request.method
     << " "  << request.url.path
     << " HTTP/" << request.major_version
     << "." << request.minor_version << "\""
     << " " << *request.status_code
     << " " << object_size
     << " \"" << request.referer << "\""
     << " \"" << request.user_agent << "\""
     << endl;
}
