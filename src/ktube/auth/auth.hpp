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

bool is_valid() const;
};

bool     ValidateAuthJSON(const nlohmann::json& json_file);
AuthData ParseAuthFromJSON(nlohmann::json json_file);

class Authenticator {

public:

  Authenticator();
  bool FetchToken(const bool fetch_fresh_token = false);
  bool refresh_access_token();
  bool is_authenticated();
  bool verify_ssl();
  std::string get_token() const;
  std::string get_key()   const;

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
