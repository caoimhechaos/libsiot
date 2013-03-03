/**
 * Tests for the server class.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <iostream>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <clib/clib.h>

#include "siot/server.h"

namespace toolbox
{
namespace siot
{
namespace testing
{
using ::testing::Return;
using ::testing::A;

using threadpp::ClosureThread;
using google::protobuf::NewCallback;

class MockConnectionCallback : public ConnectionCallback
{
public:
	MOCK_METHOD1(ConnectionEstablished, void(Connection* conn));
	MOCK_METHOD1(DataReady, void(Connection* conn));
	MOCK_METHOD1(ConnectionTerminated, void(Connection* conn));
};

ACTION(CloseConnection) {
	arg0->Receive();
	arg0->Send("Yeah\n", 0);
	arg0->GetServer()->Shutdown();
}

class ServerTest : public ::testing::Test
{
};

TEST_F(ServerTest, StartUpShutDown)
{
	int fake_argc = 0;
	char** fake_argv = { 0 };
	::testing::InitGoogleMock(&fake_argc, fake_argv);
	MockConnectionCallback* cb = new MockConnectionCallback();
	ScopedPtr<Server> srv(0);

	ASSERT_NO_THROW(srv.Reset(new Server("[::1]:12345", cb, 1)));

	ClosureThread ct(NewCallback(srv.Get(), &Server::Listen));
	ct.Start();
	srv->Shutdown();
	ct.WaitForFinished();
}

TEST_F(ServerTest, SystemTest)
{
	struct addrinfo *info;
	char buf[5];
	int sock;
	int fake_argc = 0;
	char** fake_argv = { 0 };
	::testing::InitGoogleMock(&fake_argc, fake_argv);
	ScopedPtr<Server> srv(0);
	MockConnectionCallback* cb = new MockConnectionCallback();

	ASSERT_NO_THROW(srv.Reset(new Server("[::1]:12346", cb, 1)));

	EXPECT_CALL(*cb, ConnectionEstablished(A<Connection*>()))
		.WillOnce(Return());
	EXPECT_CALL(*cb, DataReady(A<Connection*>()))
		.WillOnce(CloseConnection());
	EXPECT_CALL(*cb, ConnectionTerminated(A<Connection*>()))
		.WillOnce(Return());

	ClosureThread ct(NewCallback(srv.Get(), &Server::Listen));
	ct.Start();

	EXPECT_NE(-1, sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP))
		<< "Error creating socket: " << strerror(errno);

	EXPECT_EQ(0, c_str2addrinfo("[::1]:12346", &info))
		<< "Error converting to addrinfo: " << strerror(errno);
	EXPECT_EQ(0, c_connect2addrinfo(sock, info))
		<< "Error connecting: " << strerror(errno);
	freeaddrinfo(info);

	EXPECT_EQ(12, send(sock, "Hello World\n", 12, 0))
		<< "Error sending: " << strerror(errno);
	EXPECT_EQ(5, recv(sock, buf, 5, 0))
		<< "Error receiving: " << strerror(errno);
	EXPECT_EQ("Yeah\n", string(buf, 5));

	EXPECT_EQ(0, shutdown(sock, SHUT_RDWR))
		<< "Error shutting down: " << strerror(errno);
	EXPECT_EQ(0, close(sock))
		<< "Error closing socket: " << strerror(errno);

	ct.WaitForFinished();
}

}  // namespace testing
}  // namespace siot
}  // namespace toolbox
