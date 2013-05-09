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

#include "siot/rangereaderdecorator.h"
#include "siot/connection.h"
#include "siot/server.h"

#include <string>

namespace toolbox
{
namespace siot
{
using std::string;

RangeReaderDecorator::RangeReaderDecorator(Connection* wrapped,
		uint64_t max_size, bool own)
: wrapped_(wrapped), owned_(own), max_size_(max_size), offset_(0)
{
}

RangeReaderDecorator::~RangeReaderDecorator()
{
}

string
RangeReaderDecorator::Receive(size_t len, int flags)
{
	size_t toread = len;

	if (offset_ >= max_size_)
		return string();

	if (toread == 0 || toread > max_size_ - offset_)
		toread = max_size_ - offset_;

	string data = wrapped_->Receive(toread, flags);
	if (wrapped_->IsEOF())
		offset_ = max_size_;
	else
		offset_ += data.length();
	return data;
}

bool
RangeReaderDecorator::IsEOF()
{
	if (offset_ >= max_size_)
		return true;
	return wrapped_->IsEOF();
}

ssize_t
RangeReaderDecorator::Send(string data, int flags)
{
	throw ClientConnectionException("read-only",
		       	"Write attempted on read-only connection");
}

void
RangeReaderDecorator::Shutdown()
{
	Deregister();
	// Eat all remaining data.
	while (offset_ < max_size_ && !wrapped_->IsEOF())
	{
		size_t toread = max_size_ - offset_;
		string data = wrapped_->Receive(toread, 0);
		offset_ += data.length();
	}

	// Shut down the connection if requested.
	if (owned_)
		wrapped_->Shutdown();
	delete this;
}

string
RangeReaderDecorator::PeerAsText()
{
	return wrapped_->PeerAsText();
}

Server*
RangeReaderDecorator::GetServer()
{
	return wrapped_->GetServer();
}

uint64_t
RangeReaderDecorator::GetLastUse()
{
	return wrapped_->GetLastUse();
}

void
RangeReaderDecorator::SetBlocking(bool blocking)
{
	wrapped_->SetBlocking(blocking);
}

bool
RangeReaderDecorator::IsShutdown()
{
	return wrapped_->IsShutdown();
}
}  // namespace siot
}  // namespace toolbox
