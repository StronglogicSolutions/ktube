#include "ktube.hpp"


int main(int argc, char** argv)
{

  ktube::YouTubeDataAPI api{};

  api.init();

  bool has_auth = api.is_authenticated();
  api.FetchLiveVideoID();
  api.FetchLiveDetails();
  api.FetchChatMessages();
  api.PostMessage("Hello");

  return 0;
}
