#ifndef __AUTH_HPP__
#define __AUTH_HPP__

#include <INIReader.h>
#include "kstodon/common/request.hpp"
#include "kstodon/common/mastodon_util.hpp"

inline const std::string get_dir() {
  char* path = realpath("/proc/self/exe", NULL);
  char* name = basename(path);
  return std::string {path, path + strlen(path) - strlen(name)};
}

inline bool ValidateCredentialsJSON(nlohmann::json json_file) {
  return(
    !json_file.is_null()                &&
    json_file.is_object()               &&
    json_file.contains("id")            &&
    json_file.contains("name")          &&
    json_file.contains("website")       &&
    json_file.contains("redirect_uri")  &&
    json_file.contains("scope")         &&
    json_file.contains("client_id")     &&
    json_file.contains("client_secret") &&
    json_file.contains("vapid_key")     &&
    json_file.contains("code")
  );
}

inline bool JSONHasUser(nlohmann::json data, std::string username) {
  auto is_null = data.is_null();
  auto is_obj  = data.is_object();
  auto hasname = data.contains(username);

  return (!data.is_null() && data.is_object() && data.contains(username));
}

inline Credentials ParseCredentialsFromJSON(nlohmann::json json_file, std::string username) {
  using json = nlohmann::json;

  Credentials creds{};

  if (json_file.contains("users")) {
    json users_json = json_file["users"];

    if (
      !users_json.is_null()            &&
      JSONHasUser(users_json, username) &&
      ValidateCredentialsJSON(users_json[username])) {

      json user_json = users_json[username];

      creds.id =            GetJSONStringValue(user_json, "id");
      creds.name =          GetJSONStringValue(user_json, "name");
      creds.website =       GetJSONStringValue(user_json, "website");
      creds.redirect_uri =  GetJSONStringValue(user_json, "redirect_uri");
      creds.scope        =  GetJSONStringValue(user_json, "scope");
      creds.client_id =     GetJSONStringValue(user_json, "client_id");
      creds.client_secret = GetJSONStringValue(user_json, "client_secret");
      creds.vapid_key =     GetJSONStringValue(user_json, "vapid_key");
      creds.code =          GetJSONStringValue(user_json, "code");
    }
  }

  return creds;
}

inline bool ValidateAuthJSON(nlohmann::json json_file) {
  return(
    !json_file.is_null()               &&
    json_file.is_object()              &&
    json_file.contains("access_token") &&
    json_file.contains("token_type")   &&
    json_file.contains("scope")        &&
    json_file.contains("created_at")
  );
}

inline std::string AuthToJSON(Auth auth) {
  nlohmann::json auth_json{};
  auth_json["access_token"] = auth.access_token;
  auth_json["token_type"]   = auth.token_type;
  auth_json["scope"]        = auth.scope;
  auth_json["created_at"]   = auth.created_at;

  return auth_json.dump();
}

inline Auth ParseAuthFromJSON(nlohmann::json json_file) {
  using json = nlohmann::json;

  Auth auth{};

  if (ValidateAuthJSON(json_file)) {
    auth.access_token =  GetJSONStringValue(json_file, "access_token");
    auth.token_type   =  GetJSONStringValue(json_file, "token_type");
    auth.scope        =  GetJSONStringValue(json_file, "scope");
    auth.created_at   =  std::to_string(GetJSONValue<uint32_t>(json_file, "created_at"));
  }

  return auth;
}




class Authenticator {

public:

Authenticator(std::string username = "")
: m_username(username),
  m_authenticated(false)
{
  if (m_username.empty()) {
    INIReader reader{std::string{get_dir() + "../" + constants::DEFAULT_CONFIG_PATH}};

    if (reader.ParseError() < 0) {
      log("Error loading config");
      throw std::invalid_argument{"No configuration path"};
    }


    auto name = reader.GetString(constants::KSTODON_SECTION, constants::USER_CONFIG_KEY, "");

    if (name.empty()) {
      throw std::invalid_argument{"No username in config. Please provide a username"};
    }

    m_username = name;
  }

  m_credentials_json = LoadJSONFile(get_dir() + "../" + constants::CONFIG_JSON_PATH);
  auto credentials   = ParseCredentialsFromJSON(m_credentials_json, m_username);

  if (!credentials.is_valid()) {
    throw std::invalid_argument{"Credentials not found"};
  }

  m_credentials = credentials;

  m_token_json = LoadJSONFile(get_dir() + "../" + constants::TOKEN_JSON_PATH);

  if (
    m_token_json.contains("users")    &&
    !m_token_json["users"].is_null()  &&
    m_token_json["users"].contains(m_username) &&
    !m_token_json["users"][m_username].is_null()) {

    auto auth = ParseAuthFromJSON(m_token_json["users"][m_username]);

    if (auth.is_valid()) {
      m_auth = auth;
      m_authenticated = true;
    }
  }
}

/**
 * FetchToken
 *
 * @returns [out] {bool}
 */
bool FetchToken() {
  using namespace constants::MastodonOnline;
  using json = nlohmann::json;

  const std::string AUTHORIZATION_CODE_GRANT_TYPE{"authorization_code"};
  const std::string AUTH_URL = BASE_URL + PATH.at(TOKEN_INDEX);
  std::string       response;
  std::string       status;

  if (m_credentials.is_valid()) {
    cpr::Response r = cpr::Post(
      cpr::Url{AUTH_URL},
      cpr::Body{std::string{
        "client_id=" + m_credentials.client_id +  "&" +
        "client_secret=" + m_credentials.client_secret + "&" +
        "redirect_uri=" + m_credentials.redirect_uri + "&" +
        "grant_type=" + AUTHORIZATION_CODE_GRANT_TYPE + "&" +
        "code=" + m_credentials.code + "&" +
        "scope=" + m_credentials.scope
      }}
    );

    response = r.text;
    status   = std::string{"Status code: " + r.status_code};

    if (!response.empty()) {
      json auth_json = json::parse(response, nullptr, constants::JSON_PARSE_NO_THROW);
      Auth auth      = ParseAuthFromJSON(auth_json);

      if (auth.is_valid()) {
        m_auth = auth;
        m_authenticated = true;
        m_token_json["users"][m_username] = auth_json;
        SaveToFile(m_token_json.dump(), constants::TOKEN_JSON_PATH);
        return true;
      } else {
        log("Failed to parse token");
      }
    } else {
      log("Token request failed");
    }
    log(std::string{
      "Failed to fetch token.\n"
      "Code: "     + status + "\n"
      "Response: " + response
    });
  } else {
    log("Credentials are invalid");
  }

  return false;
}

bool VerifyToken() {
  using namespace constants;
  using namespace constants::MastodonOnline;

  const std::string URL = BASE_URL + PATH.at(TOKEN_VERIFY_INDEX);

  // curl \
	// -H 'Authorization: Bearer our_access_token_here' \
	// https://mastodon.example/api/v1/accounts/verify_credentials
  if (m_auth.is_valid()) {
    cpr::Response r = cpr::Get(
      cpr::Url{URL},
      cpr::Header{
        {HEADER_NAMES.at(HEADER_AUTH_INDEX), GetBearerAuth()}
     }
    );

    if (!r.text.empty()) {
      log(r.text);

      Account account = ParseAccountFromJSON(
        nlohmann::json::parse(
          r.text, nullptr, constants::JSON_PARSE_NO_THROW
        )
      );

      m_account = account;

      return true;
    }
  }

  return false;
}


bool IsAuthenticated() {
  return m_authenticated;
}

bool HasValidToken() {
  return m_auth.is_valid();
}

void ClearToken() {
  m_auth = Auth{};
}

std::string GetBearerAuth() {
  if (m_auth.access_token.empty())
    return "";
  return std::string{"Bearer " + m_auth.access_token};
}

Credentials get_credentials() {
  return m_credentials;
}

Account GetAccount() {
  return m_account;
}

std::string GetUsername() {
  return m_username;
}

private:
using json = nlohmann::json;

Credentials  m_credentials;
Auth         m_auth;
Account      m_account;
std::string  m_username;
bool         m_authenticated;
json         m_token_json;
json         m_credentials_json;

};

#endif // __AUTH_HPP__
