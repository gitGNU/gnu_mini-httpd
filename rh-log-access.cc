/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <stdexcept>
#include <cstdio>
#include <ctime>
#include "system-error/system-error.hh"
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

namespace
    {
    inline string time_to_ascii(time_t t)
        {
        char buffer[1024];
        size_t len = strftime(buffer, sizeof(buffer), "%d/%b/%Y:%H:%M:%S %z", localtime(&t));
        if (len == 0 || len >= sizeof(buffer))
            throw length_error("strftime() failed because the internal buffer is too small");
        return buffer;
        }
    }

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

        string timestamp = time_to_ascii(time(0));

        // Open it and write the entry.

        FILE* fh = fopen(filename, "a");
        if (fh == 0)
            throw system_error("Can't open logfile");

        fprintf(fh, "%s - - [%s] \"%s %s HTTP/1.1\" %u %u \"%s\" \"%s\"\n",
                peer_addr_str, timestamp.c_str(), method.c_str(), path.c_str(),
                returned_status_code, returned_object_size, referer.c_str(),
                user_agent.c_str());

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
