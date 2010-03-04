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

#include "system-error.hh"
#include "RequestHandler.hh"
#include "config.hh"
#include "log.hh"
using namespace std;

/*
  This callback is invoked every time socket becomes readable. So what
  we do is to read up to 4kb of data into our line buffer and then
  jump into the state handlers. They will process the data and remove
  anything that's been dealt with. If the buffer overflows, it means
  someone sent us a single header line that was longer than the 4kb
  buffer limit -- obviously a jerk, so we reject the request.
*/

void RequestHandler::fd_is_readable(int)
    {
    TRACE();
    try
	{
        // Protect against flooding.

        if (read_buffer.size() > config->max_line_length)
            {
            protocol_error("This server won't process excessively long\r\n" \
                           "request header lines.\r\n");
            return;
            }

        // Read sockfd stuff into the line buffer.

        ssize_t rc = read(sockfd, line_buffer.get(), config->max_line_length);
        if (rc < 0)
            {
            if (errno != EINTR)
                throw system_error("read() failed");
            else
                return;
            }
        else if (rc == 0)
            {
            if (state != READ_REQUEST_LINE || read_buffer.empty() == false)
                info("Connection to %s was terminated by peer.", peer_address);
            state = TERMINATE;
            }
        else
            read_buffer.append(line_buffer.get(), rc);

        // Call the state handler.

        call_state_handler();
        }
    catch(const exception& e)
	{
	error("Run-time error on connection to %s: %s", peer_address, e.what());
	delete this;
	}
    catch(...)
	{
	error("Unspecified run-time error on connection to %s; shutting down.", peer_address);
	delete this;
	}
    }

/*
  The "writable" callback will sent any data waiting in write_buffer
  to the peer -- unless we're in TERMINATE state, obviously. Then it
  will call the state handlers in case there's any work left to do.
*/

void RequestHandler::fd_is_writable(int)
    {
    TRACE();
    try
 	{
        // If there is output waiting in the write buffer, write it.

        if (state != TERMINATE && !write_buffer.empty())
            {
            ssize_t rc = write(sockfd, write_buffer.data(), write_buffer.size());
            if (rc < 0)
                {
                if (errno != EINTR)
                    throw system_error("write() failed");
                else
                    return;
                }
            else if (rc == 0)
                {
                info("Connection to %s was terminated by peer.", peer_address);
                state = TERMINATE;
                }
            else
                write_buffer.erase(0, rc);
            }

        // Call state handler.

        call_state_handler();
        }
    catch(const exception& e)
	{
	error("Run-time error on connection to %s: %s", peer_address, e.what());
	delete this;
	}
    catch(...)
	{
	error("Unspecified run-time error on connection to %s; shutting down.", peer_address);
	delete this;
	}
    }

// These callbacks have rather descriptive names ...

void RequestHandler::read_timeout(int)
    {
    TRACE();
    if (state != READ_REQUEST_LINE || read_buffer.empty() == false)
        info("No activity on connection to %s for %u seconds; shutting down.",
             peer_address, config->network_read_timeout);
    delete this;
    }

void RequestHandler::write_timeout(int)
    {
    TRACE();
    info("Couldn't send any data to %s for %u seconds; shutting down.",
         peer_address, config->network_write_timeout);
    delete this;
    }

void RequestHandler::error_condition(int)
    {
    TRACE();
    error("Unspecified error on connection to %s; shutting down.", peer_address);
    delete this;
    }

void RequestHandler::pollhup(int)
    {
    TRACE();
    if (state == TERMINATE)
        call_state_handler();
    else
        {
        if (state != READ_REQUEST_LINE || read_buffer.empty() == false)
            info("Connection to %s was terminated by peer.", peer_address);
        delete this;
        }
    }

void RequestHandler::go_to_read_mode()
    {
    scheduler::handler_properties prop;
    prop.poll_events   = POLLIN;
    prop.read_timeout  = config->network_read_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }

void RequestHandler::go_to_write_mode()
    {
    scheduler::handler_properties prop;
    prop.poll_events   = POLLOUT;
    prop.write_timeout = config->network_write_timeout;
    mysched.register_handler(sockfd, *this, prop);
    }
