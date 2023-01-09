#include "auth.hpp"

namespace ktube {

bool AuthData::is_valid() const
{
  return (!access_token.empty() &&
          !token_type  .empty() &&
          !scope       .empty());

}

static bool ValidateAuthJSON(const nlohmann::json& json_file)
{
  return(
    !json_file.is_null()               &&
    json_file.is_object()              &&
    json_file.contains("access_token") &&
    json_file.contains("token_type")   &&
    json_file.contains("scope")
  );
}

static AuthData ParseAuthFromJSON(nlohmann::json json_file) {
  using namespace kjson;

  AuthData auth;

  if (ValidateAuthJSON(json_file))
  {
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

static std::string get_config(const std::string& name, const std::string& default_value = "")
{
  static const auto config = GetConfigReader();
  if (config.ParseError() < 0)
    throw std::invalid_argument{"No configuration path"};
  return config.GetString(constants::KTUBE_CONFIG_SECTION, name, default_value);
}


Authenticator::Authenticator()
: m_authenticated{false},
  m_tokens_json{nullptr},
  m_verify_ssl{true}
{
  m_auth.token_app_path = get_config(constants::YOUTUBE_TOKEN_APP);
  m_verify_ssl          = get_config(constants::VERIFY_SSL_KEY, "true") == "true";
  m_auth.key            = get_config(constants::YOUTUBE_KEY);
  m_username            = get_config(constants::USER_CONFIG_KEY);
  m_auth.client_id      = get_config(constants::CLIENT_ID);
  m_auth.client_secret  = get_config(constants::CLIENT_SECRET);
  m_auth.refresh_token  = get_config(constants::REFRESH_TOKEN);
  m_tokens_path         = get_config(constants::TOKENS_PATH_KEY);
  m_tokens_json         = LoadJSONFile(m_tokens_path);

  if (!m_tokens_json.contains(m_username) || m_tokens_json[m_username].is_null())
    return;

  if (const auto auth = ParseAuthFromJSON(m_tokens_json[m_username]); auth.is_valid())
  {
    m_auth.access_token = auth.access_token;
    m_auth.scope        = auth.scope;
    m_auth.token_type   = auth.token_type;
  }
}

bool Authenticator::FetchToken(const bool fetch_fresh_token)
{
  using json = nlohmann::json;

  if (!fetch_fresh_token && !m_auth.refresh_token.empty() && refresh_access_token())
    return true;

  if (!m_auth.token_app_path.empty())
  {
    if (kiq::ProcessResult result = kiq::qx({m_auth.token_app_path}); !result.error)
    {
      const auto auth_json = json::parse(result.output, nullptr, false);
      if (const auto auth = ParseAuthFromJSON(auth_json); auth.is_valid())
      {
        m_auth.access_token       = auth.access_token;
        m_auth.scope              = auth.scope;
        m_auth.token_type         = auth.token_type;
        m_auth.expires_in         = auth.expires_in;
        m_authenticated           = true;
        m_tokens_json[m_username] = auth_json;
        SaveToFile(m_tokens_json.dump(), m_tokens_path);

        return true;
      }
    }
  }
  return false;
}

bool Authenticator::refresh_access_token()
{
  using namespace constants;
  using json = nlohmann::json;

  cpr::Response response;

  try
  {
    response = cpr::Post(
      cpr::Url{URL_VALUES.at(GOOGLE_AUTH_URL_INDEX)},
      cpr::Header{
        {HEADER_NAMES.at(CONTENT_TYPE_INDEX), HEADER_VALUES.at(FORM_URL_ENC_INDEX)}
      },
      cpr::VerifySsl(m_verify_ssl),
      cpr::Body{
        PARAM_NAMES.at(CLIENT_ID_INDEX)          + "=" + m_auth.client_id + "&" +
        PARAM_NAMES.at(CLIENT_SECRET_INDEX)      + "=" + m_auth.client_secret + "&" +
        PARAM_NAMES.at(REFRESH_TOKEN_NAME_INDEX) + "=" + m_auth.refresh_token + "&" +
        PARAM_NAMES.at(GRANT_TYPE_INDEX)         + "=" + PARAM_VALUES.at(REFRESH_TOKEN_VALUE_INDEX)});
  }
  catch (const std::exception& e)
  {
    ktube::log("Exception thrown while fetching refresh token:");
    ktube::log(e.what());
  }

  if (response.error.code == cpr::ErrorCode::OK)
  {
    auto     auth_json = json::parse(response.text);
    if (const auto auth = ParseAuthFromJSON(auth_json); auth.is_valid())
    {
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
    ktube::log("Failed to refresh access token. Error:\n" + response.error.message);

  return false;
}

bool Authenticator::is_authenticated()
{
  return m_authenticated;
}

bool Authenticator::verify_ssl()
{
  return m_verify_ssl;
}

std::string Authenticator::get_token() const
{
  return "Bearer " + m_auth.access_token;
}

std::string Authenticator::get_key() const
{
  return m_auth.key;
}

} // namespace ktube
