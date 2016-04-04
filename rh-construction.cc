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

#include <config.h>

#include <stdexcept>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "system-error.hh"
#include "RequestHandler.hh"
#include "config.hh"
#include "log.hh"

using namespace std;

unsigned int RequestHandler::instances = 0;

const RequestHandler::state_fun_t RequestHandler::state_handlers[] =
{
  &RequestHandler::get_request_line,
  &RequestHandler::get_request_header,
  &RequestHandler::get_request_body,
  &RequestHandler::setup_reply,
  &RequestHandler::copy_file,
  &RequestHandler::flush_buffer,
  &RequestHandler::terminate
};

RequestHandler::RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin)
    : mysched(sched), sockfd(fd), filefd(-1)
{
  TRACE();

  // Store the peer's address as ASCII string.

  if (inet_ntop(AF_INET, &sin.sin_addr, peer_address, sizeof(peer_address)) == 0)
    throw system_error("inet_ntop() failed");

  // Set socket parameters.

  linger ling;
  ling.l_onoff  = 0;
  ling.l_linger = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &ling, sizeof(linger)) == -1)
    throw system_error("cannot switch LINGER mode off");

  if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
    throw system_error("cannot set non-blocking mode");

  // Allocate our line buffer for reading.

  line_buffer.reset( new char[config->max_line_length] );

  // Initialize internal variables.

  reset();
  debug(("%d: Accepted new connection from peer '%s'.", sockfd, peer_address));
  ++instances;
}

void RequestHandler::reset()
{
  TRACE();

  // Freshen up the internal variables.

  state = READ_REQUEST_LINE;

  if (filefd >= 0)
  {
    close(filefd);
    filefd = -1;
  }

  request = HTTPRequest();
  request.start_up_time = time(0);

  go_to_read_mode();
}

RequestHandler::~RequestHandler()
{
  TRACE();

  debug(("%d: Closing connection to peer '%s'.", sockfd, peer_address));

  --instances;

  mysched.remove_handler(sockfd);

  close(sockfd);

  if (filefd >= 0)
    close(filefd);
}
