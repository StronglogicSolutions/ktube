#include "ktube.test.hpp"

TEST(KTubeTest, DISABLED_FetchCommentThreads) {
  ktube::YouTubeDataAPI api{};
  api.init();

  std::vector<ktube::Comment> comments = api.FetchVideoComments(ktube::TEST_VIDEO_ID);

  for (const auto& comment : comments)
    ktube::log(comment);

  EXPECT_FALSE(comments.empty());
}

TEST(KTubeTest, DISABLED_ReplyToComment)
{
  ktube::YouTubeDataAPI api{};
  api.init();

  ktube::Comment comment = ktube::Comment::Create("Agreed!!");
  comment.video_id  = ktube::TEST_VIDEO_ID;
  comment.parent_id = ktube::TEST_COMMENT_ID;

  bool result = api.PostCommentReply(comment).empty();

  EXPECT_TRUE(result);
}

TEST(KTubeTest, DISABLED_TopLevelComment)
{
  ktube::YouTubeDataAPI api{};
  api.init();

  ktube::Comment comment = ktube::Comment::Create("Top level comment, son");
  comment.video_id  = ktube::TEST_VIDEO_ID;

  bool result = api.PostComment(comment).empty();

  EXPECT_TRUE(result);
}



TEST(KTubeTest, FindAndCommentOnVideo)
{
  using namespace ktube;
  using Videos = std::vector<Video>;
  std::string reply_text{"외국인과 언어교환하고 싶은 분은 여기로 방문해 주세요 👉 https://discord.gg/j5Rjhk96"};

  bool result{false};
  bool reply_result{false};

  YouTubeDataAPI api{};

  api.init();

  for (const auto& video : api.fetch_rival_videos(Video::CreateFromTags("영어 공부", "영어 수업")))
  {
    Comment comment  = Comment::Create(reply_text);
    comment.video_id = video.id;

    log(comment);

    result = !(api.PostComment(comment).empty());

    if (result)
    {
      auto comments = api.FetchVideoComments(video.id);
      if (!comments.empty())
      {
        Comment reply_comment   = Comment::Create(reply_text);
        reply_comment.parent_id = comments.front().id;
        reply_comment.video_id  = video.id;

        reply_result = !(api.PostCommentReply(reply_comment).empty());
      }
      break;
    }
  }

  EXPECT_TRUE(result);
  EXPECT_TRUE(reply_result);
}