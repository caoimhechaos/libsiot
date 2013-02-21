/*-
 * Copyright (c) 2012 Caoimhe Chaos <caoimhechaos@protonmail.com>,
 *                    Ancient Solutions. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions  of source code must retain  the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions  in   binary  form  must   reproduce  the  above
 *    copyright  notice, this  list  of conditions  and the  following
 *    disclaimer in the  documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS  SOFTWARE IS  PROVIDED BY  ANCIENT SOLUTIONS  AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO,  THE IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS
 * FOR A  PARTICULAR PURPOSE  ARE DISCLAIMED.  IN  NO EVENT  SHALL THE
 * FOUNDATION  OR CONTRIBUTORS  BE  LIABLE FOR  ANY DIRECT,  INDIRECT,
 * INCIDENTAL,   SPECIAL,    EXEMPLARY,   OR   CONSEQUENTIAL   DAMAGES
 * (INCLUDING, BUT NOT LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE,  DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT  LIABILITY,  OR  TORT  (INCLUDING NEGLIGENCE  OR  OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDED_SIOT_SERVER_H
#define INCLUDED_SIOT_SERVER_H 1

#include <thread++/threadpool.h>
#include <toolbox/scopedptr.h>
#include <siot/connection.h>
#include <string>

namespace toolbox
{
namespace siot
{
// Exception for errors which occurr during setup of the server.
class ServerSetupException : public std::exception
{
public:
	// Creates a new exception with the given error message.
	ServerSetupException(std::string errmsg) noexcept;

	// Retrieves the error message from the exception.
	virtual const char* what() const noexcept;

private:
	std::string errmsg_;
};

// Prototype of a class to notify when new connections have been established.
// The class should take ownership of the connection object.
class ConnectionCallback
{
public:
	// This method is invoked when a new connection "conn" has been made
	// to the server. The class should take ownership of the connection
	// object.
	virtual void ConnectionEstablished(Connection* conn) = 0;

	// Report an error which occurred trying to establish the socket.
	virtual void ConnectionFailed(string msg) = 0;
};

// The server class implements a server which accepts new connections and
// spawns off handlers for them.
class Server
{
public:
	// Create a new server and bind it to the address specified in
	// "addr". The callback "connected" is invoked with the Connection
	// structure when a new connection has been established.
	// "num_threads" sets the number of parallel processing threads in
	// the pool for answering requests to the clients.
	Server(std::string addr, ConnectionCallback* connected = 0,
			uint32_t num_threads = 16);

	// Stop listening and shut down the server. This will wait until
	// all outstanding connections are terminated.
	virtual ~Server();

	// Set the maximum number of parallel connections to "maxconn". The
	// default is SOMAXCONN.
	Server* SetMaxConnections(int maxconn);

	// Set the callback to be invoked when a new connection was
	// established.
	Server* SetConnectionCallback(ConnectionCallback* connected);

	// Start listening on the given address. This call will block, so you
	// may want to start it in a separate thread.
	Server* Listen();

private:
	ScopedPtr<ConnectionCallback> connected_;
	threadpp::ThreadPool executor_;
	int maxconn_;
	uint32_t num_threads_;

#ifdef _POSIX_SOURCE
	struct addrinfo *info_;
	int serverfd_;
#endif /* _POSIX_SOURCE */
};
}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_SERVER_H */
