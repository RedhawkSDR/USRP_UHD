/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK USRP_UHD.
 *
 * REDHAWK USRP_UHD is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK USRP_UHD is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef MULTICAST_H_
#define MULTICAST_H_

#include <arpa/inet.h>
#include <stdexcept>
#include "SourceNicUtils.h"
#include "unicast.h"

#ifdef __cplusplus
extern "C" {
#endif

connection_t multicast_client (const char* iface, const char* group, int port) throw (BadParameterError);
ssize_t multicast_receive (connection_t client, void* buffer, size_t bytes);
connection_t multicast_server (const char* iface, const char* group, int port);
ssize_t multicast_transmit (connection_t server, const void* buffer, size_t bytes);
int multicast_poll_in (connection_t client, int timeout);
void multicast_close(connection_t socket);

#ifdef __cplusplus
}
#endif

#endif /* MULTICAST_H_ */
