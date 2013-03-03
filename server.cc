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

#include <string>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /*  HAVE_SYS_TYPES_H */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#ifdef HAVE_CLIB_CLIB_H
#undef HAVE_CLIB_CLIB_H
#include <clib/clib.h>
#endif /* HAVE_CLIB_CLIB_H */

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif /* HAVE_SYS_EPOLL_H */

#include "siot/server.h"

#ifdef _POSIX_SOURCE
#include "unixsocketconnection.h"
#endif /* _POSIX_SOURCE */

namespace toolbox
{
namespace siot
{
ServerSetupException::ServerSetupException(string errmsg) noexcept
: errmsg_(errmsg)
{
}

const char*
ServerSetupException::what() const noexcept
{
	return errmsg_.c_str();
}

Server::Server(string addr, ConnectionCallback* connected,
		uint32_t num_threads)
: connected_(connected), executor_(num_threads), maxconn_(num_threads),
	num_threads_(num_threads), running_(true)
{
#ifdef _POSIX_SOURCE
	int error = c_str2addrinfo(addr.c_str(), &info_);
	if (error)
		throw ServerSetupException(string(gai_strerror(error)));

	serverfd_ = socket(AF_INET6, SOCK_STREAM, 0);
	if (serverfd_ == -1)
	{
		freeaddrinfo(info_);
		info_ = 0;
		throw ServerSetupException(strerror(errno));
	}
#endif /* _POSIX_SOURCE */
}

Server::~Server()
{
#ifdef _POSIX_SOURCE
	if (info_)
	{
		freeaddrinfo(info_);
		info_ = 0;
	}
	close(serverfd_);
#endif /* _POSIX_SOURCE */
}

void
Server::Shutdown()
{
	running_ = false;
}

void
Server::Listen()
{
#ifdef _POSIX_SOURCE
#ifdef HAVE_EPOLL_CREATE
	ListenEpoll();
#else /* !HAVE_EPOLL_CREATE */
#ifdef HAVE_KQUEUE
	ListenKQueue();
#else
#ifdef HAVE_SELECT
	ListenSelect();
#else /* !HAVE_SELECT */
	ListenPoll();
#endif
#endif /* HAVE_KQUEUE */
#endif /* HAVE_EPOLL_CREATE */
#endif /* _POSIX_SOURCE */
}

#ifdef _POSIX_SOURCE
#ifdef HAVE_EPOLL_CREATE
void
Server::ListenEpoll()
{
	struct epoll_event ev, events[num_threads_];
	socklen_t addrlen;
	int error;

	error = c_bind2addrinfo(serverfd_, info_);
	if (error)
	{
		close(serverfd_);
		freeaddrinfo(info_);
		info_ = 0;
		throw ServerSetupException(strerror(errno));
	}

	if (listen(serverfd_, maxconn_))
		throw ServerSetupException(strerror(errno));

	int epollfd = epoll_create(num_threads_);
	if (epollfd == -1)
	{
		std::stringstream ss;
		ss << num_threads_;
		throw ServerSetupException(string("epoll_create (") +
				ss.str() + "): " +
				string(strerror(errno)));
	}

	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
	ev.data.fd = serverfd_;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd_, &ev) == -1)
		throw ServerSetupException("epoll_ctl: " +
				string(strerror(errno)));

	memset(events, 0, num_threads_ * sizeof(struct epoll_event));

	while (running_)
	{
		int nfds = epoll_wait(epollfd, events, num_threads_, -1);
		if (nfds == -1)
			throw ServerSetupException("epoll_wait: " +
					string(strerror(errno)));

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == serverfd_)
			{
				ScopedPtr<struct sockaddr_storage> addr(
						new sockaddr_storage);
				int clientfd = accept(serverfd_,
						(struct sockaddr*) addr.Get(),
						&addrlen);
				if (clientfd == -1)
				{
					ScopedPtr<char> err(strerror(errno));
					connected_->ConnectionFailed(string(
								err.Get()));
					continue;
				}

				Connection* conn = new UNIXSocketConnection(
						this, clientfd,
						addr.Release());
				connections_[clientfd] = conn;
				memset(events, 0, num_threads_ *
						sizeof(struct epoll_event));

				ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR |
					EPOLLHUP | EPOLLET;
				ev.data.fd = clientfd;

				if (epoll_ctl(epollfd, EPOLL_CTL_ADD,
							clientfd, &ev) == -1)
				{
					throw ServerSetupException(
						"epoll_ctl: " +
						string(strerror(errno)));
				}

				// Run connected_->ConnectionEstablished(conn)
				google::protobuf::Closure* cc =
					google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::ConnectionEstablished,
							conn);
				executor_.Add(cc);
			}
			else if (events[n].data.fd > 0)
			{
				Connection* conn = connections_[
					events[n].data.fd];
				if (conn && (events[n].events & (EPOLLHUP |
								EPOLLRDHUP)))
				{
					// Call connected_->ConnectionTerminated(conn);
					google::protobuf::Closure* cc =
						google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::ConnectionTerminated,
							conn);
					executor_.Add(cc);
					connections_.erase(events[n].data.fd);
					delete conn;
				}
				else if (conn && (events[n].events & EPOLLERR))
				{
					// Call connected_->Error(conn);
					google::protobuf::Closure* cc =
						google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::Error,
							conn);
					executor_.Add(cc);
				}
				else if (conn && (events[n].events & EPOLLIN))
				{
					// Call connected_->DataReady(conn);
					google::protobuf::Closure* cc =
						google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::DataReady,
							conn);
					executor_.Add(cc);
				}
			}
		}

		memset(events, 0, num_threads_ * sizeof(struct epoll_event));
	}
}
#endif /* HAVE_EPOLL_CREATE */
#endif /* _POSIX_SOURCE */

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

ConnectionCallback::~ConnectionCallback()
{
}

void
ConnectionCallback::ConnectionFailed(std::string msg)
{
}

void
ConnectionCallback::ConnectionTerminated(Connection* conn)
{
}

void
ConnectionCallback::Error(Connection* conn)
{
}
}  // namespace siot
}  // namespace toolbox
