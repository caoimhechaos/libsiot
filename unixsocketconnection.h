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

#ifndef INCLUDED_UNIXSOCKETCONNECTION_H
#define INCLUDED_UNIXSOCKETCONNECTION_H 1

#include <toolbox/scopedptr.h>
#include "siot/connection.h"

namespace toolbox
{
namespace siot
{
using toolbox::ScopedPtr;

class UNIXSocketConnection : public Connection
{
public:
	explicit UNIXSocketConnection(Server* srv, int socketid,
			struct sockaddr_storage* peer);
	virtual ~UNIXSocketConnection();

	// Implements Connection.
	virtual string Receive(size_t maxlen = -1, int flags = 0);
	virtual ssize_t Send(string data, int flags = 0);
	virtual string PeerAsText();
	virtual Server* GetServer();

private:
	int socket_;
	struct sockaddr_storage* peer_;
	Server* server_;
};
}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_UNIXSOCKETCONNECTION_H */
