/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
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

#include <stdexcept>
#include <cstdio>
#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "timestamp-to-string.hh"
#include "search-and-replace.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

static inline string escape_quotes(const string& input)
    {
    return search_and_replace(input, "\"", "\\\"", true);
    }

void RequestHandler::log_access()
    {
    TRACE();

    // Shouldn't happen ... But better check rather than to log
    // nonsense.

    if (request.status_code.empty())
        {
        error("Can't write access log entry for connection with %d because there is no status code.", peer_address);
        return;
        }

    // Construct the path of the logfile.

    string logfile = config->logfile_directory + "/";
    if (request.host.empty())
        logfile += "no-hostname";
    else
        logfile += request.host + "-access";

    // Convert the object size now, so that we can write a string.
    // That's necessary, because in some cases we write "-" rather
    // than a number.

    char object_size[32];
    if (request.object_size.empty())
        strcpy(object_size, "-");
    else
        {
        int len = snprintf(object_size, sizeof(object_size), "%u", request.object_size.data());
        if (len < 0 || len > static_cast<int>(sizeof(object_size)))
            {
            error("Internal error: While formatting object_size, snprintf() exceeded the internal buffer.");
            // Just kidding.
            }
        }
    // Open it and write the entry.

    FILE* fh = fopen(logfile.c_str(), "a");
    if (fh == 0)
        throw system_error(string("Can't open logfile '") + logfile + "'");

    try
        {
        fprintf(fh, "%s - - [%s] \"%s %s HTTP/%u.%u\" %u %s \"%s\" \"%s\"\n",
                peer_address, time_to_logdate(request.start_up_time).c_str(),
                request.method.c_str(), escape_quotes(request.url.path).c_str(),
                request.major_version, request.minor_version,
                request.status_code.data(), object_size,
                escape_quotes(request.referer).c_str(),
                escape_quotes(request.user_agent).c_str());
        }
    catch(...)
        {
        fclose(fh);
        throw;
        }
    fclose(fh);
    }
