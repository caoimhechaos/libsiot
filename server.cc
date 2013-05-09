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
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif /* HAVE_MEMORY_H */

#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif /* HAVE_SYS_ERRNO_H */
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif /* HAVE_ERRNO_H */
#ifdef HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifdef HAVE_CLIB_CLIB_H
#undef HAVE_CLIB_CLIB_H
#include <clib/clib.h>
#endif /* HAVE_CLIB_CLIB_H */

#ifdef HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif /* HAVE_SYS_EPOLL_H */

#include <toolbox/expvar.h>
#include <thread++/mutex.h>

#include "siot/server.h"

#ifdef _POSIX_SOURCE
#include "unixsocketconnection.h"
#include "opensslconnection.h"
#endif /* _POSIX_SOURCE */

#ifndef HAVE_STRERROR
#define strerror(x) std::to_string(x)
#endif /* HAVE_STRERROR */

namespace toolbox
{
namespace siot
{
using ssl::OpenSSLConnection;
using threadpp::MutexLock;
using threadpp::ReadMutexLock;

static ExpMap<int64_t> client_connection_errors("client-connection-errors");
#ifdef _POSIX_SOURCE
static ExpMap<int64_t> accept_errors("accept-errors");
#ifdef HAVE_EPOLL_CREATE
static ExpMap<int64_t> epoll_errors("siot-epoll-errors");
#endif /* HAVE_EPOLL_CREATE */
#endif /* _POSIX_SOURCE */

ServerSetupException::ServerSetupException(const string& errmsg) noexcept
: errmsg_(errmsg)
{
}

const char*
ServerSetupException::what() const noexcept
{
	return errmsg_.c_str();
}

ClientConnectionException::ClientConnectionException(
		const std::string& identifier, const string& errmsg) noexcept
: identifier_(identifier), errmsg_(errmsg)
{
}

const char*
ClientConnectionException::what() const noexcept
{
	return errmsg_.c_str();
}

string
ClientConnectionException::identifier() const noexcept
{
	return identifier_;
}

Server::Server(string addr, ConnectionCallback* connected,
		uint32_t num_threads)
: connected_(connected), ssl_context_(0), executor_(num_threads + 1),
	maxconn_(num_threads), num_threads_(num_threads), max_idle_(-1),
	running_(true)
#ifdef _POSIX_SOURCE
	 , connections_lock_(ReadWriteMutex::Create())
#endif /* _POSIX_SOURCE */
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
	shutdown(serverfd_, SHUT_RDWR);
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
	int error;

	executor_.Add(google::protobuf::NewCallback(
				this,
				&Server::ReapConnectionsEpoll));

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

	epollfd_ = epoll_create(num_threads_);
	if (epollfd_ == -1)
	{
		std::stringstream ss;
		ss << num_threads_;
		throw ServerSetupException(string("epoll_create (") +
				ss.str() + "): " +
				string(strerror(errno)));
	}

	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP;
	ev.data.fd = serverfd_;

	if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, serverfd_, &ev) == -1)
		throw ServerSetupException("epoll_ctl: " +
				string(strerror(errno)));

	memset(events, 0, num_threads_ * sizeof(struct epoll_event));

	while (running_)
	{
		int nfds = epoll_wait(epollfd_, events, num_threads_,
				max_idle_ < 0 ? -1 : max_idle_ * 1000);
		if (nfds == -1)
		{
			string errmsg =
				string(strerror(errno));
			connected_->ConnectionFailed(errmsg);

			if (errno == ERESTART || errno == EINTR ||
				       	errno == EAGAIN)
			{
				epoll_errors.Add(errmsg, 1);
				continue;
			}
			else
				throw ServerSetupException("epoll_wait: " +
						errmsg);
		}

		for (int n = 0; n < nfds; ++n)
		{
			if (events[n].data.fd == serverfd_)
			{
				// A connection is waiting on the server
				// socket. We just accept it and wait for
				// data on it.
				ScopedPtr<struct sockaddr_storage> addr(
						new sockaddr_storage);
				socklen_t addrlen =
					sizeof(struct sockaddr_storage);
				int clientfd = accept(serverfd_,
						(struct sockaddr*) addr.Get(),
						&addrlen);
				if (clientfd == -1)
				{
					string errmsg =
						string(strerror(errno));
					connected_->ConnectionFailed(errmsg);
					accept_errors.Add(errmsg, 1);
					continue;
				}

				Connection* conn;
				if (ssl_context_)
				{
					try
					{
						conn = new OpenSSLConnection(
								this, clientfd,
								addr.Release(),
								ssl_context_);
					}
					catch (ClientConnectionException e)
					{
						client_connection_errors.Add(
								e.identifier(),
								1);
					}
				}
				else
				{
					try
					{
						conn = new UNIXSocketConnection(
								this, clientfd,
								addr.Release());
					}
					catch (ClientConnectionException e)
					{
						client_connection_errors.Add(
								e.identifier(),
								1);
					}
				}
				connections_lock_->Lock();
				connections_[clientfd] =
					connected_->AddDecorators(conn);
				connections_lock_->Unlock();
				memset(events, 0, num_threads_ *
						sizeof(struct epoll_event));

				ev.events = EPOLLIN | EPOLLRDHUP | EPOLLERR |
					EPOLLHUP | EPOLLET;
				ev.data.fd = clientfd;

				if (epoll_ctl(epollfd_, EPOLL_CTL_ADD,
							clientfd, &ev) == -1)
				{
					string errmsg =
						string(strerror(errno));
					connected_->ConnectionFailed(
								"epoll_ctl: " +
								errmsg);
					epoll_errors.Add(errmsg, 1);
					continue;
				}

				// Run connected_->ConnectionEstablished(conn)
				google::protobuf::Closure* cc =
					google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::ConnectionEstablished,
							conn);
				executor_.Add(google::protobuf::NewCallback(
							this,
							&Server::LockCallAndUnlock,
							cc));
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
					executor_.Add(google::protobuf::NewCallback(
								this,
								&Server::LockCallAndUnlock,
								cc));
					conn->Shutdown();
				}
				else if (conn && (events[n].events & EPOLLERR))
				{
					// Call connected_->Error(conn);
					google::protobuf::Closure* cc =
						google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::Error,
							conn);
					executor_.Add(google::protobuf::NewCallback(
								this,
								&Server::LockCallAndUnlock,
								cc));
				}
				else if (conn && (events[n].events & EPOLLIN))
				{
					// Call connected_->DataReady(conn);
					google::protobuf::Closure* cc =
						google::protobuf::NewCallback(
							connected_.Get(),
							&ConnectionCallback::DataReady,
							conn);
					executor_.Add(google::protobuf::NewCallback(
								this,
								&Server::LockCallAndUnlock,
								cc));
				}
			}
		}

		memset(events, 0, num_threads_ * sizeof(struct epoll_event));
	}
}

void
Server::ReapConnectionsEpoll()
{
	// TODO(caoimhe): Lock the same lock as above.
	std::mutex m;
	const uint64_t max_idle = (uint64_t) max_idle_;

	while (running_)
	{
		std::unique_lock<std::mutex> l(m);

		connections_updated_.wait_for(l,
				std::chrono::milliseconds(max_idle_ > 2 ?
					max_idle_ * 500 : 500));

		if (!running_)
			break;

		MutexLock lk(connections_lock_.Get());
		const uint64_t tm = time(NULL);

		for (std::pair<int, Connection*> s : connections_)
		{
			Connection* conn = s.second;

			if (conn->IsShutdown() ||
					(max_idle_ > 0 &&
					 tm - conn->GetLastUse() > max_idle))
			{
				connections_.erase(s.first);

				if (epoll_ctl(epollfd_, EPOLL_CTL_DEL,
							s.first, NULL) == -1)
				{
					string errmsg =
						string(strerror(errno));
					connected_->ConnectionFailed(
							"epoll_ctl: " +
							errmsg);
					epoll_errors.Add(errmsg, 1);
				}

				if (!conn->IsShutdown())
				{
					conn->Shutdown();
					connected_->ConnectionTerminated(conn);
				}
			}
		}
	}
}
#endif /* HAVE_EPOLL_CREATE */

void
Server::LockCallAndUnlock(Closure* c)
{
	ReadMutexLock l(connections_lock_.Get());
	c->Run();
}
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

Server*
Server::SetServerSSLContext(const ServerSSLContext* context)
{
	ssl_context_ = context;
	return this;
}

void
Server::DeferShutdown(Connection* conn)
{
	// Run conn->Shutdown(). Will not take the connection lock.
	google::protobuf::Closure* cc =
		google::protobuf::NewCallback(conn, &Connection::Shutdown);
	executor_.Add(cc);
}

void
Server::DequeueConnection(Connection* conn)
{
	for (std::pair<int, Connection*> it : connections_)
	{
		if (it.second == conn)
		{
			MutexLock l(connections_lock_.Get());
			connections_.erase(it.first);

			if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, it.first, NULL)
					== -1 && errno != EBADFD)
			{
				string errmsg =
					string(strerror(errno));
				connected_->ConnectionFailed(
						"epoll_ctl: " + errmsg);
				epoll_errors.Add(errmsg, 1);
			}

			connections_updated_.notify_one();
		}
	}
}

void
Server::SetMaxIdle(int max_idle)
{
	max_idle_ = max_idle;
}

Connection::Connection()
: mtx_(Mutex::Create()), is_shutdown_(false)
{
}

Connection::~Connection()
{
}

void
Connection::Shutdown()
{
	Deregister();
}

void
Connection::DeferredShutdown()
{
	Server* srv = GetServer();
	if (srv)
		srv->DeferShutdown(this);
	else
		Shutdown();
}

void
Connection::Deregister()
{
	Server* srv = this->GetServer();
	is_shutdown_ = true;
	if (srv)
		srv->DequeueConnection(this);
}

bool
Connection::IsShutdown()
{
	return is_shutdown_;
}

void
Connection::Lock()
{
	mtx_->Lock();
}

bool
Connection::TryLock()
{
	return mtx_->TryLock();
}

void
Connection::Unlock()
{
	mtx_->Unlock();
}

ConnectionCallback::~ConnectionCallback()
{
}

Connection*
ConnectionCallback::AddDecorators(Connection* in)
{
	return in;
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
