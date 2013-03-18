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

#ifndef INCLUDED_SIOT_SSL_H
#define INCLUDED_SIOT_SSL_H 1

#include <list>
#include <string>

namespace toolbox
{
namespace siot
{
namespace ssl
{
using std::string;
using std::list;

// SSL context for servers
class ServerSSLContext
{
public:
	// Set the path to an SSL certificate and key file. Without this,
	// there won't be an SSL context.
	ServerSSLContext(const string& cert_file, const string& key_file);
	virtual ~ServerSSLContext();

	// Retrieve the path to the certificate file which was associated with
	// this object.
	virtual string GetCertFilePath() const;

	// Retrieve the path to the private key file which was associated with
	// this object.
	virtual string GetPrivateKeyFilePath() const;

	// Adds additional certificates to the CA chain.
	virtual ServerSSLContext* AddChainCertPath(const string& cert_file);

	// Gets the list of all paths of CA certificates from the chain.
	virtual list<string> GetChainCertPaths() const;

	// Set the path to a client CA file. This is required for setting
	// up client certificates.
	virtual ServerSSLContext* SetClientCAFilePath(const string& ca_file);

	// Gets the path to the client CA file.
	virtual string GetClientCAFilePath() const;

private:
	list<string> chain_certs_;
	string client_ca_file_;
	const string cert_file_;
	const string key_file_;
};

}  // namespace ssl
}  // namespace siot
}  // namespace toolbox

#endif /* INCLUDED_SIOT_SSL_H */
