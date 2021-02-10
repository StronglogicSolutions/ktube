#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include <vector>
#include <string>
#include <unordered_map>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "util.hpp"

/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░ CONSTANTS ░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/
namespace constants {
namespace StatusVisibility {
const std::string PUBLIC  {"public"};
const std::string UNLISTED{"unlisted"};
const std::string PRIVATE {"private"};
const std::string DIRECT  {"direct"};
} // namespace StatusVisibility
} // namespace constants

inline bool visibility_is_valid(const std::string& visibility) {
  return (
    visibility.compare(constants::StatusVisibility::PUBLIC)   == 0 ||
    visibility.compare(constants::StatusVisibility::UNLISTED) == 0 ||
    visibility.compare(constants::StatusVisibility::PRIVATE)  == 0 ||
    visibility.compare(constants::StatusVisibility::DIRECT)   == 0
  );
}

class PostDataInterface {
public:
virtual ~PostDataInterface() {}
virtual std::string postdata() = 0;
};


struct File : public PostDataInterface {
std::string path;
std::string description;
std::string thumbnail;
std::string url;

std::string GetBytes() {
  return ReadFromFile(path);
}

File(std::string file_path) : path(file_path) {}

virtual ~File() override {}
/**
 * @brief
 *
 * TODO: Add thumbnail support
 *
 * @return std::string
 */
virtual std::string postdata() {
  std::string payload = GetBytes();
  return std::string{
    "file="         + payload + "&" +
    "description="  + description
  };
}

cpr::Multipart multiformdata() {
  std::string bytes = GetBytes();
  return cpr::Multipart{
    {"description", description},
    {"file", cpr::File{path}}
  };
}
};

inline std::string GetJSONStringValue(nlohmann::json data, std::string key) {
  if (!data.is_null() && data.contains(key) && !data[key].is_null())
    return data[key].get<std::string>();
  return "";
}

template<typename T>
inline T GetJSONValue(nlohmann::json data, std::string key) {
  if (!data.is_null() && data.contains(key) && !data[key].is_null())
    return data[key].get<T>();

  if (std::is_integral<T>::value)
    return static_cast<T>(0);
  if (std::is_floating_point<T>::value)
    return static_cast<T>(0);
  if constexpr (std::is_same_v<std::string, T>)
    return "";
  if constexpr (std::is_same_v<std::vector<std::string>, T>) {
    return std::vector<std::string>{};
  }

  throw std::invalid_argument{"Unsupported type"};
}

inline bool GetJSONBoolValue(nlohmann::json data, std::string key) {
  if (!data.is_null() && data.contains(key) && !data[key].is_null())
    return data[key].get<bool>();
  return "";
}

/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░░ STRUCTS ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/

struct AccountField {
std::string name;
std::string value;
};

struct Account {
std::string               id;
std::string               username;
std::string               acct;
std::string               display_name;
bool                      locked;
bool                      bot;
bool                      discoverable;
std::string               group;
std::string               created_at;
std::string               note;
std::string               url;
std::string               avatar;
std::string               header;
std::string               header_static;
uint32_t                  followers_count;
uint32_t                  following_count;
uint32_t                  statuses_count;
std::string               last_status_at;
std::vector<AccountField> fields;

friend std::ostream &operator<<(std::ostream& o, const Account& a) {
  std::string fields{};
  for (const auto& field : a.fields) fields += "\nName: " + field.name + "\nValue: " + field.value;

  o << "ID:           " << a.id              << "\n" <<
       "Username:     " << a.username        << "\n" <<
       "Account:      " << a.acct            << "\n" <<
       "Display Name: " << a.display_name    << "\n" <<
       "Locked:       " << a.locked          << "\n" <<
       "Bot:          " << a.bot             << "\n" <<
       "Discoverable: " << a.discoverable    << "\n" <<
       "Group:        " << a.group           << "\n" <<
       "Created:      " << a.created_at      << "\n" <<
       "Note:         " << a.note            << "\n" <<
       "URL:          " << a.url             << "\n" <<
       "Avatar:       " << a.avatar          << "\n" <<
       "Header:       " << a.header          << "\n" <<
       "Followers:    " << a.followers_count << "\n" <<
       "Following:    " << a.following_count << "\n" <<
       "Statuses:     " << a.statuses_count  << "\n" <<
       "Last Status:  " << a.last_status_at  << "\n" <<

       "FIELDS\n"       << fields;

  return o;
}
};

struct Mention {
std::string acct;
std::string id;
std::string url;
std::string username;

friend std::ostream &operator<<(std::ostream& o, const Mention& m) {
  o << "ID: " << m.id << "\n" << "Acct: " << m.acct << "\n" << "URL: " << m.url << "\nUsername:" << m.username << std::endl;

  return o;
}

};


struct Credentials {
std::string id;
std::string name;
std::string website;
std::string redirect_uri;
std::string scope;
std::string client_id;
std::string client_secret;
std::string vapid_key;
std::string code;

bool is_valid() {
  return
    !id.empty() &&
    !name.empty() &&
    !website.empty() &&
    !redirect_uri.empty() &&
    !scope.empty() &&
    !client_id.empty() &&
    !client_secret.empty() &&
    !vapid_key.empty() &&
    !code.empty();
}
};

struct Auth {
std::string access_token;
std::string token_type;
std::string scope;
std::string created_at;

bool is_valid() {
  return (
    !access_token.empty() &&
    !token_type.empty()   &&
    !scope.empty()        &&
    !created_at.empty()
  );
}
};

struct Application {
std::string name;
std::string url;

friend std::ostream &operator<<(std::ostream& o, const Application& a) {
  o << "Name: " << a.name << "\n" << "URL: " << a.url << std::endl;

  return o;
}
};

struct MetaDetails {
uint32_t    width;
uint32_t    height;
std::string size;
float       aspect;
};
struct MediaMetadata {
MetaDetails original;
MetaDetails small;
};

struct Tag {
std::string name;
std::string url;

friend std::ostream &operator<<(std::ostream& o, const Tag& t) {
  o << "Name: " << t.name << "\n" << "URL: " << t.url << std::endl;

  return o;
}
};

struct Card {};
struct Poll {};
struct Media {
std::string id;
std::string type;
std::string url;
std::string preview_url;
std::string remote_url;
std::string preview_remote_url;
std::string text_url;
MediaMetadata meta;
std::string description;
std::string blurhash;

bool has_media() { return !id.empty(); }

friend std::ostream &operator<<(std::ostream& o, const Media& m) {
o << "ID: " << m.id << "\nURL: " << m.url << "\n(TODO: Complete media ostream)" << std::endl;
return o;
}
};

struct Status : public PostDataInterface {
Status()
: sensitive(false) {}

Status(std::string text)
: sensitive(false), content(text) {}

static Status create_instance_with_message(
  Status             status,
  const std::string& message,
  const std::string& replying_to_id) {

  status.content        = message;
  status.replying_to_id = replying_to_id;
  return status;
}

uint64_t                 id;
std::string              created_at;
std::string              replying_to_id;
std::string              replying_to_account;
bool                     sensitive;
std::string              spoiler;
std::string              visibility;
std::string              language;
std::string              uri;
std::string              url;
uint32_t                 replies;
uint32_t                 reblogs;
uint32_t                 favourites;
std::string              content;
Account                  reblog;
Application              application;
Account                  account;
std::vector<Media>       media;
std::vector<Mention>     mentions;
std::vector<Tag>         tags;
std::vector<std::string> emojis;
Card                     card;
Poll                     poll;

friend std::ostream &operator<<(std::ostream& o, const Status& s) {
  o << "ID:"           << std::to_string(s.id) << "\n" <<
       "Created:"      << s.created_at << "\n" <<
       "To ID:"        << s.replying_to_id << "\n" <<
       "To acc:"       << s.replying_to_account << "\n" <<
       "Sensitive:"    << s.sensitive << "\n" <<
       "Spoiler:"      << s.spoiler << "\n" <<
       "Visibility:"   << s.visibility << "\n" <<
       "Language:"     << s.language << "\n" <<
       "URI:"          << s.uri << "\n" <<
       "URL:"          << s.url << "\n" <<
       "Replies:"      << s.replies << "\n" <<
       "Reblogs:"      << s.reblogs << "\n" <<
       "Favourites:"   << s.favourites << "\n" <<
       "Content:"      << s.content << "\n" <<
       "Reblog:"       << s.reblog << "\n" <<
       "APPLICATION\n" << s.application << "\n" <<
       "ACCOUNT\n"     << s.account << "\n";

       o << "MEDIA\n";
       for (const auto& media : s.media)
        o << media << std::endl;
      o << "MENTIONS\n";
       for (const auto& mention : s.mentions)
        o << mention << std::endl;
      o << "TAGS\n";
       for (const auto& tag : s.tags)
        o << tag << std::endl;
      o << "EMOJIS\n";
       for (const auto& emoji : s.emojis)
        o << emoji << std::endl;

      o << "Card:" << "TODO" << "\n" <<
           "Poll:" << "TODO" << "\n";
  return o;
}

virtual std::string postdata() override {
  std::string media_ids{};
  std::string delim{""};

  for (const auto& media_item : media) {
    media_ids += media_item.id;
    delim = ",";
  }

  if (!visibility.empty() && !visibility_is_valid(visibility))
    throw std::invalid_argument{"This visibility is not recognized"};

  std::string RC{
    "status="         + content   + "&" +
    "media_ids[]="    + media_ids + "&" +
    "spoiler_text="   + spoiler   + "&" +
    "in_reply_to_id=" + replying_to_id + "&" +
    "visibility="     + visibility + "&" +
    "sensitive="      + std::to_string(sensitive)
  };

  return RC;
}

bool is_valid() const {
  return (!content.empty());
}

virtual ~Status() override {}
};

struct Conversation {
std::string          id;
bool                 unread;
Status               status;
std::vector<Account> accounts;

friend std::ostream &operator<<(std::ostream& o, const Conversation& c) {
  o << "ConversationID: " << c.id << std::endl;

  if (!c.status.content.empty())
    o << c.status;

  return o;
}
};

#endif // __TYPES_HPP__
