/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::is_persistent_connection() const
    {
    TRACE();

    if (major_version < 1)
        return false;

    if (strcasecmp(connection.c_str(), "keep-alive") == 0)
        return true;

    return false;
    }


static const char non_persistant[] = "Connection: close\r\n";
static const char persistant[]     = "Connection: Keep-Alive\r\nKeep-Alive: timeout=15\r\n"; // TODO

const char* RequestHandler::make_connection_header() const
    {
    TRACE();

    if (is_persistent_connection())
        return persistant;
    else
        return non_persistant;
    }
