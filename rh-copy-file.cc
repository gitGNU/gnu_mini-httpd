/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
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
