#include "youtube.hpp"


namespace ktube {
const std::string CreateLocationResponse(std::string location) {
  return std::string{
    "Cool to see someone from " + location + ". How is life treating you there?"
  };
}

const std::string CreatePersonResponse(std::string name) {
  return std::string{
    "Hello, " + name + ". How are you?"
  };
}

const std::string CreateOrganizationResponse(std::string name) {
  return std::string{
    "I will need to familiarize myself better with " + name
  };
}

const std::string CreatePromoteResponse(bool test_mode) {
  if (test_mode) {
    return constants::promotion::support;
  }

  return constants::promotion::test_support;
}

std::string to_youtube_url(const std::string& id)
{
  return "https://www.youtube.com/watch?v=" + id;
}




/**
 * @brief Construct a new YouTubeDataAPIAPI object
 *
 * @constructor
 *
 * @returns [out] {API}
 *
 * Notes:
 * - Sets the channels upon which API functions will be performed
 *
 */
YouTubeDataAPI::YouTubeDataAPI ()
: m_channel_ids{
  constants::CHANNEL_IDS.at(constants::KSTYLEYO_CHANNEL_ID_INDEX),
  constants::CHANNEL_IDS.at(constants::WALKAROUNDWORLD_CHANNEL_ID_INDEX)
  },
  m_greet_on_entry{false},
  m_test_mode{false},
  m_retry_mode{false} {
  INIReader reader{constants::DEFAULT_CONFIG_PATH};

  if (reader.ParseError() < 0) {
    log("Error loading config");
  }

  auto username = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_USERNAME, "");
  if (!username.empty()) {
    m_username = username;
  }

  auto greet_on_entry = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_GREET, "");
  if (!greet_on_entry.empty()) {
    m_greet_on_entry = greet_on_entry.compare("true") == 0;
  }

  auto test_mode = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_TEST_MODE, "");
  if (!test_mode.empty()) {
    m_test_mode = test_mode.compare("true") == 0;
  }

  auto retry_mode = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_RETRY_MODE, "");
  if (!retry_mode.empty()) {
    m_retry_mode = retry_mode.compare("true") == 0;
  }

}



  /**
   * FetchLiveVideoID
   *
   * @returns [out] {std::string}
   */
  std::string YouTubeDataAPI::FetchLiveVideoID() {
    using namespace constants;

    cpr::Response r = cpr::Get(
      cpr::Url{URL_VALUES.at(SEARCH_URL_INDEX)},
      cpr::Header{
        {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
        {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}
      },
      cpr::Parameters{
        {PARAM_NAMES.at(PART_INDEX),    PARAM_VALUES.at(SNIPPET_INDEX)},
        {PARAM_NAMES.at(KEY_INDEX),     m_authenticator.get_key()},
        {PARAM_NAMES.at(CHAN_ID_INDEX), PARAM_VALUES.at(CHAN_KEY_INDEX)},
        {PARAM_NAMES.at(EVENT_T_INDEX), PARAM_VALUES.at(LIVE_EVENT_TYPE_INDEX)},
        {PARAM_NAMES.at(TYPE_INDEX),    PARAM_VALUES.at(VIDEO_TYPE_INDEX)}
      }
    );

    json video_info = json::parse(r.text);

    if (!video_info.is_null() && video_info.is_object()) {
      auto items = video_info["items"];
      if (!items.is_null() && items.is_array() && items.size() > 0) {
        log(items[0]["snippet"].dump());
        m_video_details.id = items[0]["id"]["videoId"];
        SanitizeJSON(m_video_details.id);
        m_video_details.title         = items[0]["snippet"]["title"];
        m_video_details.description   = items[0]["snippet"]["description"];
        m_video_details.channel_title = items[0]["snippet"]["channelTitle"];
        m_video_details.channel_id    = items[0]["snippet"]["channelId"];
        m_video_details.url           = to_youtube_url(m_video_details.id);
        m_video_details.thumbnail     = (items[0]["snippet"].contains("thumbnails") &&
                                        !items[0]["snippet"]["thumbnails"].is_null()) ?
                                        items[0]["snippet"]["thumbnails"]["high"]["url"] :
                                        "";
      }
      log("Fetched live video details for channel " + PARAM_VALUES.at(CHAN_KEY_INDEX));
    }

    return m_video_details.id;
  }

  /**
   * GetLiveDetails
   *
   * @returns [out] {VideoDetails}
   */
  VideoDetails YouTubeDataAPI::GetLiveDetails() {
    return m_video_details;
  }

  /**
   * FetchLiveDetails
   *
   * @returns [out] {bool}
   */
  bool YouTubeDataAPI::FetchLiveDetails() {
    using namespace constants;

    if (m_video_details.id.empty()) {
      log("Unable to Fetch live details: no video ID");
      return false;
    }

    cpr::Response r = cpr::Get(
      cpr::Url{URL_VALUES.at(VIDEOS_URL_INDEX)},
      cpr::Header{
        {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
        {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}
      },
      // ,
      cpr::Parameters{
        {PARAM_NAMES.at(PART_INDEX),    PARAM_VALUES.at(LIVESTREAM_DETAILS_INDEX)},
        {PARAM_NAMES.at(KEY_INDEX),     m_authenticator.get_key()},
        {PARAM_NAMES.at(ID_INDEX),      m_video_details.id}
      }
    );

    json live_info = json::parse(r.text);

    if (!live_info.is_null() && live_info.is_object()) {
      auto items = live_info["items"];
      if (!items.is_null() && items.is_array() && items.size() > 0) {
        m_video_details.chat_id = items[0]["liveStreamingDetails"]["activeLiveChatId"].dump();
        SanitizeJSON(m_video_details.chat_id);
        if (!m_video_details.chat_id.empty()) {
          m_chats.insert({m_video_details.chat_id, std::vector<LiveMessage>{}});
          log("Added chat details for " + m_video_details.chat_id);
          return true;
        }
      }
    }
    return false;
  }

  /**
   * FetchChatMessages
   *
   * @returns [out] {std::string}
   */
  std::string YouTubeDataAPI::FetchChatMessages() {
  using namespace constants;

    log("Fetching chat messages for " + m_video_details.chat_id);

    cpr::Response r = cpr::Get(
      cpr::Url{URL_VALUES.at(LIVE_CHAT_URL_INDEX)},
      cpr::Header{
        {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
        {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}
      },
      cpr::Parameters{
        {PARAM_NAMES.at(PART_INDEX),           PARAM_VALUES.at(SNIPPET_INDEX)},
        {PARAM_NAMES.at(KEY_INDEX),            m_authenticator.get_key()},
        {PARAM_NAMES.at(LIVE_CHAT_ID_INDEX),   m_video_details.chat_id},
      }
    );

    json chat_info = json::parse(r.text);

    if (!chat_info.is_null() && chat_info.is_object()) {
      auto      items = chat_info["items"];


      if (!items.is_null() && items.is_array() && !items.empty()) {
        for (const auto& item : items) {

          try {
            auto details       = item.at("snippet").at("textMessageDetails");
            std::string text   = (details.contains("messageText")) ?
                                   details["messageText"] :
                                   "";

            std::string author = item["snippet"]["authorChannelId"];
            std::string time   = item["snippet"]["publishedAt"];

            if (!IsNewer(time.c_str())) { // Ignore duplicates
              continue; // FIX: This is a bug -> tie is always old -> timezone issues?
            }

            SanitizeJSON(text);
            SanitizeJSON(author);
            SanitizeJSON(time);

            m_chats.at(m_video_details.chat_id).push_back(
              LiveMessage{
                .timestamp = time,
                .author    = author,
                .text      = text
              }
            );
          } catch (const std::exception& e) {
            std::string error_message{"Exception was caught: "};
            error_message += e.what();
            log(error_message);
          }
        }

        if (!m_chats.at(m_video_details.chat_id).empty()) {
          m_last_fetch_timestamp = to_unixtime(m_chats.at(m_video_details.chat_id).back().timestamp.c_str());
        }
      }
    }

    return r.text;
  }

  bool YouTubeDataAPI::IsNewer(const char* datetime) {
    return std::difftime(to_unixtime(datetime), m_last_fetch_timestamp) > 0;
  }

  /**
   * Parsetokens
   *
   * @returns
   *
   */
  bool YouTubeDataAPI::ParseTokens() {
    if (HasChats()) {
      for (auto&& chat : m_chats.at(m_video_details.chat_id)) {
        std::string tokenized_text = conversation::TokenizeText(chat.text);

        if (!tokenized_text.empty()) {
          chat.tokens = conversation::SplitTokens(tokenized_text);
        }
      }
    return (!GetCurrentChat().at(0).tokens.empty());
  }
  return false;
}

  /**
   * GetChats
   *
   * @returns [out] {LiveChatMap}
   */
  LiveChatMap YouTubeDataAPI::GetChats() {
    return m_chats;
  }

  /**
   * GetCurrentChat
   *
   * @param   [in]  {bool}
   * @returns [out] {LiveMessages}
   */
  LiveMessages YouTubeDataAPI::GetCurrentChat(bool keep_messages) {
    return m_chats.at(m_video_details.chat_id);
  }

  /**
   * HasChats
   *
   * @returns [out] {bool}
   */
  bool YouTubeDataAPI::HasChats() {
    return !m_chats.empty() && !GetCurrentChat().empty();
  }

  /**
   * InsertMessages
   *
   * @param   [in]  {std::string}
   * @param   [in]  {LiveMessages}
   * @returns [out] {bool}
   */
  bool YouTubeDataAPI::InsertMessages(std::string id, LiveMessages&& messages) {
    if (m_chats.find(id) != m_chats.end()) {
      for (auto&& message : messages) m_chats.at(id).emplace_back(message);
      return true;
    }
    return false;
  }

  /**
   * ClearChat
   *
   * @param   [in]  {std::string}
   * @returns [out] {bool}
   */
  bool YouTubeDataAPI::ClearChat(std::string id) {
    id = (id.empty()) ? m_video_details.chat_id : id;

    if (m_chats.find(id) != m_chats.end()) {
      m_chats.at(id).clear();
      return true;
    }

    return false;
  }
  /**
   * FindMentions
   *
   * @returns [out] {LiveMessages}
   */
  LiveMessages YouTubeDataAPI::FindMentions(bool keep_messages) {
    using ChatPair = std::map<std::string, LiveMessages>;
    const std::string bot_name = GetUsername();

    LiveMessages matches{};

    for (const Chat& m : GetChats()) {
      std::string            chat_name = m.first;
      LiveMessages           messages  = m.second;
      LiveMessages::iterator it        = messages.begin();

      for (; it != messages.end(); ) {
        LiveMessage message = *it;

        if (message.text.find(bot_name) != std::string::npos) {
          matches.push_back(message);

          (keep_messages) ?
            it++ :
            it = messages.erase(it);
        } else {
          it++;
        }
      }
    }
    return matches;
  }

  /**
   * FindChat
   *
   * @returns [out] {bool}
   */
  bool YouTubeDataAPI::FindChat() {
    if (!m_authenticator.is_authenticated())
      m_authenticator.FetchToken();

    if (FetchLiveVideoID().empty()) {
      return false;
    }

    if (!FetchLiveDetails()) {
      return false;
    }

    FetchChatMessages();

    return true;
  }

  /**
   * PostMessage
   *
   * @param
   * @returns
   */
  bool YouTubeDataAPI::PostMessage(std::string message) {
    using namespace constants;

    if (m_video_details.chat_id.empty()) {
      log("No chat to post to");
      return false;
    }

    log("Posting " + message);

    json payload{};
    payload["snippet"]["liveChatId"]                        = m_video_details.chat_id;
    payload["snippet"]["live_chat_id"]                      = m_video_details.chat_id;
    payload["snippet"]["textMessageDetails"]["messageText"] = message;
    payload["snippet"]["type"]                              = "textMessageEvent";

    cpr::Response r = cpr::Post(
      cpr::Url{URL_VALUES.at(LIVE_CHAT_URL_INDEX)},
      cpr::Header{
        {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
        {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()},
        {HEADER_NAMES.at(CONTENT_TYPE_INDEX),  HEADER_VALUES.at(APP_JSON_INDEX)}
      },
      cpr::Parameters{
        {PARAM_NAMES.at(PART_INDEX),           PARAM_VALUES.at(SNIPPET_INDEX)},
        {PARAM_NAMES.at(KEY_INDEX),            m_authenticator.get_key()},
        {PARAM_NAMES.at(LIVE_CHAT_ID_INDEX),   m_video_details.chat_id},
      },
      cpr::Body{payload.dump()}
    );

    return r.status_code < 400;
  }

/**
 * GreetOnEntry
 *
 * @returns
 */
bool YouTubeDataAPI::GreetOnEntry() {
  return m_greet_on_entry;
}

/**
 * TestMode
 *
 * @returns [out] {bool}
 */
// bool YouTubeDataAPI::TestMode() {
//   return m_test_mode;
// }

/**
 * RecordInteraction
 *
 * @param
 * @param
 */
void YouTubeDataAPI::RecordInteraction(std::string id, Interaction interaction, std::string value) {
  if (m_interactions.find(id) == m_interactions.end()) {
    std::pair<std::string, bool> interaction_pair{value, true};
    m_interactions.insert({id, UserInteraction{.id = id}});
    if (interaction == Interaction::greeting) {
      m_persons.insert(interaction_pair);
    }
    else
    if (interaction == Interaction::location_ask) {
      m_locations.insert(interaction_pair);
    }
    else
    if (interaction == Interaction::probing) {
      m_orgs.insert(interaction_pair);
    }
  }

  UserInteraction& user_interaction = m_interactions.at(id);

  if (interaction == Interaction::greeting) {
    user_interaction.greeted = true;
  }
  else
  if (interaction == Interaction::promotion) {
    user_interaction.promoted = true;
  }
  else
  if (interaction == Interaction::probing) {
    user_interaction.probed = true;
  }
  else
  if (interaction == Interaction::location_ask) {
    user_interaction.location = true;
  }
}

/**
 * HasInteracted
 */
bool YouTubeDataAPI::HasInteracted(std::string id, Interaction interaction) {
  if (m_interactions.find(id) == m_interactions.end()) {
    return false;
  }

  UserInteraction& user_interaction = m_interactions.at(id);

  bool has_interacted{false};

  if (interaction == Interaction::greeting) {
    has_interacted = user_interaction.greeted;
  }
  else
  if (interaction == Interaction::promotion) {
    has_interacted = user_interaction.promoted;
  }
  else
  if (interaction == Interaction::probing) {
    has_interacted = user_interaction.probed;
  }
  else
  if (interaction == Interaction::location_ask) {
    has_interacted = user_interaction.location;
  }

  return has_interacted;
}

/**
 *
 */
bool YouTubeDataAPI::HasDiscussed(std::string value, Interaction type) {
  std::map<std::string, bool>::iterator it{};
  if (type == Interaction::greeting) {
    it = m_persons.find(value);
  }
  else
  if (type == Interaction::promotion) {
    // TODO: ?
  }
  else
  if (type == Interaction::probing) {
    it = m_orgs.find(value);
  }
  else
  if (type == Interaction::location_ask) {
    it = m_locations.find(value);
  }
  return (it->second == true);
}


/**
 * @brief
 *
 * @return true
 * @return false
 */
bool YouTubeDataAPI::is_authenticated()
{
  return m_authenticator.is_authenticated();
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool YouTubeDataAPI::init()
{
  return m_authenticator.FetchToken();
}

bool YouTubeDataAPI::fetch_channel_data() {
  std::string id_string{};
  for (const auto& channel_id : m_channel_ids) id_string += channel_id + ",";

  try {
    m_channels = fetch_channel_info(id_string);
    return !(m_channels.empty());
  }
  catch (const std::exception& e) {
    log(e.what());
  }
  return false;
}

/**
 * fetch_channel_videos
 *
 * @return std::vector<VideoInfo>
 */
bool YouTubeDataAPI::fetch_channel_videos()
{
  using namespace constants;
  using json = nlohmann::json;

  if (m_channels.empty()) {
    if (!fetch_channel_data()) {
      log("Unable to fetch channel data");
      return false;
    }
  }
  // TODO: Replace below with standard for loop?
  for (ChannelInfo& channel : m_channels)
    {
      std::vector<VideoInfo> info_v{};

      cpr::Response r = cpr::Get(
        cpr::Url{URL_VALUES.at(SEARCH_URL_INDEX)},
        cpr::Header{
          {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
          {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}},
        cpr::Parameters{
          {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)},    // snippet
          {PARAM_NAMES.at(KEY_INDEX),        m_authenticator.get_key()},         // key
          {PARAM_NAMES.at(CHAN_ID_INDEX),    channel.id},                        // channel id
          {PARAM_NAMES.at(TYPE_INDEX),       PARAM_VALUES.at(VIDEO_TYPE_INDEX)}, // type
          {PARAM_NAMES.at(ORDER_INDEX),      PARAM_VALUES.at(DATE_VALUE_INDEX)}, // order by
          {PARAM_NAMES.at(MAX_RESULT_INDEX), std::to_string(5)}                  // limit
        }
      );

      m_quota += youtube::QUOTA_LIMIT.at(youtube::SEARCH_LIST_QUOTA_INDEX);

      json video_info = json::parse(r.text);

      if (!video_info.is_null() && video_info.is_object())
      {
        auto items = video_info["items"];
        if (!items.is_null() && items.is_array() && items.size() > 0)
        {
          for (const auto &item : items)
          {
            try
            {
              auto video_id = item["id"]["videoId"];
              auto datetime = item["snippet"]["publishedAt"];

              info_v.emplace_back(
                VideoInfo{
                  .channel_id  = item["snippet"]["channelId"],
                  .id          = video_id,
                  .title       = item["snippet"]["title"],
                  .description = item["snippet"]["description"],
                  .datetime    = datetime,
                  .time        = to_readable_time(datetime.dump().c_str()),
                  .url         = youtube_id_to_url(video_id)
                }
              );
            }
            catch (const std::exception &e)
            {
              std::string error_message{"Exception was caught: "};
              error_message += e.what();
              log(error_message);
              return false;
            }
          }
        }
      }
      channel.videos = info_v;
    }

  return true;
}

/**
 * fetch_video_stats
 *
 * @param id_string
 * @return std::vector<VideoStats>
 */
std::vector<VideoStats> YouTubeDataAPI::fetch_video_stats(std::string id_string)
{
  using namespace constants;

  using json = nlohmann::json;

  std::vector<VideoStats> stats{};

  cpr::Response r = cpr::Get(
    cpr::Url{URL_VALUES.at(VIDEOS_URL_INDEX)},
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}
    },
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),    VideoParamsFull()},
      {PARAM_NAMES.at(KEY_INDEX),     m_authenticator.get_key()},
      {PARAM_NAMES.at(ID_INDEX),      id_string}
    }
  );

  m_quota += youtube::QUOTA_LIMIT.at(youtube::VIDEO_LIST_QUOTA_INDEX);

  json video_info = json::parse(r.text);

  if (!video_info.is_null() && video_info.is_object())
  {
    auto items = video_info["items"];
    if (!items.is_null() && items.is_array() && items.size())
    {
      for (int i = 0; i < items.size(); i++)
      {
        try
        {
          const auto &item = items.at(i);

          stats.emplace_back(VideoStats{
            .views = (item["statistics"].contains("viewCount")) ? item["statistics"]["viewCount"] : "0",
            .likes = (item["statistics"].contains("likeCount")) ? item["statistics"]["likeCount"] : "0",
            .dislikes = (item["statistics"].contains("dislikeCount")) ? item["statistics"]["dislikeCount"] : "0",
            .comments = (item["statistics"].contains("commentCount")) ? item["statistics"]["commentCount"] : "0",
            .keywords = (item["snippet"].contains("tags")) ?
                          item["snippet"]["tags"].get<std::vector<std::string>>() :
                          std::vector<std::string>{}
          });
        }
        catch (const std::exception &e)
        {
          std::string error_message{"Exception was caught: "};
          error_message += e.what();
          log(error_message);
        }
      }
    }
  }
  return stats;
}

/**
 * fetch_youtube_stats
 *
 * @returns [out] {std::vector<VideoInfo}
 */
std::vector<ChannelInfo> YouTubeDataAPI::fetch_youtube_stats()
{
  using namespace constants;
  using json = nlohmann::json;

  if (m_authenticator.is_authenticated() || m_authenticator.FetchToken())
  {
    if (fetch_channel_videos()) {
      for (auto& channel : m_channels) {
        std::string id_string{};

        for (const auto &info : channel.videos) id_string += info.id + ",";

        std::vector<VideoStats> stats = fetch_video_stats(id_string);
        std::size_t             stats_size = stats.size();

        if (stats_size == channel.videos.size())
        {
          for (uint8_t i = 0; i < stats_size; i++)
          {
            channel.videos.at(i).stats = stats.at(i);
          }
        }
      }
    }
  }

  return m_channels;
}

/**
 * fetch_rival_videos
 *
 * @param video
 * @return std::vector<VideoInfo>
 */
std::vector<VideoInfo> YouTubeDataAPI::fetch_rival_videos(VideoInfo video)
{
  using namespace constants;
  using json = nlohmann::json;

  std::vector<VideoInfo> info_v{};
  std::string            id_string{};
  std::string            delim{};

  if (video.stats.keywords.empty()) // Nothing to search
    return info_v;

  auto search_term = video.stats.keywords.front();

  cpr::Response r = cpr::Get(
    cpr::Url{URL_VALUES.at(SEARCH_URL_INDEX)},
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}},
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)},    // snippet
      {PARAM_NAMES.at(KEY_INDEX),        m_authenticator.get_key()},         // key
      {PARAM_NAMES.at(QUERY_INDEX),      search_term},                       // query term
      {PARAM_NAMES.at(TYPE_INDEX),       PARAM_VALUES.at(VIDEO_TYPE_INDEX)}, // type
      {PARAM_NAMES.at(ORDER_INDEX),      PARAM_VALUES.at(VIEW_COUNT_INDEX)}, // order by
      {PARAM_NAMES.at(MAX_RESULT_INDEX), std::to_string(5)}                  // limit
    }
  );

  m_quota += youtube::QUOTA_LIMIT.at(youtube::SEARCH_LIST_QUOTA_INDEX);

  json video_info = json::parse(r.text);

  if (!video_info.is_null() && video_info.is_object())
  {
    auto items = video_info["items"];
    if (!items.is_null() && items.is_array())
    {
      for (const auto &item : items)
      {
        try
        {
          auto video_id = item["id"]["videoId"];
          auto datetime = item["snippet"]["publishedAt"];

          VideoInfo info{
              .channel_id  = item["snippet"]["channelId"],
              .id          = video_id,
              .title       = item["snippet"]["title"],
              .description = item["snippet"]["description"],
              .datetime    = datetime,
              .time        = to_readable_time(datetime),
              .url         = youtube_id_to_url(video_id)};

          info_v.push_back(info);
          id_string += delim + video_id.dump();
          delim = ",";
        }
        catch (const std::exception &e)
        {
          std::string error_message{"Exception was caught: "};
          error_message += e.what();
          log(error_message);
        }
      }
    }
  }

  std::vector<VideoStats> vid_stats = fetch_video_stats(id_string);
  auto stats_size = vid_stats.size();

  if (stats_size == info_v.size())
  {
    for (uint8_t i = 0; i < stats_size; i++)
    {
      info_v.at(i).stats = vid_stats.at(i);
    }
  }
  return info_v;
}

/**
 * find_similar_videos
 *
 * @param   [in]  {VideoInfo}
 * @returns [out] {std::vector<ChannelInfo>}
 */
std::vector<ChannelInfo> YouTubeDataAPI::find_similar_videos(VideoInfo video)
{
  std::vector<ChannelInfo> channels{};
  std::vector<VideoInfo>   videos = fetch_rival_videos(video);

  auto    it     = std::make_move_iterator(videos.begin());
  auto    it_end = std::make_move_iterator(videos.end());
  uint8_t idx{0};

  while (it != it_end) {
    auto chan_it = std::find_if(
      channels.begin(), channels.end(),
      [&it](const ChannelInfo& channel) {
        return channel.id.compare(it->channel_id) == 0;
      }
    );

    if (chan_it != channels.end()) {
      chan_it->videos.emplace_back(std::move((*it)));
    } else {
      std::vector<ChannelInfo> channel_infos = fetch_channel_info(it->channel_id);

      if (!channel_infos.empty()) {
        if (channel_infos.size() > 1) {
          // TODO: Logger should note that this was a strange result
        }

        channels.emplace_back(std::move(channel_infos.front()));
        channels.back().videos.emplace_back(std::move((*it)));
      } else {
        // TODO: Unable to find channel. Strange
      }
    }

    idx++;
    it = std::make_move_iterator(videos.begin() + idx);
  }

  return channels;
}

/**
 * get_videos
 */
std::vector<VideoInfo> YouTubeDataAPI::get_videos()
{
  std::vector<VideoInfo> videos{};

  for (const auto& channel : m_channels) {
    videos.insert(
      videos.end(),
      channel.videos.begin(),
      channel.videos.end()
    );
  }

  return videos;
}

/**
 * has_videos
 *
 * @interface
 */

bool YouTubeDataAPI::has_videos() {
  return !m_videos.empty();
}

/**
 *
 * fetch_google_trends
 *
 * @param   [in]  {std::vector<std::string>} terms
 * @returns [out] {std::vector<GoogleTrend>}
 */
std::vector<GoogleTrend> YouTubeDataAPI::fetch_google_trends(std::vector<std::string> terms) {
  return query_google_trends(terms);
}

/**
 * fetch_videos_by_terms
 *
 * @param terms
 * @return std::vector<VideoInfo>
 */
std::vector<VideoInfo> YouTubeDataAPI::fetch_videos_by_terms(std::vector<std::string> terms) {
  using namespace constants;
  using json = nlohmann::json;

  const char delimiter{'|'};

  std::vector<VideoInfo> info_v{};

  std::string query = std::accumulate(
    std::next(terms.cbegin()),
    terms.cend(),
    terms.front(),
    [&delimiter](const std::string& a, const std::string& b) {
      return a + delimiter + b;
    }
  );

  cpr::Response r = cpr::Get(
    cpr::Url{URL_VALUES.at(SEARCH_URL_INDEX)},
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}},
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),           PARAM_VALUES.at(SNIPPET_INDEX)},    // snippet
      {PARAM_NAMES.at(KEY_INDEX),            m_authenticator.get_key()},         // key
      {PARAM_NAMES.at(QUERY_INDEX),          query},                             // query terms
      {PARAM_NAMES.at(TYPE_INDEX),           PARAM_VALUES.at(VIDEO_TYPE_INDEX)}, // type
      {PARAM_NAMES.at(ORDER_INDEX),          PARAM_VALUES.at(VIEW_COUNT_INDEX)}, // order by
      {PARAM_NAMES.at(MAX_RESULT_INDEX),     std::to_string(5)}                  // limit
    }
  );

  m_quota += youtube::QUOTA_LIMIT.at(youtube::SEARCH_LIST_QUOTA_INDEX);

  json video_info = json::parse(r.text);

  if (!video_info.is_null() && video_info.is_object())
  {
    auto items = video_info["items"];
    if (!items.is_null() && items.is_array())
    {
      for (const auto &item : items)
      {
        try
        {
          auto video_id = item["id"]["videoId"];
          auto datetime = item["snippet"]["publishedAt"];

          VideoInfo info{
            .channel_id  = PARAM_VALUES.at(CHAN_KEY_INDEX),
            .id          = video_id,
            .title       = item["snippet"]["title"],
            .description = item["snippet"]["description"],
            .datetime    = datetime,
            .time        = to_readable_time(datetime),
            .url         = youtube_id_to_url(video_id)};

          info_v.push_back(info);
        }
        catch (const std::exception &e)
        {
          std::string error_message{"Exception was caught: "};
          error_message += e.what();
          log(error_message);
        }
      }
    }
  }

  return info_v;
}

/**
 * fetch_term_info
 *
 * @param terms
 * @return std::vector<TermInfo>
 */
std::vector<TermInfo> YouTubeDataAPI::fetch_term_info(std::vector<std::string> terms) {
  using namespace constants;
  const char delimiter{','};

  std::vector<TermInfo> metadata_v{};

  if (m_authenticator.is_authenticated() || m_authenticator.FetchToken())
  {
    std::vector<VideoInfo> videos = fetch_videos_by_terms(terms);

    if (!videos.empty()) {
    std::string                                    id_string{};
    std::vector<VideoInfo>::const_iterator it = videos.cbegin();

    id_string.reserve(videos.size() * youtube::YOUTUBE_VIDEO_ID_LENGTH);
    id_string += (*it).id;

    while (++it != videos.cend()) id_string += delimiter + (*it).id;

    std::vector<VideoStats> stats = fetch_video_stats(id_string);

    if (stats.size() == videos.size())
    {
      for (uint8_t i = 0; i < stats.size(); i++)
      {
        videos.at(i).stats = stats.at(i);
      }
    }

    int score{};

    for (const auto& video : videos) {
      if (std::stoi(video.stats.views) > 100)
        score ++;
    }

    for (const auto& term : terms)
      metadata_v.emplace_back(
        TermInfo{
          .term = term, .value = score
        });
    }
  }

  return metadata_v;
}

std::vector<ChannelInfo> YouTubeDataAPI::fetch_channel_info(std::string id_string) {
  using namespace constants;
  using json = nlohmann::json;

  bool  NO_EXCEPTIONS_THROWN{false};

  std::vector<ChannelInfo> info_v{};

  cpr::Response r = cpr::Get(
    cpr::Url{URL_VALUES.at(CHANNELS_URL_INDEX)},
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}},
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),           PARAM_VALUES.at(SNIPPET_STATS_INDEX)}, // snippet
      {PARAM_NAMES.at(KEY_INDEX),            m_authenticator.get_key()},            // key
      {PARAM_NAMES.at(ID_INDEX),             id_string},                            // query term
      {PARAM_NAMES.at(TYPE_INDEX),           PARAM_VALUES.at(VIDEO_TYPE_INDEX)},    // type
      {PARAM_NAMES.at(ORDER_INDEX),          PARAM_VALUES.at(VIEW_COUNT_INDEX)},    // order by
      {PARAM_NAMES.at(MAX_RESULT_INDEX),     std::to_string(5)}                     // limit
    }
  );

  m_quota += youtube::QUOTA_LIMIT.at(youtube::SEARCH_LIST_QUOTA_INDEX);

  json channel_json = json::parse(r.text, nullptr, NO_EXCEPTIONS_THROWN);

  if (!channel_json.is_null() && channel_json.is_object() && channel_json.contains("items")) {
    json items = channel_json["items"];

    // TODO: Combine follower counts with channel info
    if (!items.is_null() && items.is_array()) {
      for (const auto& item : items) {
        info_v.emplace_back(
          ChannelInfo{
            .name          = item["snippet"]["title"],
            .description   = item["snippet"]["description"],
            .created       = item["snippet"]["publishedAt"],
            .thumb_url     = item["snippet"]["thumbnails"]["default"]["url"],
            .stats         = ChannelStats{
                .views       = item["statistics"]["viewCount"],
                .subscribers = (item["statistics"].contains("subscriberCount")) ?
                                std::string{item["statistics"]["subscriberCount"]} :
                                std::to_string(0),
                .videos      = item["statistics"]["videoCount"]
            },
            .id            = item["id"]
          }
        );
      }
    }
  }

  return info_v;
}
/**
 * get_quota_used
 *
 * @returns [out] {uint32_t}
 */
const uint32_t YouTubeDataAPI::get_quota_used() const {
  return m_quota;
}

} // namespace ktube
