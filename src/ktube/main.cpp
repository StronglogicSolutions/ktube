#include "ktube.hpp"


int main(int argc, char** argv)
{

  ktube::YouTubeDataAPI api{};

  bool has_auth = api.is_authenticated();

  api.init();

  has_auth = api.is_authenticated();

  return 0;
}
