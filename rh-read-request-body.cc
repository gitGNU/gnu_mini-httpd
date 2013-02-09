/*
 * Copyright (c) 2001-2013 Peter Simons <simons@cryp.to>
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

#include "RequestHandler.hh"
#include "log.hh"

using namespace std;

bool RequestHandler::get_request_body()
{
  TRACE();

  // We ain't reading any bodies yet.

  debug(("%d: No request body; going into SETUP_REPLY state.", sockfd));
  state = SETUP_REPLY;
  return true;
}
