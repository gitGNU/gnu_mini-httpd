/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <cstdio>
#include <ctime>
#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "config.hh"
#include "timestamp-to-string.hh"
#include "log.hh"

using namespace std;

namespace
    {
    inline string escape_quotes(const string& str)
        {
        string tmp = str;
        for (string::size_type pos = 0; pos < tmp.size(); ++pos)
            {
            if (tmp[pos] == '"')
                {
                tmp.replace(pos, 1, "\\\"");
                ++pos;
                }
            }
        return tmp;
        }
    }
#if 0
void RequestHandler::log_access() const throw()
    {
    try
        {
        TRACE();

        // Construct the path of the logfile.

        char filename[4096];
        int len = snprintf(filename, sizeof(filename), config->logfile, host.c_str());
        if (len < 0 || len >= static_cast<int>(sizeof(filename)))
            throw length_error("logfile path exceeds size of internal buffer");
        debug(("logfile = '%s'", filename));

        // Create the data for the log entry.

        string timestamp = time_to_logdate(time(0));

        // Open it and write the entry.

        FILE* fh = fopen(filename, "a");
        if (fh == 0)
            throw system_error("Can't open logfile");

        fprintf(fh, "%s - - [%s] \"%s %s HTTP/%u.%u\" %u %u \"%s\" \"%s\"\n",
                peer_addr_str, timestamp.c_str(), method.c_str(),
                escape_quotes(path).c_str(), major_version, minor_version,
                returned_status_code, returned_object_size,
                escape_quotes(referer).c_str(),
                escape_quotes(user_agent).c_str());

        fclose(fh);
        }
    catch(const exception& e)
        {
        error("Caught exception: %s", e.what());
        }
    catch(...)
        {
        }
    }
#endif
