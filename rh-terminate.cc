/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::terminate()
    {
    TRACE();
    log_access();
    delete this;
    return false;
    }
