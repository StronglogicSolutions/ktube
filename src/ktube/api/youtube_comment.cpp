#include "youtube.hpp"
#include "ktube/common/constants.hpp"

namespace ktube {

std::vector<Comment> YouTubeDataAPI::FetchVideoComments(const std::string& id)
{
  using namespace constants;

  const std::string URL = URL_VALUES.at(COMMENT_THREADS_URL_INDEX);

  cpr::Response r = cpr::Get(
    cpr::Url(URL),
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()}},
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)},    // snippet
      {PARAM_NAMES.at(VIDEO_ID_INDEX),   id},
    }
  );

  if (r.error.code == cpr::ErrorCode::OK)
  {
    // TODO: parse
  }

  return std::vector<Comment>{};
}

} // namespace ktube
