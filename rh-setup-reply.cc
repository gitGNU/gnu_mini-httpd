/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "request-handler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

namespace
    {
    inline string time_to_ascii(time_t t)
        {
        char buffer[1024];
        size_t len = strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
        if (len == 0 || len >= sizeof(buffer))
            throw std::logic_error("strftime() failed because an internal buffer is too small!");
        return buffer;
        }
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
            {
            moved_permanently((path + config->default_page).c_str());
            return false;
            }
        else
            {
            moved_permanently((path + "/" + config->default_page).c_str());
            return false;
            }
        }

    if (method == "HEAD")
        {
        state = WRITE_REMAINING_DATA;
        debug(("%d: Answering HEAD; going into WRITE_REMAINING_DATA state.", sockfd));

        ostringstream buf;
        buf << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: " << config->get_content_type(filename.c_str()) << "\r\n"
            << "Date: " << time_to_ascii(time(0)) << " \r\n"
            << "Last-Modified: " << time_to_ascii(sbuf.st_mtime) << "\r\n";
        connect_header(buf);
        buf << "\r\n";
        write_buffer = buf.str();
        returned_status_code = 200;
        returned_object_size = 0;
        }
    else if (method == "GET")
        {
        filefd = open(filename.c_str(), O_RDONLY, 0);
        if (filefd == -1)
            {
            info("%d: Can't open requested file %s: %s", sockfd, filename.c_str(), strerror(errno));
            file_not_found(filename.c_str());
            return false;
            }
        state = COPY_FILE;
        debug(("%d: Answering GET; going into COPY_FILE state.", sockfd));

        ostringstream buf;
        buf << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: " << config->get_content_type(filename.c_str()) << "\r\n"
            << "Content-Length: " << sbuf.st_size << "\r\n"
            << "Date: " << time_to_ascii(time(0)) << " \r\n"
            << "Last-Modified: " << time_to_ascii(sbuf.st_mtime) << "\r\n";
        connect_header(buf);
        buf << "\r\n";
        write_buffer = buf.str();
        returned_status_code = 200;
        returned_object_size = sbuf.st_size;
        }

    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    return false;
    }
