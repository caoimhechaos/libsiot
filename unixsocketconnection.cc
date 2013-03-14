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

#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif /* HAVE_STDINT_H */
#include <unistd.h>

#include <time.h>

// TODO(caoimhe): get rid of this hack
#define HAVE_CLIB_HASH_H 1
#include <clib/clib.h>

#include "siot/connection.h"
#include "unixsocketconnection.h"

namespace toolbox
{
namespace siot
{
UNIXSocketConnection::UNIXSocketConnection(Server* srv, int socketid,
		struct sockaddr_storage* peer)
: socket_(socketid), peer_(peer), server_(srv), eof_(false),
	last_use_(time(NULL))
{
}

UNIXSocketConnection::~UNIXSocketConnection()
{
	shutdown(socket_, SHUT_RDWR);
	close(socket_);
}

string
UNIXSocketConnection::Receive(size_t maxlen, int flags)
{
	ssize_t len;
	if (maxlen > 65536)
		len = 65536;
	else
		len = maxlen;
	ScopedPtr<char> buf(new char[len]);

	len = recv(socket_, buf.Get(), len, flags);

	if (len == -1)
	{
		len = 0;
		if (errno == EBADF || errno == EINVAL || errno == ENOTCONN)
			eof_ = true;
	}
	last_use_ = time(NULL);
	return string(buf.Get(), len);
}

ssize_t
UNIXSocketConnection::Send(string data, int flags)
{
	last_use_ = time(NULL);
	return send(socket_, data.c_str(), data.size(), flags);
}

string
UNIXSocketConnection::PeerAsText()
{
	ScopedPtr<char> addr_str(c_sockaddr2str(peer_));
	return string(addr_str.Get());
}

Server*
UNIXSocketConnection::GetServer()
{
	return server_;
}

bool
UNIXSocketConnection::IsEOF()
{
	return eof_;
}

uint64_t
UNIXSocketConnection::GetLastUse()
{
	return last_use_;
}

}  // namespace siot
}  // namespace toolbox
