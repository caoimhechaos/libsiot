/**
 * Tests for the acknowledgement decorator implementation.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <toolbox/scopedptr.h>

#include "siot/connection.h"
#include "siot/acknowledgementdecorator.h"

namespace toolbox
{
namespace siot
{
namespace testing
{
using ::testing::Return;
using std::string;

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

class AcknowledgementDecoratorTest : public ::testing::Test
{
};

TEST_F(AcknowledgementDecoratorTest, ReadRepeatedly)
{
	MockConnection mc;
	AcknowledgementDecorator ack(&mc, 512);
	string rv;

	EXPECT_CALL(mc, Receive(0, 0))
		.WillOnce(Return("In... "))
		.WillOnce(Return("wait for it... "))
		.WillOnce(Return("credible!"))
		.WillOnce(Return(""));
	EXPECT_CALL(mc, IsEOF())
		.Times(2)
		.WillRepeatedly(Return(true));

	EXPECT_EQ("In... ", ack.Receive());
	EXPECT_EQ("In... wait for it... ", ack.Receive());
	EXPECT_EQ("In... wait for it... credible!", ack.Receive());
	rv = ack.Receive();
	EXPECT_EQ("In... wait for it... credible!", rv);
	EXPECT_FALSE(ack.IsEOF());

	EXPECT_FALSE(ack.Acknowledge(rv.length() + 10));
	EXPECT_TRUE(ack.Acknowledge(rv.length()));
	EXPECT_TRUE(ack.IsEOF());
}

TEST_F(AcknowledgementDecoratorTest, AckLess)
{
	MockConnection mc;
	AcknowledgementDecorator ack(&mc, 512);

	EXPECT_CALL(mc, Receive(0, 0))
		.WillOnce(Return("0123456789"))
		.WillOnce(Return(""));

	EXPECT_EQ("0123456789", ack.Receive());
	EXPECT_TRUE(ack.Acknowledge(5));
	EXPECT_EQ("56789", ack.Receive());
}

}  // namespace testing
}  // namespace siot
}  // namespace toolbox
