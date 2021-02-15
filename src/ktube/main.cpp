#include "ktube.hpp"


int main(int argc, char** argv)
{

  ktube::YouTubeDataAPI api{};

  api.init();

  bool has_auth = api.is_authenticated();

  return 0;
}
