#pragma once

#include "ktube/common/youtube_util.hpp"
#include "ktube/common/request.hpp"

namespace ktube {
struct AuthData {
  std::string access_token;
  std::string refresh_token;
  std::string scope;
  std::string token_type;
  std::string expires_in;
  std::string key;
  std::string token_app_path;
  std::string client_id;
  std::string client_secret;

bool is_valid() {
  return (
    !access_token.empty() &&
    !token_type.empty()   &&
    !scope.empty()
  );
}
};

inline bool ValidateAuthJSON(const nlohmann::json& json_file) {
  return(
    !json_file.is_null()               &&
    json_file.is_object()              &&
    json_file.contains("access_token") &&
    json_file.contains("token_type")   &&
    json_file.contains("scope")
  );
}

inline AuthData ParseAuthFromJSON(nlohmann::json json_file) {
  using namespace kjson;

  AuthData auth{};

  if (ValidateAuthJSON(json_file)) {
    auth.access_token  = GetJSONStringValue(json_file, "access_token");
    auth.refresh_token = GetJSONStringValue(json_file, "refresh_token");
    auth.token_type    = GetJSONStringValue(json_file, "token_type");
    auth.scope         = GetJSONStringValue(json_file, "scope");
    auth.key           = GetJSONStringValue(json_file, "key");
    auth.expires_in    = json_file.contains("expires_in") ?
                           std::to_string(GetJSONValue<uint32_t>(json_file, "expires_in")) :
                           std::to_string(GetJSONValue<uint32_t>(json_file, "expiry_date"));
    auth.client_id     = GetJSONStringValue(json_file, "client_id");
    auth.client_secret = GetJSONStringValue(json_file, "client_secret");
   }

  return auth;
}

static const std::string JSON_EMPTY_OBJECT{
  "{\"arbitratus\":[1, 2, 3]}"
};


class Authenticator {

public:

Authenticator()
: m_authenticated{false},
  m_tokens_json{nullptr},
  m_verify_ssl{true}
{
  auto type = m_tokens_json.type();
  auto config = GetConfigReader();

  if (config.ParseError() < 0) {
    log("Error loading config");
    throw std::invalid_argument{"No configuration path"};
  }

  auto app_path = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::YOUTUBE_TOKEN_APP, "");
  if (!app_path.empty()) {
    m_auth.token_app_path = app_path;
  }

  auto verify_ssl = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::VERIFY_SSL_KEY, "true");
  if (!verify_ssl.empty()) {
    m_verify_ssl = (verify_ssl == "true");
  }

  auto youtube_key = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::YOUTUBE_KEY, "");
  if (!youtube_key.empty()) {
    m_auth.key = youtube_key;
  }

  auto name = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::USER_CONFIG_KEY, "");

    if (name.empty()) {
      throw std::invalid_argument{"No username in config. Please provide a username"};
    }

    m_username = name;

  auto client_id = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::CLIENT_ID, "");
  if (!client_id.empty()) {
    m_auth.client_id = client_id;
  }

  auto client_secret = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::CLIENT_SECRET, "");
  if (!client_secret.empty()) {
    m_auth.client_secret = client_secret;
  }

  m_tokens_path = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::TOKENS_PATH_KEY, "");

  if (!m_tokens_path.empty())
    m_tokens_json = LoadJSONFile(m_tokens_path);
  else
    throw std::runtime_error{"Must provide a path to store tokens JSON"};

  auto tokens_str = m_tokens_json.get<std::string>();
  ktube::log("Tokens are:\n" + tokens_str);
  if (m_tokens_json.contains(m_username) && !m_tokens_json[m_username].is_null()) {
    auto auth = ParseAuthFromJSON(m_tokens_json[m_username]);

    if (auth.is_valid()) {
      m_auth.access_token = auth.access_token;
      m_auth.scope        = auth.scope;
      m_auth.token_type   = auth.token_type;
    }
  }

  auto refresh_token = config.GetString(constants::KTUBE_CONFIG_SECTION, constants::REFRESH_TOKEN, "");
  if (!refresh_token.empty()) {
    m_auth.refresh_token = refresh_token;
  }
}

/**
 * FetchToken
 *
 * @returns [out] {bool}
 */
bool FetchToken(const bool fetch_fresh_token = false) {
  using json = nlohmann::json;

  // Attempt to refresh token
  if (!fetch_fresh_token && !m_auth.refresh_token.empty()) {
    if (refresh_access_token())
      return true;
  }

  // Attempt to fetch a fresh token
  if (!m_auth.token_app_path.empty()) {
    // TODO: Replace this with our own HTTP request
    ProcessResult result = qx({m_auth.token_app_path});

    if (!result.error) {
      auto auth_json = json::parse(result.output, nullptr, false);
      auto auth = ParseAuthFromJSON(auth_json);

      if (auth.is_valid()) {
        m_auth.access_token = auth.access_token;
        m_auth.scope        = auth.scope;
        m_auth.token_type   = auth.token_type;
        m_auth.expires_in   = auth.expires_in;

        m_authenticated = true;

        m_tokens_json[m_username] = auth_json;
        SaveToFile(m_tokens_json.dump(), m_tokens_path);

        return true;
      }
    }
  }
  return false;
}

bool refresh_access_token() {
  using namespace constants;
  using json = nlohmann::json;

  RequestResponse response{cpr::Post(
    cpr::Url{URL_VALUES.at(GOOGLE_AUTH_URL_INDEX)},
    cpr::Header{
      {HEADER_NAMES.at(CONTENT_TYPE_INDEX), HEADER_VALUES.at(FORM_URL_ENC_INDEX)}
    },
    cpr::Body{
      std::string{
        PARAM_NAMES.at(CLIENT_ID_INDEX)          + "=" + m_auth.client_id + "&" +
        PARAM_NAMES.at(CLIENT_SECRET_INDEX)      + "=" + m_auth.client_secret + "&" +
        PARAM_NAMES.at(REFRESH_TOKEN_NAME_INDEX) + "=" + m_auth.refresh_token + "&" +
        PARAM_NAMES.at(GRANT_TYPE_INDEX)         + "=" + PARAM_VALUES.at(REFRESH_TOKEN_VALUE_INDEX)
      }
    },
    cpr::VerifySsl{m_verify_ssl}
  )};

  if (!response.error)
  {
    auto     auth_json = response.json();
    AuthData auth      = ParseAuthFromJSON(auth_json);

    if (auth.is_valid()) {
      m_auth.access_token       = auth.access_token;
      m_auth.expires_in         = auth.expires_in;
      m_auth.scope              = auth.scope;
      m_auth.token_type         = m_auth.token_type;
      m_tokens_json[m_username] = auth_json;
      m_authenticated           = true;

      SaveToFile(m_tokens_json.dump(), m_tokens_path);

      return true;
    }
  }
  else
  {
    ktube::log("Failed to refresh access token. Error:\n" + response.GetError());
  }

  return false;
}

bool is_authenticated() {
  return m_authenticated;
}

bool verify_ssl() {
  return m_verify_ssl;
}

std::string get_token() {
  return "Bearer " + m_auth.access_token;
}

std::string get_key() {
  return m_auth.key;
}

private:
using json = nlohmann::json;

AuthData     m_auth;
bool         m_authenticated;
std::string  m_username;
std::string  m_tokens_path;
json         m_tokens_json;
bool         m_verify_ssl;
};

} // namespace ktube
