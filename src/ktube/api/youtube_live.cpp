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
      },
      cpr::VerifySsl{m_authenticator.verify_ssl()}
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
      },
      cpr::VerifySsl{m_authenticator.verify_ssl()}
    );

    json live_info = json::parse(r.text);

    if (!live_info.is_null() && live_info.is_object()) {
      auto items = live_info["items"];
      if (!items.is_null() && items.is_array() && items.size() > 0) {
        m_video_details.chat_id = kjson::GetJSONStringValue(items[0]["liveStreamingDetails"], "activeLiveChatId");
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
      },
      cpr::VerifySsl{m_authenticator.verify_ssl()}
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

            // if (!IsNewer(time.c_str())) { // Ignore duplicates
            //   continue; // FIX: This is a bug -> tie is always old -> timezone issues?
            // }

            // SanitizeJSON(text);
            // SanitizeJSON(author);
            // SanitizeJSON(time);

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

        // if (!m_chats.at(m_video_details.chat_id).empty()) {
        //   m_last_fetch_timestamp = to_unixtime(m_chats.at(m_video_details.chat_id).back().timestamp.c_str());
        // }
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
      cpr::Body{payload.dump()},
      cpr::VerifySsl{m_authenticator.verify_ssl()}
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
  if (m_interactions.find(id) == m_interactions.end())
    return false;


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

} // namespace ktube
