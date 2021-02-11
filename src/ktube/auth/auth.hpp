#pragma once

#include <process.hpp>
#include <INIReader.h>
#include <cpr/cpr.h>

#include "ktube/common/constants.hpp"
#include "ktube/common/util.hpp"

namespace ktube {
struct AuthData {
  std::string access_token;
  std::string refresh_token;
  std::string scope;
  std::string token_type;
  std::string expiry_date;
  std::string key;
  std::string token_app_path;
  std::string client_id;
  std::string client_secret;
};

class Authenticator {

public:

Authenticator() {
  std::string cwd = get_current_working_directory();
  std::string reader_path{cwd + "../" + constants::DEFAULT_CONFIG_PATH};
  INIReader   reader{reader_path};

  if (reader.ParseError() < 0) {
    log("Error loading config");
    throw std::invalid_argument{"No configuration path"};
  }

  auto app_path = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_TOKEN_APP, "");
  if (!app_path.empty()) {
    m_auth.token_app_path = app_path;
  }

  auto youtube_key = reader.GetString(constants::YOUTUBE_CONFIG_SECTION, constants::YOUTUBE_KEY, "");
  if (!youtube_key.empty()) {
    m_auth.key = youtube_key;
  }

  auto refresh_token = reader.GetString(constants::GOOGLE_CONFIG_SECTION, constants::REFRESH_TOKEN, "");
  if (!refresh_token.empty()) {
    m_auth.refresh_token = refresh_token;
    m_authenticated      = true;
  }

  auto client_id = reader.GetString(constants::GOOGLE_CONFIG_SECTION, constants::CLIENT_ID, "");
  if (!client_id.empty()) {
    m_auth.client_id = client_id;
  }

  auto client_secret = reader.GetString(constants::GOOGLE_CONFIG_SECTION, constants::CLIENT_SECRET, "");
  if (!client_secret.empty()) {
    m_auth.client_secret = client_secret;
  }
}

/**
 * FetchToken
 *
 * @returns [out] {bool}
 */
bool FetchToken() {
  using json = nlohmann::json;
  if (!m_auth.refresh_token.empty()) {
    return refresh_access_token();
  }

  if (!m_auth.token_app_path.empty()) {
    // TODO: Replace this with our own HTTP request
    ProcessResult result = qx({m_auth.token_app_path});

    if (!result.error) {
      json auth_json = json::parse(result.output);

      if (!auth_json.is_null() && auth_json.is_object()) {
        m_auth.access_token = auth_json["access_token"].dump();
        m_auth.scope        = auth_json["scope"].dump();
        m_auth.token_type   = auth_json["token_type"].dump();
        m_auth.expiry_date  = auth_json["expiry_date"].dump();

        // log("Fetched token successfully");
        m_authenticated = true;

        return true;
      }
    }
  }
  return false;
}

bool refresh_access_token() {
  using namespace constants;
  using json = nlohmann::json;

  cpr::Response r = cpr::Post(
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
    }
  );

  json response = json::parse(r.text);

  if (!response.is_null() && response.is_object()) {
    m_auth.access_token = response["access_token"].dump();
    m_auth.expiry_date  = response["expires_in"].dump();
    m_auth.scope        = response["scope"].dump();
    m_auth.token_type   = response["token_type"].dump();

    // log("Refreshed token successfully");
    m_authenticated = true;

    return true;
  }

  return false;
}

bool is_authenticated() {
  return m_authenticated;
}

std::string get_token() {
  return m_auth.access_token;
}

std::string get_key() {
  return m_auth.key;
}

private:
AuthData     m_auth;
bool         m_authenticated;
};

} // namespace ktube
