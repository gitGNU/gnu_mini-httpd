/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::terminate()
    {
    TRACE();
    delete this;
    return false;
    }
