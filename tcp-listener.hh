/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __TCP_LISTENER_HH__
#define __TCP_LISTENER_HH__

#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "system-error.hh"
#include "libscheduler/scheduler.hh"

template<class connection_handlerT>
class TCPListener : public scheduler::event_handler
    {
  public:
    explicit TCPListener(scheduler& sched, u_int16_t port_no, int queue_backlog = 50)
	    : mysched(sched)
	{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	    throw system_error("socket() failed");

	try {
	    int true_flag = 1;
	    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true_flag, sizeof(int)) == -1)
		throw system_error("Can't set listen socket to REUSEADDR mode");

	    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw system_error("Can't set listen socket to non-blocking mode");

	    sin.sin_family      = AF_INET;
	    sin.sin_addr.s_addr = htonl(INADDR_ANY);
	    sin.sin_port        = htons(port_no);
	    sin_size            = sizeof(sin);
	    if (bind(sockfd, (sockaddr*)&sin, sin_size) == -1)
		throw system_error("bind() failed");

	    if (listen(sockfd, queue_backlog) == -1)
		throw system_error("listen() failed");
	    }
	catch(...)
	    {
	    close(sockfd);
	    throw;
	    }

	scheduler::handler_properties prop;
	prop.poll_events   = POLLIN;
	prop.read_timeout  = 0;
	prop.write_timeout = 0;
	mysched.register_handler(sockfd, *this, prop);

	cerr << "Listening on TCP port " << port_no << " for incoming requests ..." << endl;
	}

    ~TCPListener()
	{
	mysched.remove_handler(sockfd);
	close(sockfd);
	}

  private:
    virtual void fd_is_readable(int)
	{
	int streamfd = accept(sockfd, (sockaddr*)&sin, &sin_size);
	if (streamfd == -1)
	    {
	    cerr << "TCPListener: Failed to accept new connection with accept(). Continuing.\n";
	    return;
	    }
	try { new connection_handlerT(mysched, streamfd, sin); }
	catch(const exception& e)
	    {
	    cerr << "TCPListener: Caught exception while creating connection handler: " << e.what() << endl;
	    }
	catch(...)
	    {
	    cerr << "TCPListener: Caught unknown exception while creating connection handler." << endl;
	    }
	}
    virtual void fd_is_writable(int)
        {
        throw std::logic_error("This routine should not be called.");
        }
    virtual void read_timeout(int)
        {
        throw std::logic_error("This routine should not be called.");
        }
    virtual void write_timeout(int)
        {
        throw std::logic_error("This routine should not be called.");
        }

    scheduler&   mysched;
    int          sockfd;
    sockaddr_in  sin;
    socklen_t    sin_size;
    };

#endif //!defined(__TCP_LISTENER_HH__)
