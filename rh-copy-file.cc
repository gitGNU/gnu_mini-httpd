/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::copy_file()
    {
    TRACE();

    if (write_buffer.empty())
        {
        char buf[4096];
        ssize_t rc = read(filefd, buf, sizeof(buf));
        if (rc < 0)
            throw system_error("read() from hard disk failed");
        else if (rc == 0)
            {
            debug(("%d: The complete file is copied: going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd));
            state = FLUSH_BUFFER_AND_TERMINATE;
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
