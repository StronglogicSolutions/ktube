#pragma once

#include "ktube/common/types.hpp"

namespace ktube {
class SecureAPI {
public:
virtual ~SecureAPI() {}

virtual bool is_authenticated() = 0;
virtual bool init()             = 0;
};

class VideoAPI {
public:
virtual ~VideoAPI() {}

virtual std::vector<VideoInfo> get_videos() = 0;
virtual bool                   has_videos() = 0;
};
} // namespace ktube
