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

#include <list>
#include <string>

#include "siot/ssl.h"

namespace toolbox
{
namespace siot
{
namespace ssl
{
using std::string;
using std::list;

ServerSSLContext::ServerSSLContext(
		const string& cert_file, const string& key_file)
: cert_file_(cert_file), key_file_(key_file)
{
}

ServerSSLContext::~ServerSSLContext()
{
}

string
ServerSSLContext::GetCertFilePath() const
{
	return cert_file_;
}

string
ServerSSLContext::GetPrivateKeyFilePath() const
{
	return key_file_;
}

ServerSSLContext*
ServerSSLContext::AddChainCertPath(const string& cert_file)
{
	for (string path : chain_certs_)
		if (path == cert_file)
			return this;

	chain_certs_.push_back(cert_file);
	return this;
}

list<string>
ServerSSLContext::GetChainCertPaths() const
{
	return chain_certs_;
}

ServerSSLContext*
ServerSSLContext::SetClientCAFilePath(const string& ca_file)
{
	client_ca_file_ = ca_file;
	return this;
}

string
ServerSSLContext::GetClientCAFilePath() const
{
	return client_ca_file_;
}
}  // namespace ssl
}  // namespace siot
}  // namespace toolbox
