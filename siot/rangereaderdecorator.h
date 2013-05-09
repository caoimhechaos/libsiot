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

#ifndef INCLUDED_SIOT_RANGEREADERDECORATOR_H
#define INCLUDED_SIOT_RANGEREADERDECORATOR_H 1

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
 * Reads up to a specified length of bytes from a given connection handle.
 * After reading the full length, the handle will act like it has reached
 * the end of the file. Upon calling Shutdown(), this will advance until
 * the specified length has been reached, so that the part of the buffer
 * will be considered as read.
 *
 * Calls to Send() will be rejected as this is most commonly used to
 * subdelegate reading specified ranges from buffers.
 *
 * By default, this takes ownership of the connection handle. If you don't
 * want this behavior, set the own parameter to false.
 */
class RangeReaderDecorator : public Connection
{
public:
	// Creates a new range reader around the connection "wrapped" with
	// a maximum of "max_size" bytes to read.. If "own" is true (which is
	// the default), this will take ownership of "wrapped".
	explicit RangeReaderDecorator(Connection* wrapped,
			uint64_t max_size, bool own = true);
	virtual ~RangeReaderDecorator();

	// Reads up to "len" bytes from the underlying connection. If
	// max_size is reached before "len" bytes can be read, only the
	// bytes remaining to max_size will be returned and IsEOF will
	// be set to true.
	virtual string Receive(size_t len = 0, int flags = 0);

	// This will return true when the end-of-line indicator is set on the
	// underlying connection object or max_size bytes have been read from
	// the handle.
	virtual bool IsEOF();

	// Calling Send() will always fail on RangeReaderDecorators.
	virtual ssize_t Send(string data, int flags = 0);

	// Advances until the end of max_size, then destroys the
	// RangeReaderDecorator and, if own was set to true, tells the wrapped
	// connection to shut down as well.
	virtual void Shutdown();

	// Forwarded to wrapped connection object.
	virtual string PeerAsText();
	virtual Server* GetServer();
	virtual uint64_t GetLastUse();
	virtual void SetBlocking(bool blocking = true);
	virtual bool IsShutdown();

private:
	Connection* wrapped_;
	const bool owned_;
	const uint64_t max_size_;
	uint64_t offset_;
};

}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_RANGEREADERDECORATOR_H */
