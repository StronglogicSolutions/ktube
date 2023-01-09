#pragma once

#include <iostream>
#include <algorithm>

#include "process.hpp"
#include "ktube/api/results.hpp"

namespace ktube {
/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░░░ Helpers ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/
kiq::ProcessResult execute(std::string program, std::vector<std::string> argv = {});

std::vector<GoogleTrend> query_google_trends(std::vector<std::string> terms);

/**
 * Platform
 * @enum
 */
enum Platform {
  INSTAGRAM = 0x00,
  UNKNOWN   = 0x01
};

/**
 * @interface
 */
class PlatformSpecificComparator {
public:
  virtual ~PlatformSpecificComparator() {}
  virtual Platform get_type() = 0;
};

class VideoStudy {
public:
using Videos = std::vector<Video>;

struct VideoStudyResult {
Videos::const_iterator most_likes;
Videos::const_iterator most_dislikes;
Videos::const_iterator most_comments;
Videos::const_iterator top_view_score;
Videos::const_iterator top_like_score;
Videos::const_iterator top_dislike_score;
Videos::const_iterator top_comment_score;
};

VideoStudy(Videos videos);

const VideoStudyResult analyze();
const Videos::const_iterator most_liked() const;
const Videos::const_iterator most_controversial() const;
const Videos::const_iterator top_view_score() const;
const Videos::const_iterator top_like_score() const;
const Videos::const_iterator top_dislike_score() const;
const Videos::const_iterator top_comment_score() const;
Videos get_videos();

private:
double compute_view_score(Video v);
double compute_like_score(Video v);
double compute_dislike_score(Video v);
double compute_comment_score(Video v);

Videos m_videos;
};

using StudyMap   = std::unordered_map<std::string, VideoStudy>;
using ResultMap  = std::unordered_map<std::string, VideoStudy::VideoStudyResult>;
using ResultPair = std::pair<std::string, VideoStudy::VideoStudyResult>;

/**
 * VideoAnalyst
 *
 * @class
 */
class VideoAnalyst {
public:
/**
 * VideoAnalysis
 *
 * @struct
 */
struct VideoAnalysis {
ResultMap get_result_map() {
  return map;
}

std::string most_likes_key;
std::string most_dislikes_key;
std::string most_comments_key;
std::string best_viewscore_key;
std::string best_likescore_key;
std::string best_dislikescore_key;
std::string best_commentscore_key;

std::string most_likes_channel_name() {
  return most_likes_key;
}

std::string most_dislikes_channel_name() {
  return most_dislikes_key;
}

std::string most_comments_channel_name() {
  return most_comments_key;
}

std::string best_viewscore_channel_name() {
  return best_viewscore_key;
}

ResultMap map;
};

VideoAnalysis get_analysis();
void          analyze(StudyMap map);

private:
void find_maximums();
VideoAnalysis m_analysis;
StudyMap      m_map;

};

struct VideoCreatorComparison {
public:
using VideoAnalysis = VideoAnalyst::VideoAnalysis;

VideoCreatorComparison(StudyMap study_map);

void          analyze();
VideoAnalysis get_result();

private:
VideoAnalyst analyst;
StudyMap     map;
};


class ContentComparator : public PlatformSpecificComparator {
using Videos = VideoStudy::Videos;

public:
virtual Platform get_type() override;
bool add_content(std::string key, Videos videos);
const VideoCreatorComparison analyze() const;

private:
StudyMap m_map;
};

} // namespace ktube
