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

#ifndef INCLUDED_SIOT_LINEBUFFERDECORATOR_H
#define INCLUDED_SIOT_LINEBUFFERDECORATOR_H 1

#include <list>
#include <string>
#include <toolbox/scopedptr.h>
#include <siot/connection.h>

namespace toolbox
{
namespace siot
{
using std::string;

/**
 * Reads data from a connection and returns just the first line on each call.
 * The remainder will be stored in a buffer until it is needed. This is very
 * helpful when dealing with line based protocols.
 *
 * By default, this takes ownership of the connection handle. If you don't
 * want this behavior, set the own parameter to false.
 *
 * The Send() part is unchanged, i.e. you can send data in any format you
 * like.
 */
class LineBufferDecorator : public Connection
{
public:
	// Creates a new line buffer around the connection "wrapped". If
	// "own" is true (which is the default), this will take ownership
	// of "wrapped".
	explicit LineBufferDecorator(Connection* wrapped, bool own = true);
	virtual ~LineBufferDecorator();

	// Receive a line from the connected peer. The newline character
	// (\n, \r\n) will be returned as \n at the end of the line.
	virtual string Receive(size_t ignored = 0, int flags = 0);

	// Forwarded to wrapped connection object.
	virtual ssize_t Send(string data, int flags = 0);
	virtual string PeerAsText();
	virtual Server* GetServer();
	virtual bool IsEOF();
	virtual uint64_t GetLastUse();
	virtual void SetBlocking(bool blocking = true);
	virtual void Shutdown();
	virtual bool IsShutdown();

private:
	Connection* wrapped_;
	const bool owned_;
	string remainder_;
	std::list<string> remaining_lines_;
};

}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_LINEBUFFERDECORATOR_H */
