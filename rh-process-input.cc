/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include "request-handler.hh"
using namespace std;

const RegExp RequestHandler::full_get_port_regex("^GET http://([^:]+):[0-9]+([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::full_get_regex("^GET http://([^/]+)([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::get_regex("^GET +([^ ]+) +HTTP/([0-9]+)\\.([0-9]+)", REG_EXTENDED);
const RegExp RequestHandler::host_port_regex("^Host: +([^ ]+):[0-9]+", REG_EXTENDED);
const RegExp RequestHandler::host_regex("^Host: +([^ ]+)", REG_EXTENDED);

bool RequestHandler::process_input(const char* begin, const char* end)
    {
    TRACE();
    string line(begin, end-begin);
    if (!line.empty())
	{
	vector<string> vec;
	if (host.empty() && uri.empty() && full_get_port_regex.submatch(line, vec))
	    {
	    host = vec[1];
	    uri  = vec[2];
	    debug("%d: Got full GET request including port: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
	    }
	else if (host.empty() && uri.empty() && full_get_regex.submatch(line, vec))
	    {
	    host = vec[1];
	    uri  = vec[2];
	    debug("%d: Got full GET request: host = '%s' and uri = '%s'.", sockfd, host.c_str(), uri.c_str());
	    }
	else if (uri.empty() && get_regex.submatch(line, vec))
	    {
	    uri = vec[1];
	    debug("%d: Got GET request: uri = '%s'.", sockfd, uri.c_str());
	    }
	else if (host.empty() && host_port_regex.submatch(line, vec))
	    {
	    host = vec[1];
	    debug("%d: Got Host header including port: host = '%s'.", sockfd, host.c_str());
	    }
	else if (host.empty() && host_regex.submatch(line, vec))
	    {
	    host = vec[1];
	    debug("%d: Got Host header: host = '%s'.", sockfd, host.c_str());
	    }
	else
	    {
	    debug("%d: Got unknown header '%s'.", sockfd, line.c_str());
	    }
	return false;
	}
    else
	return true;
    }
