#include "ktube.hpp"


int main(int argc, char** argv)
{

  ktube::YouTubeDataAPI api{};

  api.init();

  bool has_auth = api.is_authenticated();

  bool did_fetch = api.fetch_channel_videos();

  std::vector<ktube::VideoInfo> videos = api.get_videos();

  return 0;
}
