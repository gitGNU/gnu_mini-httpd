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
const RegExp RequestHandler::referer_regex("^Referer: +([^ ]+)", REG_EXTENDED);
const RegExp RequestHandler::user_agent_regex("^User-Agent: +(.*)", REG_EXTENDED);

bool RequestHandler::process_input(char* begin, char* end)
    {
    TRACE();

    if (begin == end)
	return true;

    char backup = *end;
    *end = '\0';

    static regmatch_t pmatch[10];
    if (host.empty() && url.empty() && regexec(full_get_port_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	host.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	url.assign(begin + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
	debug(("%d: Got full GET request including port: host = '%s' and url = '%s'.", sockfd, host.c_str(), url.c_str()));
	}
    else if (host.empty() && url.empty() && regexec(full_get_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	host.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	url.assign(begin + pmatch[2].rm_so, pmatch[2].rm_eo - pmatch[2].rm_so);
	debug(("%d: Got full GET request: host = '%s' and url = '%s'.", sockfd, host.c_str(), url.c_str()));
	}
    else if (url.empty() && regexec(get_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	url.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	debug(("%d: Got GET request: url = '%s'.", sockfd, url.c_str()));
	}
    else if (host.empty() && regexec(host_port_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	host.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	debug(("%d: Got host header including port: host = '%s'.", sockfd, host.c_str()));
	}
    else if (host.empty() && regexec(host_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	host.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	debug(("%d: Got host header: host = '%s'.", sockfd, host.c_str()));
	}
    else if (referer.empty() && regexec(referer_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	referer.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	debug(("%d: Got referer header: referer = '%s'.", sockfd, referer.c_str()));
	}
    else if (user_agent.empty() && regexec(user_agent_regex, begin, sizeof(pmatch) / sizeof(regmatch_t), pmatch, 0) == 0)
	{
	user_agent.assign(begin + pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so);
	debug(("%d: Got user_agent header: user-agent = '%s'.", sockfd, user_agent.c_str()));
	}
    else
	debug(("%d: Got unknown header '%s'.", sockfd, begin));

    *end = backup;
    return false;
    }
