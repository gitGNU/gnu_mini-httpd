/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPD_HH
#define HTTPD_HH

#include <sstream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <boost/scoped_array.hpp>
#include "libscheduler/scheduler.hh"
#include "HTTPRequest.hh"

// This is the HTTP protocol driver class.

class RequestHandler : public scheduler::event_handler
    {
  public:
    explicit RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin);
    virtual ~RequestHandler();

  private:
    // This function will (re-)initialize all internals. It will be
    // used to start the request handler over when using persistent
    // connections.

    void reset();

  private:
    // These are the callbacks required by the scheduler. These will
    // be called when the file descriptor we're registered for become
    // readable, writable, etc.

    virtual void fd_is_readable(int fd);
    virtual void fd_is_writable(int fd);
    virtual void read_timeout(int fd);
    virtual void write_timeout(int fd);
    virtual void error_condition(int fd);
    virtual void pollhup(int fd);

    // Helper functions to save some typing.

    void go_to_read_mode();
    void go_to_write_mode();

  private:
    // The whole class is implemented as a state machine. Depending on
    // the contents of the state variable, the appropriate state
    // handler will be called via a lookup table. each state handler
    // may return true (go on) or false (re-schedule). errors are
    // reported via exceptions.

    enum state_t
        {
        READ_REQUEST_LINE,
        READ_REQUEST_HEADER,
        READ_REQUEST_BODY,
        SETUP_REPLY,
        COPY_FILE,
        FLUSH_BUFFER,
        TERMINATE
        };
    state_t state;

    typedef bool (RequestHandler::*state_fun_t)();
    static const state_fun_t state_handlers[];

    bool get_request_line();
    bool get_request_header();
    bool get_request_body();
    bool setup_reply();
    bool copy_file();
    bool flush_buffer();
    bool terminate();

    void call_state_handler()
        {
        while((this->*state_handlers[state])())
            ;
        }

  private:
    // These are helper functions that will create the standard
    // replies of the server. The names should be rather descriptive.

    void protocol_error(const std::string& message);
    void moved_permanently(const std::string& path);
    void file_not_found();
    void not_modified();

  private:
    // The routine for making the logfile entries.

    void log_access();

  private:
    // Our I/O interface.

    scheduler&  mysched;
    int         sockfd;
    std::string read_buffer;
    std::string write_buffer;
    boost::scoped_array<char> line_buffer;

  private:
    // Information associated with the HTTP request.

    char         peer_address[64];
    HTTPRequest  request;
    bool         use_persistent_connection;

  private:
    // Information about the file associated with the request.

    std::string document_root;
    std::string filename;
    int         filefd;
    struct stat file_stat;

  public:
    // The number of instantiated RequestHandlers.

    static unsigned int instances;
    };

#endif
