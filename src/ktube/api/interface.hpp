#pragma once

#include "ktube/common/types.hpp"

namespace ktube {
class SecureAPI {
public:
virtual ~SecureAPI() {}

virtual bool is_authenticated()                 = 0;
virtual bool init(const bool fetch_fresh_token) = 0;
};

class VideoAPI {
public:
virtual ~VideoAPI() {}

virtual std::vector<VideoInfo>   get_videos() = 0;
virtual bool                     has_videos() = 0;
virtual bool                     fetch_channel_data() = 0;
virtual std::vector<ChannelInfo> fetch_channel_info(std::string id_string) = 0;
virtual bool                     fetch_channel_videos() = 0;
virtual std::vector<VideoStats>  fetch_video_stats(std::string id_string) = 0;
virtual std::vector<ChannelInfo> fetch_youtube_stats() = 0;
virtual std::vector<VideoInfo>   fetch_rival_videos(VideoInfo video) = 0;
virtual std::vector<ChannelInfo> find_similar_videos(VideoInfo video) = 0;
virtual std::vector<GoogleTrend> fetch_google_trends(std::vector<std::string> terms) = 0;
virtual std::vector<TermInfo>    fetch_term_info(std::vector<std::string> terms) = 0;
virtual std::vector<VideoInfo>   fetch_videos_by_terms(std::vector<std::string> terms) = 0;
};

class CommentAPI {
public:
virtual ~CommentAPI() {}
virtual std::vector<Comment>     FetchVideoComments(const std::string& id) = 0;
};

class LiveAPI {
public:
virtual ~LiveAPI() {}
virtual std::string              FetchLiveVideoID()  = 0;
virtual bool                     FetchLiveDetails()  = 0;
virtual std::string              FetchChatMessages() = 0;
};
} // namespace ktube
