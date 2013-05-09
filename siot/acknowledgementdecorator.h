/*-
 * Copyright (c) 2013 Tonnerre Lombard <tonnerre@ancient-solutions.com>,
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

#ifndef INCLUDED_SIOT_ACKNOWLEDGEMENTDECORATOR_H
#define INCLUDED_SIOT_ACKNOWLEDGEMENTDECORATOR_H 1

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
 * Upon a call to Receive(), this will always attempt to read one unit of
 * data from the underlying connection and add it to the buffer of
 * unacknowledged data. Then, the whole buffer will be returned. The user
 * then has to acknowledge the amount of data which was has processed, the rest
 * will be returned again on the next call to Receive().
 *
 * By default, this takes ownership of the connection handle. If you don't
 * want this behavior, set the own parameter to false.
 *
 * The Send() part is unchanged, i.e. you can send data in any format you
 * like.
 */
class AcknowledgementDecorator : public Connection
{
public:
	// Creates a new acknowledgement buffer around the connection
	// "wrapped". It will buffer up to "max_buffer_size" bytes of
	// unacknowledged data. If "own" is true (which is the default), this
	// will take ownership of "wrapped".
	explicit AcknowledgementDecorator(Connection* wrapped,
			uint64_t max_buffer_size, bool own = true);
	virtual ~AcknowledgementDecorator();

	// Receive an unit of data from the connected peer. This will return
	// all unacknowledged data, including the data which was just read.
	virtual string Receive(size_t maxlen = 0, int flags = 0);

	// Acknowledge "bytes" number of bytes from the internal buffer so
	// they won't be returned again on the next call.
	virtual bool Acknowledge(size_t bytes);

	// This will return true when the end-of-line indicator is set on the
	// underlying connection object and the internal buffer is empty, so
	// that no unacknowledged data could possibly still exist on the
	// connection.
	virtual bool IsEOF();

	// Enable/disable automatic acknowledgement of request data. If this is
	// enabled, Receive() will acknowledge data automatically, just like in
	// a regular connection.
	virtual void SetAutoAck(bool autoack);

	// Forwarded to wrapped connection object.
	virtual ssize_t Send(string data, int flags = 0);
	virtual string PeerAsText();
	virtual Server* GetServer();
	virtual uint64_t GetLastUse();
	virtual void SetBlocking(bool blocking = true);
	virtual void Shutdown();
	virtual bool IsShutdown();

private:
	Connection* wrapped_;
	const bool owned_;
	const uint64_t max_buffer_size_;
	bool autoack_;
	string buffer_;
};

}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_ACKNOWLEDGEMENTDECORATOR_H */
