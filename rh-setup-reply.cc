/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <climits>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "system-error/system-error.hh"
#include "RequestHandler.hh"
#include "timestamp-to-string.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

static inline bool is_path_in_hierarchy(const char* hierarchy, const char* path)
    {
    char resolved_hierarchy[PATH_MAX];
    char resolved_path[PATH_MAX];

    if (realpath(hierarchy, resolved_hierarchy) == 0 || realpath(path, resolved_path) == 0)
        return false;
    if (strlen(resolved_hierarchy) > strlen(resolved_path))
        return false;
    if (strncmp(resolved_hierarchy, resolved_path, strlen(resolved_hierarchy)) != 0)
        return false;
    return true;
    }

bool RequestHandler::setup_reply()
    {
    TRACE();

    // Make sure we have a hostname.

    if (request.host.empty())
        {
        protocol_error("Your HTTP request did not contain a <tt>Host</tt> header.\r\n");
        return false;
        }

    // Construct the actual file name associated with the hostname and
    // URL, then check whether the file exists. If not, report an
    // error. If the URL points to a directory, send a redirect reply
    // pointing to the "index.html" file in that directory.

    document_root = string(config->document_root) + "/" + request.host.data();
    filename = document_root + request.url.path.data();
    struct stat sbuf;
    if (is_path_in_hierarchy(document_root.c_str(), filename.c_str()) == false||
        stat(filename.c_str(), &sbuf) == -1)
        {
        info("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
        file_not_found(request.url.path);
        return false;
        }
    if (S_ISDIR(sbuf.st_mode))
        {
        if (*request.url.path.rbegin() == '/')
            moved_permanently(request.url.path + config->default_page);
        else
            moved_permanently(request.url.path + "/" + config->default_page);
        return false;
        }

    // Check whether the If-Modified-Since header applies.

#if 0
    debug(("%d: file mtime = %d; if-modified-since = %d; timezone = %d", sockfd, sbuf.st_mtime, if_modified_since, timezone));
    if (if_modified_since > 0 && sbuf.st_mtime <= if_modified_since)
        {
        not_modified();
        return false;
        }
#endif

    // Now answer the request, which may be either HEAD or GET.

    ostringstream buf;
    buf << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: " << config->get_content_type(filename.c_str()) << "\r\n"
        << "Content-Length: " << sbuf.st_size << "\r\n";
    buf << "Date: " << time_to_rfcdate(time(0)) << "\r\n";
    buf << "Last-Modified: " << time_to_rfcdate(sbuf.st_mtime) << "\r\n";
    buf << "\r\n";
    write_buffer = buf.str();
    request_status_code = 200;

    if (request.method == "HEAD")
        {
        state = FLUSH_BUFFER_AND_TERMINATE;
        debug(("%d: Answering HEAD; going into FLUSH_BUFFER_AND_TERMINATE state.", sockfd));
        }
    else // must be GET
        {
        filefd = open(filename.c_str(), O_RDONLY, 0);
        if (filefd == -1)
            {
            info("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
            file_not_found(request.url.path);
            return false;
            }
        object_size = sbuf.st_size;
        state = COPY_FILE;
        debug(("%d: Answering GET; going into COPY_FILE state.", sockfd));
        }

    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    return false;
    }
