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

ostream& RequestHandler::connect_header(ostream& os)
    {
    if (is_persistent_connection())
        os << "Keep-Alive: timeout=" << config->network_read_timeout << ", max=100\r\n"
           << "Connection: Keep-Alive\r\n";
    else
        os << "Connection: close\r\n";
    return os;
    }
