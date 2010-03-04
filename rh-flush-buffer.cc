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

#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::flush_buffer()
{
  TRACE();

  if (write_buffer.empty())
  {
    log_access();
    if (use_persistent_connection)
    {
      debug(("%d: Connection is persistent; restarting.", sockfd));
      reset();
      return true;
    }
    else
    {
      state = TERMINATE;
      if (shutdown(sockfd, SHUT_RDWR) == -1)
        delete this;
    }
  }
  return false;
}
