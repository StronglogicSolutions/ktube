#include "youtube.hpp"
#include "ktube/common/constants.hpp"

namespace ktube {
/**
 * @brief FetchVideoComments
 *
 * @param   [in]  {std::string} id
 * @param   [in]  {bool}        fetch_all       (optional)
 * @param   [in]  {bool}        include_replies (optional)
 * @returns [out]  std::vector<Comment>
 */
std::vector<Comment> YouTubeDataAPI::FetchVideoComments(const std::string& id)
{
  using namespace constants;

  RequestResponse response{cpr::Get(
    cpr::Url(URL_VALUES.at(COMMENT_THREADS_URL_INDEX)),
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()     }
    },
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)},
      {PARAM_NAMES.at(VIDEO_ID_INDEX),   id                            },
    }
  )};

  if (response.error)
    log("Error response from server:\n" + response.GetError()); // Container will be empty

  return ParseComments(response.json());
}

bool YouTubeDataAPI::PostCommentReply(const Comment& comment)
{
  using namespace constants;

  RequestResponse response{cpr::Post(
    cpr::Url(URL_VALUES.at(COMMENT_POST_URL_INDEX)),
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(CONTENT_TYPE_INDEX),  HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()     }
    },
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)}
    },
    cpr::Body{comment.postdata()}
  )};

  if (response.error)
  {
    log("Error response from server:\n" + response.GetError()); // Container will be empty
    return false;
  }

  return true;
}

} // namespace ktube
