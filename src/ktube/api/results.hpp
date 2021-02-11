#pragma once

#include <process.hpp>

#include "ktube/api/analysis/html.hpp"
#include "ktube/common/constants.hpp"
#include "ktube/common/types.hpp"

namespace ktube {
class ResultInterface {
public:
virtual std::string to_string() = 0;
};

class TrendsJSONResult : public ResultInterface {
public:
TrendsJSONResult(std::string data)
: m_data(data) {}

virtual std::string to_string() override {
  return m_data;
}

std::vector<GoogleTrend> get_result() {
  using json = nlohmann::json;
  json parsed = json::parse(m_data);

  std::vector<GoogleTrend> result{};

  if (!parsed.is_null() && parsed.is_object()) {
    auto items = parsed["results"];

    if (!items.is_null() && items.is_array()) {
      for (const auto &item : items) {
        result.emplace_back(
          GoogleTrend{
            .term = SanitizeJSON(item["term"].dump()),
            .value = std::stoi(item["lastScore"].dump())
          }
        );
      }
    }
  }

  return result;
}

private:
std::string m_data;
};

class FollowerResultInterface : public ResultInterface {
using counts = std::vector<FollowerCount>;
public:
virtual ~FollowerResultInterface() {}

virtual bool read(std::string s) = 0;

virtual std::string to_string() override {
  return counts_to_html(m_counts);
}

protected:
counts m_counts;
};

class InstagramFollowResult : public FollowerResultInterface {
public:
virtual ~InstagramFollowResult() override {}

virtual bool read(std::string s) override {
using json = nlohmann::json;
using namespace constants;
  json r_json = json::parse(s, nullptr, JSON_PARSE_NO_THROW);
  json f_json = json::parse(ReadFromFile(get_executable_cwd() + FOLLOWERS_IG_JSON), nullptr, JSON_PARSE_NO_THROW);
  json s_json{};

  bool file_valid = (!f_json.is_null() && f_json.is_object());

  if (!r_json.is_null() && r_json.is_array()) {
    std::string as_string = r_json.dump();
    json        f_instagram   = (file_valid && f_json.contains("instagram")) ? f_json["instagram"] : json{};
    bool        found_ig_file = (!f_instagram.is_null() && f_instagram.is_object());
    std::string current_time  = get_simple_datetime();

    for (const auto& item : r_json.items()) {
      json ig_result = item.value();
      std::string username       = GetJSONStringValue(ig_result, "username");
      std::string value          = std::to_string(GetJSONValue<uint32_t>(ig_result, "follower_count"));
      std::string previous_date {};
      uint32_t    previous_count{};

      s_json["instagram"][username]["value"] = value;
      s_json["instagram"][username]["date"]  = current_time;

      if (f_instagram.contains(username) && !f_instagram[username].is_null()) {
        previous_date  = f_instagram[username]["date"];
        previous_count = std::stoi(SanitizeJSON(f_instagram[username]["value"].dump()));
      }

      m_counts.push_back(
        FollowerCount{
          .name     = username,
          .platform = "instagram",
          .value    = value,
          .time     = current_time,
          .delta_t  = datetime_delta_string(current_time, (previous_date.empty()) ? current_time : previous_date),
          .delta_v  = std::to_string(std::stoi(value) - previous_count),
        }
      );
    }

    SaveToFile(s_json.dump(), get_executable_cwd() + FOLLOWERS_IG_JSON);

    return true;
  }

  return false;
}


};
class YouTubeFollowResult : public FollowerResultInterface {
public:
virtual ~YouTubeFollowResult() override {}

/**
 * read
 */
virtual bool read(std::string s) override {
  using json = nlohmann::json;
  using namespace constants;

  json r_json = json::parse(s, nullptr, false);
  json f_json = json::parse(ReadFromFile(get_executable_cwd() + FOLLOWER_JSON), nullptr, false);
  json s_json{};

  bool file_valid = (!f_json.is_null() && f_json.is_object());

  if (!r_json.is_null() && r_json.is_object()) {
    json instagram   = r_json["instagram"];
    json youtube     = r_json["youtube"];
    json f_instagram = (file_valid && f_json.contains("instagram")) ? f_json["instagram"] : json{};
    json f_youtube   = (file_valid && f_json.contains("youtube"))   ? f_json["youtube"]   : json{};

    bool found_ig_file = (!f_instagram.is_null() && f_instagram.is_object());
    bool found_yt_file = (!f_youtube.is_null()   && f_youtube.is_object());

    std::string current_time = get_simple_datetime();

    if (!instagram.is_null() && instagram.is_object()) {
      for (const auto& it : instagram.items()) {
        std::string name           = it.key();
        std::string value          = it.value()["value"].dump();
        std::string previous_date {};
        uint32_t    previous_count{};

        s_json["instagram"][name]["value"] = value;
        s_json["instagram"][name]["date"]  = current_time;

        if (f_instagram.contains(name) && !f_instagram[name].is_null()) {
          previous_date  = f_instagram[name]["date"];
          previous_count = std::stoi(SanitizeJSON(f_instagram[name]["value"].dump()));
        }

        m_counts.push_back(
          FollowerCount{
            .name     = it.key(),
            .platform = "instagram",
            .value    = value,
            .time     = current_time,
            .delta_t  = datetime_delta_string(current_time, (previous_date.empty()) ? current_time : previous_date),
            .delta_v  = std::to_string(std::stoi(value) - previous_count),
          }
        );
      }
    }

    if (!youtube.is_null() && youtube.is_object()) {
      for (const auto& it : youtube.items()) {
        std::string name           = it.key();
        std::string value          = SanitizeJSON(it.value()["value"].dump());
        std::string previous_date {};
        uint32_t    previous_count{};

        s_json["youtube"][name]["value"] = value;
        s_json["youtube"][name]["date"]  = current_time;

        if (f_youtube.contains(name) && !f_youtube[name].is_null()) {
          previous_date  = f_youtube[name]["date"];
          previous_count = std::stoi(SanitizeJSON(f_youtube[name]["value"].dump()));
        }

        m_counts.push_back(
          FollowerCount{
            .name     = name,
            .platform = "youtube",
            .value    = value,
            .time     = current_time,
            .delta_t  = datetime_delta_string(current_time, (previous_date.empty()) ? current_time : previous_date),
            .delta_v  = std::to_string(std::stoi(value) - previous_count),
          }
        );
      }
    }

    SaveToFile(s_json.dump(), get_executable_cwd() + FOLLOWER_JSON);

    return true;
  }
  return false;
}

};
} // namespace ktube
