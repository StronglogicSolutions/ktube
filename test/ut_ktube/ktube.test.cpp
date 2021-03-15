#include "ktube.test.hpp"

TEST(KTubeTest, DISABLED_FetchCommentThreads) {
  ktube::YouTubeDataAPI api{};
  api.init();

  std::vector<ktube::Comment> comments = api.FetchVideoComments(ktube::TEST_VIDEO_ID);

  for (const auto& comment : comments)
    ktube::log(comment);

  EXPECT_FALSE(comments.empty());
}

TEST(KTubeTest, ReplyToComment)
{
  ktube::YouTubeDataAPI api{};
  api.init();

  ktube::Comment comment{};

  comment.text      = "Agreed!!";
  comment.video_id  = ktube::TEST_VIDEO_ID;
  comment.parent_id = ktube::TEST_COMMENT_ID;

  bool result = api.PostCommentReply(comment);

  EXPECT_TRUE(result);
}
