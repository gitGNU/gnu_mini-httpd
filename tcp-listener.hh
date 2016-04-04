/*
 * Copyright (c) 2001-2016 Peter Simons <simons@cryp.to>
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

#ifndef TCP_LISTENER_HH_INCLUDED
#define TCP_LISTENER_HH_INCLUDED

#include <stdexcept>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "system-error.hh"
#include "libscheduler/scheduler.hh"
#include "log.hh"

template<class connection_handlerT>
class TCPListener : public scheduler::event_handler
{
public:
  explicit TCPListener(scheduler& sched, short port_no, int queue_backlog = 50)
      : mysched(sched)
  {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
      throw system_error("socket() failed");
    try
    {
      int true_flag = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &true_flag, sizeof(int)) == -1)
        throw system_error("cannot set listen socket to REUSEADDR mode");

      if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
        throw system_error("cannot set listen socket to non-blocking mode");

      sin.sin_family      = AF_INET;
      sin.sin_addr.s_addr = htonl(INADDR_ANY);
      sin.sin_port        = htons(port_no);
      sin_size            = sizeof(sin);
      if (bind(sockfd, (sockaddr*)&sin, sin_size) == -1)
        throw system_error("bind() failed");

      if (listen(sockfd, queue_backlog) == -1)
        throw system_error("listen() failed");

      scheduler::handler_properties prop;
      prop.poll_events  = POLLIN;
      prop.read_timeout = 0;
      mysched.register_handler(sockfd, *this, prop);
    }
    catch (...)
    {
      close(sockfd);
      throw;
    }
  }

  virtual ~TCPListener()
  {
    mysched.remove_handler(sockfd);
    close(sockfd);
  }

private:
  virtual void fd_is_readable(int)
  {
    int streamfd = accept(sockfd, (sockaddr*) & sin, &sin_size);
    if (streamfd == -1)
    {
      error("TCPListener: failed to accept() new connection: %s", strerror(errno));
      return;
    }
    try
    {
      new connection_handlerT(mysched, streamfd, sin);
    }
    catch (const std::exception& e)
    {
      close(streamfd);
      error("TCPListener: caught exception while creating connection handler: %s", e.what());
    }
    catch (...)
    {
      close(streamfd);
      error("TCPListener: caught unknown exception while creating connection handler");
    }
  }
  virtual void fd_is_writable(int)
  {
    throw std::logic_error("this routine should not have be called");
  }
  virtual void read_timeout(int)
  {
    throw std::logic_error("this routine should not have been be called");
  }
  virtual void write_timeout(int)
  {
    throw std::logic_error("this routine should not have been be called");
  }
  virtual void error_condition(int)
  {
    error("TCPListener received an error condition by its socket: terminating");
    delete this;
  }
  virtual void pollhup(int fd)
  {
    error_condition(fd);
  }

  scheduler&   mysched;
  int          sockfd;
  sockaddr_in  sin;
  socklen_t    sin_size;
};

#endif // TCP_LISTENER_HH_INCLUDED
