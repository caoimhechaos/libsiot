/**
 * Tests for the OpenSSL socket connection implementation.
 */

#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread++/threadpool.h>
#include <toolbox/qsingleton.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>

#include "opensslconnection.h"

namespace toolbox
{
namespace siot
{
namespace ssl
{
namespace testing
{
// using threadpp::ClosureThread;
using threadpp::Thread;

class OpenSSLConnectionSetup : public Thread
{
public:
	OpenSSLConnectionSetup(SSL* ssl)
	: ssl_(ssl)
	{
	}

	virtual void Run()
	{
		unsigned long err = SSL_connect(ssl_);
		if (err <= 0)
		{
			std::cerr << ERR_error_string(err, NULL) << std::endl;
			throw ClientConnectionException("SSL_connect:" +
					std::to_string(err),
					ERR_error_string(err, NULL));
		}
	}

private:
	SSL* ssl_;
};

class OpenSSLConnectionTest : public ::testing::Test
{
};

TEST_F(OpenSSLConnectionTest, ReadWrite)
{
	ServerSSLContext* ctx = new ServerSSLContext("test.crt", "test.key");
	const SSL_METHOD* meth = SSLv23_client_method();
	struct sockaddr_storage ssladdr;
	SSL_CTX* ssl_ctx;
	SSL* ssl;
	char buf[16];
	int socks[2];

	memset(&ssladdr, 0, sizeof(struct sockaddr_storage));
	memset(buf, 0, sizeof(buf));

	EXPECT_FALSE(socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
		<< "Error establishing socket pair: " << strerror(errno);

	// We need to use OpenSSL, so make sure it's initialized.
	QSingleton<OpenSSLConfig>::GetInstance();

	ASSERT_NE((SSL_CTX*) 0, ssl_ctx = SSL_CTX_new(meth))
		<< ERR_error_string(ERR_get_error(), NULL);
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
	ASSERT_NE((SSL*) 0, ssl = SSL_new(ssl_ctx))
		<< ERR_error_string(ERR_get_error(), NULL);
	ASSERT_EQ(1, SSL_set_fd(ssl, socks[1]))
		<< ERR_error_string(ERR_get_error(), NULL);

	OpenSSLConnectionSetup setup(ssl);
	setup.Start();

	OpenSSLConnection sslc(0, socks[0], &ssladdr, ctx);
	setup.WaitForFinished();

	EXPECT_EQ(11, sslc.Send("Hey, buddy!"));
	EXPECT_EQ(11, SSL_read(ssl, buf, sizeof(buf)));
	EXPECT_EQ("Hey, buddy!", string(buf));

	EXPECT_EQ(strlen(buf), SSL_write(ssl, buf, strlen(buf)));
	EXPECT_EQ("Hey, buddy!", sslc.Receive());

	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ssl_ctx);
}

}  // namespace testing
}  // namespace ssl
}  // namespace siot
}  // namespace toolbox
