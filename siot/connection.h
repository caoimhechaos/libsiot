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

#ifndef INCLUDED_SIOT_CONNECTION_H
#define INCLUDED_SIOT_CONNECTION_H 1

#include <string>

namespace toolbox
{
namespace siot
{
using std::string;
class Server;

// Prototype of a connection. The implementation may be OS specific.
class Connection
{
public:
	// Some standard initializations for new connections.
	Connection();

	// Close the connection. Users of this object should always call
	// Shutdown() instead.
	virtual ~Connection();

	// Read up to maxlen bytes from the connection.
	virtual string Receive(size_t maxlen = -1, int flags = 0) = 0;

	// Send the bytes referred to by "data" over the connection.
	virtual ssize_t Send(string data, int flags = 0) = 0;

	// Get a string describing the peer the socket connects to.
	virtual string PeerAsText() = 0;

	// Gets the server this connection is bound to, or 0 if this is a
	// client socket.
	virtual Server* GetServer() = 0;

	// Determine if the end of the receivable data has been reached.
	virtual bool IsEOF() = 0;

	// Determine when the last bit of information was exchanged over
	// the socket.
	virtual uint64_t GetLastUse() = 0;

	// Sets the connection to blocking or non-blocking state.
	virtual void SetBlocking(bool blocking = true) = 0;

	// Disconnects the socket and removes it from the notification queues.
	// This should call Deregister() and then close the connection.
	virtual void Shutdown();

	// If a server is associated with the connection, instruct the server
	// to shut the connection down on the next occasion. Otherwise, just
	// run the shutdown method.
	virtual void DeferredShutdown();

	// Determines whether the server is shut down. This signal is set by
	// Deregister().
	virtual bool IsShutdown();

protected:
	// Tell the associated server to deregister the connection. If no
	// server connection was associated, this just cleans up the connection
	// object.
	virtual void Deregister();

	bool is_shutdown_;
};
}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_CONNECTION_H */
