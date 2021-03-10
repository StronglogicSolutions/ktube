#pragma once

#include <string>

#include <vector>
#include <string>
#include <string_view>

namespace ktube {
namespace constants {
// Paths
extern const std::string FOLLOWER_YT_APP;
extern const std::string TRENDS_APP;
extern const std::string FOLLOWER_JSON;
extern const std::string FOLLOWERS_IG_JSON;
extern const std::string YOUTUBE_QUOTA_PATH;

// Config Keys
extern const std::string CREDS_PATH_KEY;
extern const std::string TOKENS_PATH_KEY;
extern const std::string USER_CONFIG_KEY;

// URL Indexes
extern const uint8_t SEARCH_URL_INDEX;
extern const uint8_t VIDEOS_URL_INDEX;
extern const uint8_t LIVE_CHAT_URL_INDEX;
extern const uint8_t GOOGLE_AUTH_URL_INDEX;
extern const uint8_t CHANNELS_URL_INDEX;
extern const uint8_t COMMENT_THREADS_URL_INDEX;
extern const uint8_t COMMENT_POST_URL_INDEX;

// Header Name Indexes
extern const uint8_t ACCEPT_HEADER_INDEX;
extern const uint8_t AUTH_HEADER_INDEX;
extern const uint8_t CONTENT_TYPE_INDEX;

// Header Value Indexes
extern const uint8_t APP_JSON_INDEX;
extern const uint8_t FORM_URL_ENC_INDEX;

// Param Name Indexes
extern const uint8_t PART_INDEX;
extern const uint8_t CHAN_ID_INDEX;
extern const uint8_t EVENT_T_INDEX;
extern const uint8_t TYPE_INDEX;
extern const uint8_t KEY_INDEX;
extern const uint8_t ID_NAME_INDEX;
extern const uint8_t LIVE_CHAT_ID_INDEX;
extern const uint8_t CLIENT_ID_INDEX;
extern const uint8_t CLIENT_SECRET_INDEX;
extern const uint8_t REFRESH_TOKEN_NAME_INDEX;
extern const uint8_t GRANT_TYPE_INDEX;
extern const uint8_t ORDER_INDEX;
extern const uint8_t MAX_RESULT_INDEX;
extern const uint8_t QUERY_INDEX;
extern const uint8_t VIDEO_ID_INDEX;

// Param Value Indexes
extern const uint8_t CHAN_KEY_INDEX;
extern const uint8_t LIVE_EVENT_TYPE_INDEX;
extern const uint8_t COMPLETED_EVENT_TYPE_INDEX;
extern const uint8_t SNIPPET_INDEX;
extern const uint8_t VIDEO_TYPE_INDEX;
extern const uint8_t LIVESTREAM_DETAILS_INDEX;
extern const uint8_t KY_CHAN_KEY_INDEX;
extern const uint8_t STATISTICS_INDEX;
extern const uint8_t ID_VALUE_INDEX;
extern const uint8_t REFRESH_TOKEN_VALUE_INDEX;
extern const uint8_t DATE_VALUE_INDEX;
extern const uint8_t CONTENT_DETAILS_INDEX;
extern const uint8_t VIEW_COUNT_INDEX;
extern const uint8_t SNIPPET_STATS_INDEX;

// URL Indexes
extern const uint8_t SEARCH_URL_INDEX;
extern const uint8_t VIDEOS_URL_INDEX;
extern const uint8_t LIVE_CHAT_URL_INDEX;

// Header Name Indexes
extern const uint8_t ACCEPT_HEADER_INDEX;
extern const uint8_t AUTH_HEADER_INDEX;
extern const uint8_t CONTENT_TYPE_INDEX;

// Header Value Indexes
extern const uint8_t APP_JSON_INDEX;

// Param Name Indexes
extern const uint8_t PART_INDEX;
extern const uint8_t CHAN_ID_INDEX;
extern const uint8_t EVENT_T_INDEX;
extern const uint8_t TYPE_INDEX;
extern const uint8_t KEY_INDEX;
extern const uint8_t ID_INDEX;
extern const uint8_t LIVE_CHAT_ID_INDEX;

// Param Value Indexes
extern const uint8_t LIVE_EVENT_TYPE_INDEX;
extern const uint8_t SNIPPET_INDEX;
extern const uint8_t VIDEO_TYPE_INDEX;
extern const uint8_t LIVESTREAM_DETAILS_INDEX;
extern const uint8_t KY_CHAN_KEY_INDEX;

// Strings
extern const std::vector<std::string> URL_VALUES;

extern const std::vector<std::string> HEADER_NAMES;

extern const std::vector<std::string> HEADER_VALUES;

extern const std::vector<std::string> PARAM_NAMES;

extern const std::vector<std::string> PARAM_VALUES;

extern const std::string E_CHANNEL_ID;
extern const std::string DEFAULT_CONFIG_PATH;
extern const std::string YOUTUBE_KEY;
extern const std::string KTUBE_CONFIG_SECTION;
extern const std::string VERIFY_SSL_KEY;
extern const std::string YOUTUBE_TOKEN_APP;
extern const std::string YOUTUBE_USERNAME;
extern const std::string YOUTUBE_GREET;
extern const std::string YOUTUBE_TEST_MODE;
extern const std::string YOUTUBE_RETRY_MODE;

namespace invitations {
extern const std::string OFFER_TO_INQUIRE;
} // namespace invitations

namespace promotion {
extern const std::string support;
extern const std::string test_support;
} // namespace promotion
// Strings

extern const uint8_t KSTYLEYO_CHANNEL_ID_INDEX;
extern const uint8_t WALKAROUNDWORLD_CHANNEL_ID_INDEX;
extern const uint8_t STRONGLOGICSOLUTIONS_CHANNEL_ID_INDEX;

extern const std::vector<std::string> CHANNEL_IDS;

extern const std::string GOOGLE_CONFIG_SECTION;
extern const std::string REFRESH_TOKEN;
extern const std::string CLIENT_ID;
extern const std::string CLIENT_SECRET;
extern const std::string E_CHANNEL_ID;
extern const std::string DEFAULT_CONFIG_PATH;
extern const std::string YOUTUBE_KEY;
extern const std::string YOUTUBE_CONFIG_SECTION;
extern const std::string YOUTUBE_TOKEN_APP;
extern const std::string YOUTUBE_USERNAME;
extern const std::string YOUTUBE_GREET;
extern const std::string YOUTUBE_TEST_MODE;
extern const std::string YOUTUBE_RETRY_MODE;

inline const std::string VideoParamsFull() {
  return std::string{
    PARAM_VALUES.at(SNIPPET_INDEX) + "," +
    PARAM_VALUES.at(CONTENT_DETAILS_INDEX) + "," +
    PARAM_VALUES.at(STATISTICS_INDEX)
  };
        //  + "," + PARAM_VALUES.at(ID_VALUE_INDEX)
}

/**
* youtube data api constants
*/
namespace youtube {
const uint32_t YOUTUBE_DAILY_QUOTA        = 10000;

const uint8_t  VIDEO_LIST_QUOTA_INDEX     = 0x00;
const uint8_t  CHANNEL_LIST_QUOTA_INDEX   = 0x01;
const uint8_t  COMMENT_LIST_QUOTA_INDEX   = 0x02;
const uint8_t  SEARCH_LIST_QUOTA_INDEX    = 0x03;
const uint8_t  COMMENT_INSERT_QUOTA_INDEX = 0x04;

const std::vector<uint32_t> QUOTA_LIMIT{
  1,
  1,
  1,
  100,
  50
};

const uint8_t YOUTUBE_VIDEO_ID_LENGTH     = 11;
} // namespace youtube
} // namespace constants
} // namespace ktube