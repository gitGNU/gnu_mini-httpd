/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "system-error/system-error.hh"
#include "request-handler.hh"
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
            debug(("%d: The complete file is copied: going into WRITE_REMAINING_DATA state.", sockfd));
            state = WRITE_REMAINING_DATA;
            close(filefd);
            filefd = -1;
            }
        else
            {
            debug(("%d: Read %d bytes from file.", sockfd, rc));
            write_buffer.assign(buf, rc);
            }
        }

    return false;
    }
