/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPD_HH
#define HTTPD_HH

#include <string>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libscheduler/scheduler.hh"

class RequestHandler : public scheduler::event_handler
    {
  public:
    explicit RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin);
    virtual ~RequestHandler();

  private:
    void init();

    virtual void fd_is_readable(int);
    virtual void fd_is_writable(int);
    virtual void read_timeout(int);
    virtual void write_timeout(int);
    virtual void error_condition(int);
    virtual void pollhup(int);

    enum state_t
	{
	READ_REQUEST_LINE,
	READ_REQUEST_HEADER,
	READ_REQUEST_BODY,
        SETUP_REPLY,
	COPY_FILE,
	WRITE_REMAINING_DATA,
	TERMINATE
	};
    state_t state;

    bool get_request_line();
    bool get_request_header();
    bool get_request_body();
    bool setup_reply();
    bool copy_file();
    bool write_remaining_data();
    bool terminate();

    void call_state_handler()
        {
        while((this->*state_handlers[state])())
            ;
        }

    typedef bool (RequestHandler::*state_fun_t)();
    static const state_fun_t state_handlers[];

    bool parse_host_header();
    bool parse_user_agent_header();
    bool parse_referer_header();
    bool parse_connection_header();
    bool parse_keep_alive_header();
    bool parse_if_modified_since_header();

    typedef bool (RequestHandler::*parse_header_fun_t)();
    struct header_parser_t
        {
        const char*        name;
        parse_header_fun_t parser;
        };
    static const header_parser_t header_parsers[];

    void protocol_error(const char*);
    void file_not_found(const char*);
    void moved_permanently(const char*);

    void log_access() const throw();
    bool is_persistent_connection() const;
    const char* make_connection_header() const;

    scheduler& mysched;
    int sockfd;
    int filefd;

    std::string read_buffer, write_buffer;

    char peer_addr_str[32];
    std::string method, host, path, user_agent, referer;
    std::string connection, keep_alive;
    unsigned int returned_status_code;
    size_t returned_object_size;
    unsigned int minor_version, major_version;
    time_t if_modified_since;

  public:
    static unsigned int instances;
    };

#endif
