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
#ifndef UNICAST_H
#define UNICAST_H

#include <arpa/inet.h>
#include <stdexcept>
#include "SourceNicUtils.h"

class BadParameterError3 : public std::runtime_error {
       	public:
       		BadParameterError3(const std::string& what_arg) : std::runtime_error(what_arg) {
	        }
    };

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int sock;
  struct sockaddr_in addr;
} connection_t;

connection_t unicast_client (const char* iface, const char* group, int port) throw (BadParameterError);
ssize_t unicast_receive (connection_t client, void* buffer, size_t bytes, unsigned int to_in_msecs= 0);
connection_t unicast_server (const char* iface, const char* group, int port);
ssize_t unicast_transmit (connection_t server, const void* buffer, size_t bytes);
int unicast_poll_in (connection_t client, int timeout);
void unicast_close(connection_t socket);

#ifdef __cplusplus
}
#endif


#endif /* UNICAST_H */
