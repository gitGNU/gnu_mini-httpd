/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

inline string time_to_ascii(time_t t)
        {
        char buffer[1024];
        size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
        if (len == 0 || len >= sizeof(buffer))
            throw std::logic_error("strftime() failed because an internal buffer is too small!");
        return buffer;
        }

bool RequestHandler::setup_reply()
    {
    TRACE();

    // Make sure we have a hostname.

    if (host.empty())
        {
        protocol_error("Your HTTP request did not contain a <tt>Host</tt> header.\r\n");
        return false;
        }

    // Construct the actual file name associated with the hostname and
    // URL, then check whether the file exists. If not, report an
    // error. If the URL points to a directory, send a redirect reply
    // pointing to the "index.html" file in that directory.

    string filename = config->document_root;
    filename += "/" + host + path;
    struct stat sbuf;
    if (stat(filename.c_str(), &sbuf) == -1)
        {
        info("%d: Can't stat requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
        file_not_found(path.c_str());
        return false;
        }

    if (S_ISDIR(sbuf.st_mode))
        {
        if (*path.rbegin() == '/')
            moved_permanently((path + "index.html").c_str());
        else
            moved_permanently((path + "/index.html").c_str());
        return false;
        }

    filefd = open(filename.c_str(), O_RDONLY, 0);
    if (filefd == -1)
        {
        info("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
        file_not_found(filename.c_str());
        return false;
        }
    state = COPY_FILE;
    debug(("%d: Retrieved file from disk: Going into COPY_FILE state.", sockfd));

    char buf[4096];
    snprintf(buf, sizeof(buf),
             "HTTP/1.0 200 OK\r\n"     \
             "Content-Type: %s\r\n"    \
             "Content-Length: %ld\r\n" \
             "Date: %s\r\n"            \
             "Last-Modified: %s\r\n"   \
             "\r\n",
             config->get_content_type(filename.c_str()),
             sbuf.st_size,
             time_to_ascii(time(0)).c_str(),
             time_to_ascii(sbuf.st_mtime).c_str());
    write_buffer = buf;

    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    return false;
    }
