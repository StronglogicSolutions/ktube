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
      {"order", "relevance"}
    }
  )};

  if (response.error)
    log("Error response from server:\n" + response.GetError()); // Container will be empty

  return ParseComments(response.json());
}

// TODO: Return comment id
std::string YouTubeDataAPI::PostCommentReply(const Comment& comment)
{
  using namespace constants;
  std::string comment_id{};

  RequestResponse response{cpr::Post(
    cpr::Url(URL_VALUES.at(COMMENT_REPLY_URL_INDEX)),
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

  m_quota += youtube::QUOTA_LIMIT.at(youtube::COMMENT_INSERT_QUOTA_INDEX);

  if (response.error)
    log("Error response from server:\n" + response.GetError());
  else
  {
    comment_id = response.json()["id"];
  }

  return comment_id;
}

// TODO: Return comment id
std::string YouTubeDataAPI::PostComment(const Comment& comment)
{
  using namespace constants;

  const bool  IS_NOT_REPLY{false};
  std::string comment_id{};

  RequestResponse response{cpr::Post(
    cpr::Url(URL_VALUES.at(COMMENT_THREADS_URL_INDEX)),
    cpr::Header{
      {HEADER_NAMES.at(ACCEPT_HEADER_INDEX), HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(CONTENT_TYPE_INDEX),  HEADER_VALUES.at(APP_JSON_INDEX)},
      {HEADER_NAMES.at(AUTH_HEADER_INDEX),   m_authenticator.get_token()     }
    },
    cpr::Parameters{
      {PARAM_NAMES.at(PART_INDEX),       PARAM_VALUES.at(SNIPPET_INDEX)}
    },
    cpr::Body{comment.postdata(IS_NOT_REPLY)}
  )};

  m_quota += youtube::QUOTA_LIMIT.at(youtube::COMMENT_INSERT_QUOTA_INDEX);

  if (response.error)
    log("Error response from server:\n" + response.GetError());
  else
  {
    comment_id = response.json()["id"];
  }

  return comment_id;
}

} // namespace ktube
