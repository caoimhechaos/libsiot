/*-
 * Copyright (c) 2012 Tonnerre Lombard <tonnerre@ancient-solutions.com>,
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// TODO(tonnerre): get rid of this hack
#define HAVE_CLIB_HASH_H 1
#include <clib/clib.h>

#include "siot/server.h"

#ifdef _POSIX_SOURCE
#include "unixsocketconnection.h"
#endif /* _POSIX_SOURCE */

namespace toolbox
{
namespace siot
{
ServerSetupException::ServerSetupException(std::string errmsg) noexcept
: errmsg_(errmsg)
{
}

const char*
ServerSetupException::what() const noexcept
{
	return errmsg_.c_str();
}

Server::Server(std::string addr, ConnectionCallback* connected,
		uint32_t num_threads)
: connected_(connected), executor_(num_threads)
{
#ifdef _POSIX_SOURCE
	int error = c_str2addrinfo(addr.c_str(), &info_);
	if (error)
		throw ServerSetupException(string(gai_strerror(error)));

	serverfd_ = socket(AF_INET6, SOCK_STREAM, 0);
	if (serverfd_ == -1)
	{
		freeaddrinfo(info_);
		throw ServerSetupException(strerror(errno));
	}
#endif /* _POSIX_SOURCE */
}

Server::~Server()
{
#ifdef _POSIX_SOURCE
	freeaddrinfo(info_);
	close(serverfd_);
#endif /* _POSIX_SOURCE */
}

Server*
Server::Listen()
{
#ifdef _POSIX_SOURCE
	socklen_t addrlen;
	int error;

	if (listen(serverfd_, maxconn_))
	{
		throw ServerSetupException(strerror(errno));
	}

	error = c_bind2addrinfo(serverfd_, info_);
	if (error)
	{
		close(serverfd_);
		freeaddrinfo(info_);
		throw ServerSetupException(strerror(errno));
	}

	for (;;)
	{
		ScopedPtr<struct sockaddr_storage> addr(new sockaddr_storage);
		int clientfd = accept(serverfd_,
				(struct sockaddr*) addr.Get(), &addrlen);
		if (clientfd == -1)
		{
			ScopedPtr<char> err(strerror(errno));
			connected_->ConnectionFailed(string(err.Get()));
			continue;
		}

		Connection* conn = new UNIXSocketConnection(clientfd,
				addr.Release());
		connected_->ConnectionEstablished(conn);
	}
#endif /* _POSIX_SOURCE */
	return this;
}

Server*
Server::SetMaxConnections(int maxconn)
{
	maxconn_ = maxconn;
	return this;
}

Server*
Server::SetConnectionCallback(ConnectionCallback* connected)
{
	connected_.Reset(connected);
	return this;
}

Connection::~Connection()
{
}
}  // namespace siot
}  // namespace toolbox
