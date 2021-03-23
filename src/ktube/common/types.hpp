#pragma once

#include <unordered_map>
#include <vector>
#include <map>
#include <string>
#include <type_traits>

#include "nlp/types.hpp"
#include "constants.hpp"
#include "util.hpp"

namespace ktube {
/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░░ STRUCTS ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/

struct Comment {
public:
static Comment Create(const std::string& comment_text)
{
  Comment comment;
  comment.text = comment_text;
  return comment;
}

std::string id;
std::string video_id;
std::string text;
std::string name;
std::string channel;
uint32_t    likes;
std::string time;
// uint32_t totalReplyCount
// std::string updatedAt ||
// std::string authorProfileImageUrl
std::string parent_id;

const std::string postdata(const bool reply = true) const
{
  nlohmann::json payload{};
  payload["snippet"]["videoId"]      = video_id;

  if (reply)
  {
    payload["snippet"]["textOriginal"] = text;
    payload["snippet"]["parentId"]     = parent_id;
  }
  else
    payload["snippet"]["topLevelComment"]["snippet"]["textOriginal"] = text;

  if (!channel.empty())
    payload["snippet"]["channelId"] = channel;

  return payload.dump();
}

friend std::ostream& operator<<(std::ostream& o, const Comment& c)
{
  o << "ID: "       << c.id                    << "\n" <<
       "VIDEO_ID: " << c.video_id              << "\n" <<
       "TEXT: "     << c.text                  << "\n" <<
       "NAME: "     << c.name                  << "\n" <<
       "CHANNEL: "  << c.channel               << "\n" <<
       "LIKES: "    << std::to_string(c.likes) << "\n" <<
       "TIME: "     << c.time                  << std::endl;

  return o;
}
};

struct FollowerCount {
std::string name;
std::string platform;
std::string value;
std::string time;
std::string delta_t;
std::string delta_v;
};

struct GoogleTrend {
std::string term;
int         value;
};

struct TermInfo {
  std::string term;
  int         value;
};


struct VideoStats {
std::string              views;
std::string              likes;
std::string              dislikes;
std::string              comments;
std::vector<std::string> keywords;
double                   view_score;
double                   like_score;
double                   dislike_score;
double                   comment_score;
std::vector<GoogleTrend> trends;
double                   keyword_score;
};

/**
 * VideoInfo struct
 *
 * TODO: consider wrapping as a class with an ID
 */
struct Video {
std::string              channel_id;
std::string              id;
std::string              title;
std::string              description;
std::string              datetime;
std::string              time;
std::string              url;
VideoStats               stats;

template <typename... T>
static Video CreateFromTags(T&&... keywords)
{
  Video video{};
  video.stats.keywords = std::vector<std::string>{keywords...};
  return video;
}

/**
 * get_primary_keywords
 *
 * @returns [out] {std::vector<std::string>} up to 3 keywords
 */
std::vector<std::string> get_primary_keywords() {
  return std::vector<std::string>{
    stats.keywords.cbegin(),
    stats.keywords.size() > 2 ?
      stats.keywords.cbegin() + 3 :
      stats.keywords.cbegin() + stats.keywords.size()
    };
}

friend std::ostream &operator<<(std::ostream& o, const Video& v) {
  o << "Channel ID:  " << v.channel_id     << "\n" <<
       "Video ID:    " << v.id             << "\n" <<
       "Title:       " << v.title          << "\n" <<
       "Description: " << v.description    << "\n" <<
       "Datetime:    " << v.datetime       << "\n" <<
       "URL:         " << v.url            << "\n" <<

       "STATISTICS\n" <<

       "➝ Views:         " << v.stats.views      << "\n" <<
       "➝ Likes:         " << v.stats.likes      << "\n" <<
       "➝ Dislikes:      " << v.stats.dislikes   << "\n" <<
       "➝ Comments:      " << v.stats.comments   << "\n" <<
       "➝ View Score:    " << v.stats.view_score << "\n" <<
       "➝ Like Score:    " << v.stats.like_score << "\n" <<
       "➝ Dislike Score: " << v.stats.dislike_score << "\n" <<
       "➝ Comment Score: " << v.stats.comment_score << "\n";

  if (!v.stats.trends.empty()) {
    o << "KEYWORDS\n";
    for (const auto& trend : v.stats.trends)
      o << "➤ Term:  " << trend.term  << "\n" <<
           "  Value: " << trend.value << "\n";
  }

  return o;
}
}; // struct VideoInfo

struct ChannelStats {
std::string views;
std::string subscribers;
std::string videos;
};

struct ChannelInfo {
std::string            name;
std::string            description;
std::string            created;
std::string            thumb_url;
ChannelStats           stats;
std::string            id;
std::vector<Video> videos;

friend std::ostream &operator<<(std::ostream& o, const ChannelInfo& c) {
  o << "ID:          " << c.id     << "\n" <<
       "Name:        " << c.name             << "\n" <<
       "Description: " << c.description          << "\n" <<
       "Created:     " << c.created    << "\n" <<
       "Thumbnail:   " << c.thumb_url       << "\n" <<

       "STATISTICS\n" <<

       "➝ Views:       " << c.stats.views      << "\n" <<
       "➝ Subscribers: " << c.stats.subscribers      << "\n" <<
       "➝ Videos:      " << c.stats.videos   << "\n";

  return o;
}
};

using namespace conversation;

struct VideoDetails {
  std::string id;
  std::string chat_id;
  std::string title;
  std::string description;
  std::string channel_title;
  std::string channel_id;
  std::string url;
  std::string thumbnail;
};

struct LiveMessage {
  std::string        timestamp;
  std::string        author;
  std::string        text;
  std::vector<Token> tokens;
};

struct UserInteraction {
  std::string id;
  bool        greeted;
  bool        promoted;
  bool        probed;
  bool        location;
};

enum Interaction {
  greeting      = 0x00,
  promotion     = 0x01,
  probing       = 0x02,
  location_ask  = 0x03
};

using LiveMessages = std::vector<LiveMessage>;
using Chat         = std::pair<std::string, LiveMessages>;
using LiveChatMap  = std::map<std::string, LiveMessages>;
using ActivityMap  = std::map<std::string, UserInteraction>;
using LocationMap  = std::map<std::string, bool>;
using PersonMap    = LocationMap;
using OrgMap       = LocationMap;

}
