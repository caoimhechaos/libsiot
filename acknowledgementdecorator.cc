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

#include "siot/acknowledgementdecorator.h"
#include "siot/connection.h"
#include "siot/server.h"

#include <string>

namespace toolbox
{
namespace siot
{
using std::string;

AcknowledgementDecorator::AcknowledgementDecorator(
		Connection* wrapped, uint64_t max_buffer_size, bool own)
: wrapped_(wrapped), owned_(own), max_buffer_size_(max_buffer_size)
{
}

AcknowledgementDecorator::~AcknowledgementDecorator()
{
}

string
AcknowledgementDecorator::Receive(size_t ignored, int flags)
{
	string data = wrapped_->Receive(0, flags);
	if (data.length() > 0)
		buffer_ += data;

	if (buffer_.length() > max_buffer_size_)
		throw ClientConnectionException("buffer size exceeded",
				"The specified buffer size has been exceeded");

	return buffer_;
}

bool
AcknowledgementDecorator::Acknowledge(size_t bytes)
{
	if (bytes > buffer_.length())
		return false;
	buffer_ = buffer_.substr(bytes);
	return true;
}

ssize_t
AcknowledgementDecorator::Send(string data, int flags)
{
	return wrapped_->Send(data, flags);
}

string
AcknowledgementDecorator::PeerAsText()
{
	return wrapped_->PeerAsText();
}

Server*
AcknowledgementDecorator::GetServer()
{
	return wrapped_->GetServer();
}

bool
AcknowledgementDecorator::IsEOF()
{
	return wrapped_->IsEOF() && buffer_.length() == 0;
}

uint64_t
AcknowledgementDecorator::GetLastUse()
{
	return wrapped_->GetLastUse();
}

void
AcknowledgementDecorator::SetBlocking(bool blocking)
{
	wrapped_->SetBlocking(blocking);
}

void
AcknowledgementDecorator::Shutdown()
{
	// We have to deregister ourselves as the client isn't registered.
	Deregister();
	if (owned_)
		wrapped_->Shutdown();
	delete this;
}

bool
AcknowledgementDecorator::IsShutdown()
{
	return wrapped_->IsShutdown();
}

}  // namespace siot
}  // namespace toolbox
