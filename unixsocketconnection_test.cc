/**
 * Tests for the UNIX socket connection implementation.
 */

#include <glog/logging.h>
#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "unixsocketconnection.h"

namespace toolbox
{
namespace siot
{
namespace testing
{
class UnixSocketConnectionTest : public ::testing::Test
{
};

TEST_F(UnixSocketConnectionTest, ReadWrite)
{
	struct sockaddr_storage oneaddr, twoaddr;
	int socks[2];

	memset(&oneaddr, 0, sizeof(struct sockaddr_storage));
	memset(&twoaddr, 0, sizeof(struct sockaddr_storage));

	EXPECT_FALSE(socketpair(AF_UNIX, SOCK_STREAM, 0, socks))
		<< "Error establishing socket pair: " << strerror(errno);

	UNIXSocketConnection one(0, socks[0], &oneaddr);
	UNIXSocketConnection two(0, socks[1], &twoaddr);

	EXPECT_EQ(11, one.Send("Hey, buddy!"));
	EXPECT_EQ("Hey, buddy!", two.Receive());

	EXPECT_EQ(11, two.Send("Hey, buddy!"));
	EXPECT_EQ("Hey, buddy!", one.Receive());
}

}  // namespace testing
}  // namespace siot
}  // namespace toolbox
