#pragma once

#include <process.hpp>
#include <INIReader.h>
#include <kjson.hpp>
#include "types.hpp"

namespace ktube {
inline const std::string get_executable_cwd() {
  char* path = realpath("/proc/self/exe", NULL);
  char* name = basename(path);
  return std::string{path, path + strlen(path) - strlen(name)};
}

inline const std::string GetConfigPath() {
  return get_executable_cwd() + "../" + constants::DEFAULT_CONFIG_PATH;
}

inline INIReader GetConfigReader() {
  return INIReader{GetConfigPath()};
}

static std::vector<Comment> ParseComments(const nlohmann::json& data)
{
  std::vector<Comment> comments{};
  if (!data.is_null() && data.is_object())
  {
    for (const auto& item : data["items"])
      comments.emplace_back(
        Comment{
          .id       = kjson::GetJSONStringValue(     item, "id"),
          .video_id = kjson::GetJSONStringValue(     item["snippet"], "videoId"),
          .text     = kjson::GetJSONStringValue(     item["snippet"]["topLevelComment"]["snippet"],                    "textDisplay"),
          .name     = kjson::GetJSONStringValue(     item["snippet"]["topLevelComment"]["snippet"],                    "authorDisplayName"),
          .channel  = kjson::GetJSONStringValue(     item["snippet"]["topLevelComment"]["snippet"]["authorChannelId"], "value"),
          .likes    = kjson::GetJSONValue<uint32_t>( item["snippet"]["topLevelComment"]["snippet"],                    "likeCount"),
          .time     = kjson::GetJSONStringValue(     item["snippet"]["topLevelComment"]["snippet"],                    "publishedAt"),
        }
      );
  }

  return comments;
}

} // namespace ktube