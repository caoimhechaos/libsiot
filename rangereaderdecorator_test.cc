/**
 * Tests for the range reader decorator implementation.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <toolbox/scopedptr.h>

#include "siot/connection.h"
#include "siot/rangereaderdecorator.h"

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

class RangeReaderDecoratorTest : public ::testing::Test
{
};

TEST_F(RangeReaderDecoratorTest, ReadBounded)
{
	MockConnection mc;
	RangeReaderDecorator rr(&mc, 12);

	EXPECT_CALL(mc, Receive(12, 0))
		.WillOnce(Return("hello world\n"));
	EXPECT_CALL(mc, IsEOF())
		.Times(2)
		.WillRepeatedly(Return(false));

	EXPECT_FALSE(rr.IsEOF());
	EXPECT_EQ("hello world\n", rr.Receive());
	EXPECT_TRUE(rr.IsEOF());
	EXPECT_EQ("", rr.Receive());
	EXPECT_TRUE(rr.IsEOF());
}

TEST_F(RangeReaderDecoratorTest, ReadPartial)
{
	MockConnection mc;
	RangeReaderDecorator* rr = new RangeReaderDecorator(&mc, 15);

	EXPECT_CALL(mc, Receive(15, 0))
		.WillOnce(Return("hello world\n"));
	EXPECT_CALL(mc, Receive(3, 0))
		.WillOnce(Return("foo"));
	EXPECT_CALL(mc, IsEOF())
		.Times(2)
		.WillRepeatedly(Return(false));
	EXPECT_CALL(mc, GetServer())
		.WillOnce(Return((toolbox::siot::Server*) 0));
	EXPECT_CALL(mc, Shutdown())
		.Times(1);

	EXPECT_EQ("hello world\n", rr->Receive());
	rr->Shutdown();
}

TEST_F(RangeReaderDecoratorTest, ReadUnbounded)
{
	MockConnection mc;
	RangeReaderDecorator rr(&mc, 15);

	EXPECT_CALL(mc, Receive(15, 0))
		.WillOnce(Return("hello world\n"));
	EXPECT_CALL(mc, Receive(3, 0))
		.WillOnce(Return(""));
	EXPECT_CALL(mc, IsEOF())
		.WillOnce(Return(false))
		.WillOnce(Return(true));

	EXPECT_EQ("hello world\n", rr.Receive());
	EXPECT_EQ("", rr.Receive());
}

TEST_F(RangeReaderDecoratorTest, ReadPartialUnbounded)
{
	MockConnection mc;
	RangeReaderDecorator* rr = new RangeReaderDecorator(&mc, 20);

	EXPECT_CALL(mc, Receive(20, 0))
		.WillOnce(Return("hello world\n"));
	EXPECT_CALL(mc, Receive(8, 0))
		.WillOnce(Return("foo"));
	EXPECT_CALL(mc, IsEOF())
		.WillOnce(Return(false))
		.WillOnce(Return(false))
		.WillOnce(Return(true));
	EXPECT_CALL(mc, GetServer())
		.WillOnce(Return((toolbox::siot::Server*) 0));
	EXPECT_CALL(mc, Shutdown())
		.Times(1);

	EXPECT_EQ("hello world\n", rr->Receive());
	rr->Shutdown();
}

}  // namespace testing
}  // namespace siot
}  // namespace toolbox
