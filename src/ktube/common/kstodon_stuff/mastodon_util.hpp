#pragma once

#include "types.hpp"

/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░ HelperFns ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/

inline std::string GetConfigPath() {
  return get_executable_cwd() + "../" + constants::DEFAULT_CONFIG_PATH;
}

inline INIReader GetConfigReader() {
  return INIReader{GetConfigPath()};
}

inline bool SaveStatusID(uint64_t status_id, std::string username) {
  using namespace nlohmann;
  json database_json;
  json loaded_json = LoadJSONFile(get_executable_cwd() + "../" + constants::DB_JSON_PATH);

  if (loaded_json.is_discarded() || loaded_json.is_null())
  {
    database_json["status"] = json::object();
    database_json["status"][username] = {status_id};
  }
  else
  {
    database_json = loaded_json;
    json user_status_array_json = database_json["status"][username];

    if (!user_status_array_json.empty()) {
      for (const auto& id : user_status_array_json.get<const std::vector<uint64_t>>())
        if (id == status_id) return false; // Already exists
    }

    database_json["status"][username].emplace_back(status_id);
  }

  SaveToFile(database_json, constants::DB_JSON_PATH);

  return true;
}

inline std::vector<uint64_t> GetSavedStatusIDs(std::string username) {
  using json = nlohmann::json;

  json db_json = LoadJSONFile(get_executable_cwd() + "../" + constants::DB_JSON_PATH);

  if (!db_json.is_null()                   &&
      db_json.contains("status")           &&
      db_json["status"].contains(username) &&
      !db_json["status"][username].is_null()) {
    return db_json["status"][username].get<std::vector<uint64_t>>();
  }

  return std::vector<uint64_t>{};
}

inline bool RemoveStatusID(std::string username, uint64_t id) {
  using json = nlohmann::json;
  bool result{false};
  json db_json = LoadJSONFile(get_executable_cwd() + "../" + constants::DB_JSON_PATH);

  if (!db_json.is_null()                 &&
    db_json.contains("status")           &&
    db_json["status"].contains(username) &&
    !db_json["status"][username].is_null()) {

    for (int i = 0; i < db_json["status"][username].size(); i++) {
      if (db_json["status"][username][i].get<uint64_t>() == id) {
        db_json["status"][username].erase(i);
        result = true;
        break;
      }
    }
  }

  if (result) {
    SaveToFile(db_json, constants::DB_JSON_PATH);
  }

  return result;
}

inline Account ParseAccountFromJSON(nlohmann::json data) {
  Account account{};
  std::string s = data.dump();
  if (!data.is_null()) {
    account.id              = GetJSONStringValue(data, "id");
    account.username        = GetJSONStringValue(data, "username");
    account.acct            = GetJSONStringValue(data, "acct");
    account.display_name    = GetJSONStringValue(data, "display_name");
    account.locked          = GetJSONBoolValue  (data, "locked");
    account.bot             = GetJSONBoolValue  (data, "bot");
    account.discoverable    = GetJSONBoolValue  (data, "discoverable");
    account.group           = GetJSONBoolValue  (data, "group");
    account.created_at      = GetJSONStringValue(data, "created_at");
    account.note            = GetJSONStringValue(data, "note");
    account.url             = GetJSONStringValue(data, "url");
    account.avatar          = GetJSONStringValue(data, "avatar");
    account.header          = GetJSONStringValue(data, "header");
    account.locked          = GetJSONBoolValue  (data, "locked");
    account.last_status_at  = GetJSONStringValue(data, "last_status_at");
    account.followers_count = GetJSONValue<uint32_t>(data, "followers_count");
    account.following_count = GetJSONValue<uint32_t>(data, "following_count");
    account.statuses_count  = GetJSONValue<uint32_t>(data, "statuses_count");

    nlohmann::json fields = data["source"]["fields"];

    for (const auto& field : fields) {
      account.fields.emplace_back(AccountField{
        .name  = GetJSONStringValue(field, "name"),
        .value = GetJSONStringValue(field, "value")
      });
    }
  }

  return account;
}

inline std::vector<Tag> ParseTagsFromJSON(nlohmann::json data) {
  std::vector<Tag> tags_v{};

  if (!data.is_null()) {
    for (const auto& tag : data) {
      tags_v.emplace_back(Tag{
        .name = tag["name"],
        .url  = tag["url"]
      });
    }
  }

  return tags_v;
}

inline std::vector<Mention> ParseMentionsFromJSON(nlohmann::json data) {
  std::vector<Mention> mentions{};

  if (!data.is_null()) {
    for (const auto& mention : data) {
      mentions.emplace_back(Mention{
        .acct     = mention["acct"],
        .id       = mention["id"],
        .url      = mention["url"],
        .username = mention["username"]
      });
    }
  }

  return mentions;
}

inline Media ParseMediaFromJSON(nlohmann::json data) {
  Media media{};

  if (!data.is_null() && data.is_object()) {
    media.id                 = GetJSONStringValue(data, "id");
    media.type               = GetJSONStringValue(data, "type");
    media.url                = GetJSONStringValue(data, "url");
    media.preview_url        = GetJSONStringValue(data, "preview_url");
    media.remote_url         = GetJSONStringValue(data, "remote_url");
    media.preview_remote_url = GetJSONStringValue(data, "preview_remote_url");
    media.text_url           = GetJSONStringValue(data, "text_url");
    media.description        = GetJSONStringValue(data, "description");
    media.blurhash           = GetJSONStringValue(data, "blurhash");
    media.meta               = MediaMetadata{};

    if (data["meta"].contains("original")) {
      auto original = data["meta"]["original"];

      media.meta.original = MetaDetails{
        .width    = GetJSONValue<uint32_t>(original, "width"),
        .height   = GetJSONValue<uint32_t>(original, "height"),
        .size     = GetJSONStringValue(original, "size"),
        .aspect   = GetJSONValue<float>(original, "aspect")
      };
    }

    if (data["meta"].contains("small")) {
      auto small = data["meta"]["small"];

      media.meta.small = MetaDetails{
        .width    = small["width"],
        .height   = small["height"],
        .size     = small["size"],
        .aspect   = small["aspect"]
      };
    }
  }

  return media;
}

inline std::vector<Media> ParseMediaFromJSONArr(nlohmann::json data) {
  std::vector<Media> media_v{};

  if (!data.is_null() && data.is_array()) {
    for (const auto& item : data) {
      media_v.emplace_back(ParseMediaFromJSON(item));
    }
  }

  return media_v;
}

inline std::vector<Account> ParseAccountsFromJSON(nlohmann::json data) {
  std::vector<Account> accounts{};
  std::string s = data.dump();

  if (!data.is_null() && data.is_array()) {
    for (const auto& item : data) {
      if (!item.is_null() && item.is_object()) {
        accounts.emplace_back(ParseAccountFromJSON(item));
      }
    }
  }
  return accounts;
}

/**
 * @brief
 *
 * @param data
 * @return Status
 */
inline Status JSONToStatus(nlohmann::json data) {
  Status status{};
  std::string s = data.dump();
  try {
    if (!data.is_null() && data.is_object()) {
    status.id                  = std::stoul(data["id"].get<std::string>());
    status.created_at          = GetJSONStringValue    (data, "created_at");
    status.replying_to_id      = GetJSONStringValue    (data, "in_reply_to_id");
    status.replying_to_account = GetJSONStringValue    (data, "in_reply_to_account_id");
    status.sensitive           = GetJSONBoolValue      (data, "sensitive");
    status.spoiler             = GetJSONStringValue    (data, "spoiler_text");
    status.visibility          = GetJSONStringValue    (data, "visibility");
    status.language            = GetJSONStringValue    (data, "language");
    status.uri                 = GetJSONStringValue    (data, "uri");
    status.url                 = GetJSONStringValue    (data, "url");
    status.replies             = GetJSONValue<uint32_t>(data, "replies_count");
    status.reblogs             = GetJSONValue<uint32_t>(data, "reblogs_count");
    status.favourites          = GetJSONValue<uint32_t>(data, "favourites_count");
    status.content             = GetJSONStringValue    (data, "content");
    status.reblog              = ParseAccountFromJSON  (data["reblog"]);;
    status.application.name    = GetJSONStringValue    (data["application"], "name");
    status.application.url     = GetJSONStringValue    (data["application"], "website");
    status.account             = ParseAccountFromJSON  (data["account"]);
    status.media               = ParseMediaFromJSONArr (data["media_attachments"]);
    status.mentions            = ParseMentionsFromJSON (data["mentions"]);
    status.tags                = ParseTagsFromJSON     (data["tags"]);
    status.emojis              = GetJSONValue<std::vector<std::string>>(data, "emojis");
  }

  return status;
  } catch (std::exception& e) {
    std::cout << e.what();
  }
  return status;
}

inline std::vector<Status> JSONToStatuses(nlohmann::json data) {
  std::vector<Status> statuses{};
  if (!data.is_null() && data.is_array())
    for (const auto& status_data : data)
    {
      Status status = JSONToStatus(status_data);
      if (status.is_valid())
        statuses.emplace_back(status);
    }

  return statuses;
}


inline std::vector<Status> JSONContextToStatuses(nlohmann::json data) {
  std::vector<Status> statuses{};
  std::string s = data.dump();

  try {
    for (const auto& context : data) {
      auto new_statuses = JSONToStatuses(context);
      statuses.insert(
        statuses.end(),
        std::make_move_iterator(new_statuses.begin()),
        std::make_move_iterator(new_statuses.end())
      );
    }
  }
  catch (const std::exception& e) {
    std::string error = e.what();
    log(error);
  }

  return statuses;
}
inline std::vector<Conversation> JSONToConversation(nlohmann::json data) {
  std::vector<Conversation> conversations{};

  if (!data.is_null() && data.is_array()) {
    for (const auto& item : data) {
      if (!item.is_null() && item.is_object()) {
        std::string s = item.dump();
        conversations.emplace_back(
          Conversation{
            .id       = GetJSONStringValue(item, "id"),
            .unread   = GetJSONBoolValue(item, "unread"),
            .status   = JSONToStatus(item["last_status"]),
            .accounts = ParseAccountsFromJSON(item["accounts"])
          }
        );
      }
    }
  }
  return conversations;
}

inline std::vector<Conversation> ParseRepliesFromConversations(std::vector<Conversation> conversations, std::vector<uint64_t> status_ids) {
  std::vector<Conversation> replies{};

  for (auto&& conversation : conversations) {
    if (
      std::find(
        status_ids.cbegin(),
        status_ids.cend(),
        string_to_uint64(conversation.status.replying_to_id)) != status_ids.cend())
      replies.emplace_back(std::move(conversation));
  }

  return replies;
}

inline std::string PlatformFromURL(const std::string& url) {
  const std::string::size_type begin_length{8};
  auto begin = url.find_first_of("https://");
  std::string return_s{};
  if (begin != std::string::npos) {
  auto end = url.find_last_of('@');
    if (end != std::string::npos) {
      return_s = url.substr(begin + begin_length, (end - (begin + begin_length - 1)));
    }
  }
  return return_s;
}

/**
 * MakeMention
 *
 * TODO: Create full HTML mention:
 * <span class=\"h-card\">
 *   <a href=\"https://mastodon.online/@username\" class=\"u-url mention\">
 *     @<span>username</span>
 *   </a>
 * </span>
 *
 * For example:
 *
 * "<span class=\"h-card\">"
 * "  <a href=\"" + status.account.url + "/@" + status.account.username + "\" class=\"u-url mention\">"
 * "    @" + status.account.username + '@' + PlatformFromURL(status.account.url) +
 * "  </a>"
 * "</span>"
 *
 * @param status
 * @return std::string
 */
inline std::string MakeMention(const Status& status) {
  if (!status.account.username.empty() && !status.account.url.empty()) {
    return '@' + status.account.username + '@' + PlatformFromURL(status.account.url) + ' ';
  }
  return "";
}
