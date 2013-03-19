/*-
 * Copyright (c) 2013 Caoimhe Chaos <caoimhechaos@protonmail.com>,
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

#include <openssl/ssl.h>

#include "siot/server.h"
#include "siot/ssl.h"
#include "unixsocketconnection.h"

struct sockaddr_storage;

namespace toolbox
{
namespace siot
{
namespace ssl
{
class OpenSSLConfig
{
public:
	OpenSSLConfig();
};

// This class represents a connection using OpenSSL for encryption.
class OpenSSLConnection : public UNIXSocketConnection
{
public:
	// Create a new OpenSSL server connection from the client connection
	// "socketid", connected to "peer", and kick off negociation with
	// the settings specified in "context".
	OpenSSLConnection(Server* srv, int socketid,
		       	struct sockaddr_storage* peer,
		       	ServerSSLContext* context);
	virtual ~OpenSSLConnection();

	// Implements Connection.
	virtual string Receive(size_t maxlen = -1, int flags = 0);
	virtual ssize_t Send(string data, int flags = 0);
	virtual uint64_t GetLastUse();
	virtual void SetBlocking(bool blocking = true);
	virtual void Shutdown();

private:
	const OpenSSLConfig& openssl_cfg_;
	bool blocking_;
	uint64_t last_use_;
	SSL_CTX* ssl_ctx_;
	SSL* ssl_handle_;
};
}  // namespace ssl
}  // namespace siot
}  // namespace toolbox
