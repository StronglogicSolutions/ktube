#include "ktube.hpp"


int main(int argc, char** argv)
{

  ktube::YouTubeDataAPI api{};
  conversation::NLP     nlp{api.GetUsername()};

  api.init();

  auto videos = api.fetch_channel_videos();
  // api.FetchLiveVideoID();
  // api.FetchLiveDetails();
  // api.FetchChatMessages();
  // api.PostMessage("Hi");

  // if (api.HasChats()) {
  //   api.ParseTokens();

  //   ktube::LiveMessages messages = api.FindMentions();

  //   if (!messages.empty())
  //     ktube::log("Bot was mentioned");

  //   for (const auto& message : messages)
  //     for (const auto& token : message.tokens)
  //       nlp.Insert(
  //         conversation::Message{.text = message.text, .received = false},
  //         message.author,
  //         token.value);
  // }

  // for (const auto& conv : nlp.GetConversations())
  //   ktube::log(conv.second->objective->toString());

  return 0;
}
