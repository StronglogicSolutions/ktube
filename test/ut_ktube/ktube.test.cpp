#include "ktube.test.hpp"

TEST(KTubeTest, FetchCommentThreadsTest) {
  const std::string TEST_VIDEO_ID{"6_IGHMSsdD0"};

  ktube::YouTubeDataAPI api{};

  api.init();

  std::vector<ktube::Comment> comments = api.FetchVideoComments(TEST_VIDEO_ID);


  EXPECT_FALSE(comments.empty());
}
