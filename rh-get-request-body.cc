/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::get_request_body()
    {
    TRACE();

    // We ain't reading any bodies yet.

    state = SETUP_REPLY;
    return true;
    }
