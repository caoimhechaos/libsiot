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

#include "siot/linebufferdecorator.h"
#include "siot/connection.h"

#include <string>

namespace toolbox
{
namespace siot
{
using std::string;

LineBufferDecorator::LineBufferDecorator(Connection* wrapped, bool own)
: wrapped_(wrapped), owned_(own)
{
}

LineBufferDecorator::~LineBufferDecorator()
{
	// Ensure we're the only ones operating on the connection.
	Lock();
}

string
LineBufferDecorator::Receive(size_t ignored, int flags)
{
	if (remaining_lines_.size() == 0)
	{
		while (remainder_.rfind("\n") == string::npos &&
				!wrapped_->IsEOF())
		{
			string data = wrapped_->Receive();
			if (data.length() > 0)
				remainder_ += data;
			else
				break;
		}

		string::size_type sz;
		while ((sz = remainder_.find("\n")) != string::npos)
		{
			if (remainder_[sz-1] == '\r')
				remaining_lines_.push_back(
						remainder_.substr(0, sz-1));
			else
				remaining_lines_.push_back(
						remainder_.substr(0, sz));
			remainder_ = remainder_.substr(sz + 1);
		}

		// Still haven't found anything? Then we should give up.
		// Either we're at the end, then IsEOF() will be set, or
		// we'll be more lucky in the next round.
		if (remaining_lines_.size() == 0)
			return "";
	}

	string ret = remaining_lines_.front();
	remaining_lines_.pop_front();
	return ret + "\n";
}

ssize_t
LineBufferDecorator::Send(string data, int flags)
{
	return wrapped_->Send(data, flags);
}

string
LineBufferDecorator::PeerAsText()
{
	return wrapped_->PeerAsText();
}

Server*
LineBufferDecorator::GetServer()
{
	return wrapped_->GetServer();
}

bool
LineBufferDecorator::IsEOF()
{
	return wrapped_->IsEOF() && remaining_lines_.size() == 0;
}

uint64_t
LineBufferDecorator::GetLastUse()
{
	return wrapped_->GetLastUse();
}

void
LineBufferDecorator::SetBlocking(bool blocking)
{
	wrapped_->SetBlocking(blocking);
}

void
LineBufferDecorator::Shutdown()
{
	// We have to deregister ourselves as the client isn't registered.
	Deregister();
	if (owned_)
		wrapped_->Shutdown();
	delete this;
}

bool
LineBufferDecorator::IsShutdown()
{
	return wrapped_->IsShutdown();
}

}  // namespace siot
}  // namespace toolbox
