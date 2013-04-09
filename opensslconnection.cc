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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <time.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <thread++/mutex.h>
#include <toolbox/qsingleton.h>
#include <toolbox/scopedptr.h>

#include "opensslconnection.h"

namespace toolbox
{
namespace siot
{
namespace ssl
{
using threadpp::Mutex;
using threadpp::MutexLock;

OpenSSLConfig::OpenSSLConfig()
{
	CRYPTO_malloc_init();
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();
}

OpenSSLConnection::OpenSSLConnection(Server* srv, int socketid,                            
		struct sockaddr_storage* peer,
		const ServerSSLContext* context)
: UNIXSocketConnection(srv, socketid, peer),
	openssl_cfg_(QSingleton<OpenSSLConfig>::GetInstance()),
	blocking_(true), last_use_(time(NULL)), ssl_mtx_(Mutex::Create())
{
	const SSL_METHOD* meth = SSLv23_server_method();
	int ret;

	ssl_ctx_ = SSL_CTX_new(meth);

	if ((ret = SSL_CTX_use_certificate_file(ssl_ctx_,
				context->GetCertFilePath().c_str(),
				SSL_FILETYPE_PEM)) <= 0)
		throw ClientConnectionException("SSL_CTX_use_certificate_file:"
				+ std::to_string(ret),
				ERR_error_string(ret, NULL));

	if ((ret = SSL_CTX_use_PrivateKey_file(ssl_ctx_,
				context->GetPrivateKeyFilePath().c_str(),
				SSL_FILETYPE_PEM)) <= 0)
		throw ClientConnectionException("SSL_CTX_use_PrivateKey_file:"
				+ std::to_string(ret),
				ERR_error_string(ret, NULL));

	ssl_handle_ = SSL_new(ssl_ctx_);
	if (!ssl_handle_)
	{
		ret = ERR_get_error();
		throw ClientConnectionException("SSL_set_fd:"
				+ std::to_string(ret),
				ERR_error_string(ret, NULL));
	}

	if ((ret = SSL_set_fd(ssl_handle_, socketid)) <= 0)
		throw ClientConnectionException("SSL_set_fd:"
				+ std::to_string(ret),
				ERR_error_string(ret, NULL));

	if ((ret = SSL_accept(ssl_handle_)) <= 0)
		throw ClientConnectionException("SSL_accept:"
				+ std::to_string(ret),
				ERR_error_string(ret, NULL));
}

OpenSSLConnection::~OpenSSLConnection()
{
	Shutdown();
}

void
OpenSSLConnection::Shutdown()
{
	MutexLock l(ssl_mtx_);
	int is_down = SSL_get_shutdown(ssl_handle_);
	if (is_down >= 0)
		SSL_shutdown(ssl_handle_);
	SSL_free(ssl_handle_);
	SSL_CTX_free(ssl_ctx_);
}

string
OpenSSLConnection::Receive(size_t maxlen, int flags)
{
	MutexLock l(ssl_mtx_);
	size_t len = SSL_pending(ssl_handle_);
	int blen;
	last_use_ = time(NULL);

	if (len == 0 && !blocking_)
		return string();
	if (len <= 0)
		len = maxlen <= 0 || maxlen > 65536 ? 65536 : maxlen;
	if (maxlen > 0 && len > maxlen)
		len = maxlen;

	const ScopedPtr<char> buf(new char[len]);
	if ((blen = SSL_read(ssl_handle_, buf.Get(), len)) <= 0)
	{
		unsigned long errv = SSL_get_error(ssl_handle_, blen);
		if (errv == SSL_ERROR_SYSCALL)
			throw ClientConnectionException("SSL_read:"
					+ string(strerror(errno)),
					strerror(errno));
		throw ClientConnectionException("SSL_read:"
				+ std::to_string(errv),
				ERR_error_string(errv, NULL));
	}

	return string(buf.Get(), blen);
}

ssize_t
OpenSSLConnection::Send(string data, int flags)
{
	MutexLock l(ssl_mtx_);
	int ret = SSL_write(ssl_handle_, data.data(), data.size());
	last_use_ = time(NULL);

	if (ret <= 0)
	{
		unsigned long errv = SSL_get_error(ssl_handle_, ret);
		if (errv == SSL_ERROR_SYSCALL)
			throw ClientConnectionException("SSL_read:"
					+ string(strerror(errno)),
					strerror(errno));
		throw ClientConnectionException("SSL_write:"
				+ std::to_string(errv),
				ERR_error_string(errv, NULL));
	}

	return ssize_t(ret);
}

uint64_t
OpenSSLConnection::GetLastUse()
{
	return last_use_;
}

void
OpenSSLConnection::SetBlocking(bool blocking)
{
	blocking_ = blocking;
}
}  // namespace ssl
}  // namespace siot
}  // namespace toolbox
