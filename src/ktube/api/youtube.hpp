#pragma once

#include <iostream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <INIReader.h>

#include "interface.hpp"
#include "nlp/nlp.hpp"
#include "ktube/auth/auth.hpp"
#include "analysis/html.hpp"
#include "analysis/tools.hpp"

using json = nlohmann::json;
namespace ktube {
const std::string CreateLocationResponse(std::string location);
const std::string CreatePersonResponse(std::string name);
const std::string CreateOrganizationResponse(std::string name);
const std::string CreatePromoteResponse(bool test_mode = false);

class YouTubeDataAPI : public SecureAPI,
                       public VideoAPI,
                       public LiveAPI
{

public:
  YouTubeDataAPI();

  virtual bool                     is_authenticated()                                    override;
  virtual bool                     init()                                                override;
  virtual bool                     has_videos()                                          override;

  /** Analytics API **/
  virtual bool                     fetch_channel_data()                                  override;
  virtual std::vector<ChannelInfo> fetch_channel_info(std::string id_string)             override;
  virtual bool                     fetch_channel_videos()                                override;
  virtual std::vector<VideoStats>  fetch_video_stats(std::string id_string)              override;
  virtual std::vector<ChannelInfo> fetch_youtube_stats()                                 override;
  virtual std::vector<VideoInfo>   fetch_rival_videos(VideoInfo video)                   override;
  virtual std::vector<ChannelInfo> find_similar_videos(VideoInfo video)                  override;
          std::vector<VideoInfo>   get_videos();
  virtual std::vector<GoogleTrend> fetch_google_trends(std::vector<std::string> terms)   override;
  virtual std::vector<TermInfo>    fetch_term_info(std::vector<std::string> terms)       override;
  virtual std::vector<VideoInfo>   fetch_videos_by_terms(std::vector<std::string> terms) override;

          const   uint32_t         get_quota_used() const;
  /** Livechat API **/
  virtual std::string              FetchLiveVideoID()                                    override;
  virtual bool                     FetchLiveDetails()                                    override;
  virtual std::string              FetchChatMessages()                                   override;
          std::string              GetUsername() { return m_username; }
          VideoDetails             GetLiveDetails();
          LiveChatMap              GetChats();
          LiveMessages             GetCurrentChat(bool keep_messages = false);
          LiveMessages             FindMentions(bool keep_messages = false);

          bool                     FindChat();
          bool                     HasChats();
          bool                     ClearChat(std::string id = "");
          bool                     PostMessage(std::string message);
          bool                     ParseTokens();
          bool                     InsertMessages(std::string id, LiveMessages&& messages);
          bool                     GreetOnEntry();
          bool                     HasInteracted(std::string id, Interaction interaction);
          bool                     HasDiscussed(std::string value, Interaction type);
          void                     RecordInteraction(std::string id,
                                                     Interaction interaction,
                                                     std::string value);
          void                     SetChatMap(LiveChatMap chat_map) { m_chats = chat_map; }
          void                     SetVideoDetails(VideoDetails video_details) { m_video_details = video_details; }

protected:
LiveChatMap  m_chats;

private:
  bool                IsNewer(const char* datetime);

  Authenticator            m_authenticator;
  std::vector<VideoInfo>   m_videos;
  uint32_t                 m_quota;
  std::vector<std::string> m_channel_ids;
  std::vector<ChannelInfo> m_channels;
  VideoDetails             m_video_details;
  ActivityMap              m_interactions;
  LocationMap              m_locations;
  PersonMap                m_persons;
  OrgMap                   m_orgs;
  std::string              m_active_chat;
  std::string              m_username;
  std::time_t              m_last_fetch_timestamp;
  bool                     m_greet_on_entry;
  bool                     m_test_mode;
  bool                     m_retry_mode;
};

} // namespace ktube
