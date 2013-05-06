/**
 * Tests for the line buffer decorator implementation.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <toolbox/scopedptr.h>

#include "siot/connection.h"
#include "siot/linebufferdecorator.h"

namespace toolbox
{
namespace siot
{
namespace testing
{
using ::testing::Return;

class MockConnection : public Connection
{
public:
	MOCK_METHOD2(Receive, string(size_t maxlen, int flags));
	MOCK_METHOD2(Send, ssize_t(string data, int flags));
	MOCK_METHOD0(PeerAsText, string());
	MOCK_METHOD0(GetServer, Server*());
	MOCK_METHOD0(IsEOF, bool());
	MOCK_METHOD0(GetLastUse, uint64_t());
	MOCK_METHOD1(SetBlocking, void(bool));
	MOCK_METHOD0(Shutdown, void());
};

class LineBufferDecoratorTest : public ::testing::Test
{
};

TEST_F(LineBufferDecoratorTest, ReadJustOneLine)
{
	MockConnection mc;
	LineBufferDecorator lb(&mc);

	EXPECT_CALL(mc, Receive(-1, 0))
		.WillOnce(Return("hello world\n"))
		.WillOnce(Return(""));
	EXPECT_CALL(mc, IsEOF())
		.WillOnce(Return(false))
		.WillOnce(Return(false));

	EXPECT_EQ("hello world\n", lb.Receive());
	EXPECT_EQ("", lb.Receive());
}

TEST_F(LineBufferDecoratorTest, ReadJustOneLineCRLF)
{
	MockConnection mc;
	LineBufferDecorator lb(&mc);

	EXPECT_CALL(mc, Receive(-1, 0))
		.WillOnce(Return("hello world\r\n"))
		.WillOnce(Return(""));
	EXPECT_CALL(mc, IsEOF())
		.WillOnce(Return(false))
		.WillOnce(Return(false));

	EXPECT_EQ("hello world\n", lb.Receive());
	EXPECT_EQ("", lb.Receive());
}

TEST_F(LineBufferDecoratorTest, MultiLineOnePacket)
{
	MockConnection mc;
	LineBufferDecorator lb(&mc);

	EXPECT_CALL(mc, Receive(-1, 0))
		.WillOnce(Return("hello world\nHow's life?\nSomething weird"))
		.WillOnce(Return(" happened to the last line.\n"))
		.WillOnce(Return(""));
	EXPECT_CALL(mc, IsEOF())
		.Times(3)
		.WillRepeatedly(Return(false));

	EXPECT_EQ("hello world\n", lb.Receive());
	EXPECT_EQ("How's life?\n", lb.Receive());
	EXPECT_EQ("Something weird happened to the last line.\n",
			lb.Receive());
	EXPECT_EQ("", lb.Receive());
}

}  // namespace testing
}  // namespace siot
}  // namespace toolbox
