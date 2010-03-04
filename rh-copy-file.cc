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

#include "system-error.hh"
#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

/*
   In COPY_FILE state, all we do is fill up the write_buffer with data
   when it's empty, and if the file is through, we'll go into any of
   the FLUSH_BUFFER states -- depending on whether we support
   persistent connections or not -- to go on.
*/

bool RequestHandler::copy_file()
    {
    TRACE();

    if (write_buffer.empty())
        {
        char buf[4096];
        ssize_t rc = read(filefd, buf, sizeof(buf));
        if (rc < 0)
            {
            if (errno != EINTR)
                throw system_error(string("read() from file '") + filename + "' failed");
            else
                return true;
            }
        else if (rc == 0)
            {
            debug(("%d: The complete file is copied: going into FLUSH_BUFFER state.", sockfd));
            state = FLUSH_BUFFER;
            close(filefd);
            filefd = -1;
            }
        else
            {
            write_buffer.assign(buf, rc);
            }
        }

    return false;
    }
