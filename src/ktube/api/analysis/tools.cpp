#include "tools.hpp"

namespace ktube {
kiq::ProcessResult execute(std::string program, std::vector<std::string> argv) {
  std::vector<std::string> runtime_arguments{};
  runtime_arguments.reserve(1 + argv.size());
  runtime_arguments.emplace_back(program);
  runtime_arguments.insert(runtime_arguments.end(), argv.begin(), argv.end());
  return kiq::qx(runtime_arguments, get_executable_cwd());
}
//-----------------------------------------------------------------------
std::vector<GoogleTrend> query_google_trends(std::vector<std::string> terms) {
  std::vector<std::string> argv;
  for (const auto& term : terms) argv.emplace_back(std::string{"-t=" + term});

  const kiq::ProcessResult result = execute(constants::TRENDS_APP, argv);
  if (result.error)
    throw std::runtime_error{"Error executing trends app"};

  return TrendsJSONResult{result.output}.get_result();
}

//-----------------------------------------------------------------------
VideoStudy::VideoStudy(Videos videos)
: m_videos(videos) {}

//-----------------------------------------------------------------------
const VideoStudy::VideoStudyResult VideoStudy::analyze()
{
  VideoStudyResult result{};

  for (auto& video : m_videos)
  {
    std::vector<std::string> keywords;
    if (video.stats.keywords.size() > 3)
      keywords.insert(keywords.end(), video.stats.keywords.begin(), video.stats.keywords.begin() + 3);
    else
      keywords = video.stats.keywords;

    video.stats.view_score    = compute_view_score   (video);
    video.stats.like_score    = compute_like_score   (video);
    video.stats.dislike_score = compute_dislike_score(video);
    video.stats.comment_score = compute_comment_score(video);
    video.stats.trends        = query_google_trends  (keywords);
  }


  if (!m_videos.empty())
  {
    std::cout << __PRETTY_FUNCTION__ << ": Implement most comments max counter" << std::endl;
    result.most_likes        = most_liked();
    result.most_dislikes     = most_controversial();
    result.most_comments     = m_videos.end();
    result.top_view_score    = top_view_score();
    result.top_like_score    = top_like_score();
    result.top_dislike_score = top_dislike_score();
    result.top_comment_score = top_comment_score();
  }

  return result;
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::most_liked() const
{
  return std::max_element(m_videos.begin(), m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return std::stoi(a.stats.likes) < std::stoi(b.stats.likes);
    });
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::most_controversial() const
{
  return std::max_element(m_videos.begin(), m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return std::stoi(a.stats.dislikes) < std::stoi(b.stats.dislikes);
    });
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::top_view_score() const
{
  return std::max_element(m_videos.begin(), m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return a.stats.view_score < b.stats.view_score;
    });
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::top_like_score() const
{
  return std::max_element(m_videos.begin(), m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return a.stats.like_score < b.stats.like_score;
    });
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::top_dislike_score() const
{
  return std::max_element(m_videos.begin(), m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return a.stats.dislike_score < b.stats.dislike_score;
    });
}
//-----------------------------------------------------------------------
const VideoStudy::Videos::const_iterator VideoStudy::top_comment_score() const
{
  return std::max_element(m_videos.begin(),m_videos.end(),
    [](const Video& a, const Video& b)
    {
      return a.stats.comment_score < b.stats.comment_score;
    });
}
//-----------------------------------------------------------------------
VideoStudy::Videos VideoStudy::get_videos()
{
  return m_videos;
}
//-----------------------------------------------------------------------
double VideoStudy::compute_view_score(Video v)
{
  int     views   = std::stoi(v.stats.views);
  int64_t delta_t = std::chrono::duration_cast<std::chrono::minutes>(
    get_datetime_delta(get_simple_datetime(), v.datetime)).count();

  return static_cast<double>(views * 1000 / delta_t);
}
//-----------------------------------------------------------------------
double VideoStudy::compute_like_score(Video v)
{
  return static_cast<double>(std::stof(v.stats.likes) / std::stof(v.stats.views));
}
//-----------------------------------------------------------------------
double VideoStudy::compute_dislike_score(Video v)
{
  return static_cast<double>(std::stof(v.stats.dislikes) / std::stof(v.stats.views));
}
//-----------------------------------------------------------------------
double VideoStudy::compute_comment_score(Video v)
{
  return static_cast<double>(std::stof(v.stats.comments) / std::stof(v.stats.views));
}
//-----------------------------------------------------------------------
VideoAnalyst::VideoAnalysis VideoAnalyst::get_analysis()
{
  return m_analysis;
}
//-----------------------------------------------------------------------
void VideoAnalyst::analyze(StudyMap map)
{
  m_map = map;
  for (auto&& [key, value] : m_map)
    m_analysis.map[key] = value.analyze();

  find_maximums();
}
//-----------------------------------------------------------------------
void VideoAnalyst::find_maximums()
{
  using StudyResult = VideoStudy::VideoStudyResult;

  const auto most_likes_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return std::stoi(a.second.most_likes->stats.likes) < std::stoi(b.second.most_likes->stats.likes);
    });

  if (most_likes_index != m_analysis.map.end())
    m_analysis.most_likes_key = most_likes_index->first;

  const auto most_dislikes_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return std::stoi(a.second.most_dislikes->stats.dislikes) < std::stoi(b.second.most_dislikes->stats.dislikes);
    });

  if (most_dislikes_index != m_analysis.map.end())
    m_analysis.most_dislikes_key = most_dislikes_index->first;

  const auto best_viewscore_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return a.second.top_view_score->stats.view_score < b.second.top_view_score->stats.view_score;
    });

  if (best_viewscore_index != m_analysis.map.end())
    m_analysis.best_viewscore_key = best_viewscore_index->first;

  const auto best_likescore_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return a.second.top_like_score->stats.like_score < b.second.top_like_score->stats.like_score;
    });

  if (best_likescore_index != m_analysis.map.end())
    m_analysis.best_likescore_key = best_likescore_index->first;

  const auto best_dislikescore_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return a.second.top_dislike_score->stats.dislike_score < b.second.top_dislike_score->stats.dislike_score;
    });

  if (best_dislikescore_index != m_analysis.map.end())
    m_analysis.best_dislikescore_key = best_dislikescore_index->first;

  const auto best_commentscore_index = std::max_element(m_analysis.map.begin(), m_analysis.map.end(),
    [](const ResultPair& a, const ResultPair& b)
    {
      return a.second.top_comment_score->stats.comment_score < b.second.top_comment_score->stats.comment_score;
    });

  if (best_commentscore_index != m_analysis.map.end())
    m_analysis.best_commentscore_key = best_commentscore_index->first;
}
//-----------------------------------------------------------------------
VideoCreatorComparison::VideoCreatorComparison(StudyMap study_map)
: map(study_map) {}
//-----------------------------------------------------------------------
void VideoCreatorComparison::analyze()
{
  analyst.analyze(map);
}
//-----------------------------------------------------------------------
VideoAnalyst::VideoAnalysis VideoCreatorComparison::get_result()
{
  return analyst.get_analysis();
}
//-----------------------------------------------------------------------
Platform ContentComparator::get_type()
{
  return Platform::INSTAGRAM;
}
//-----------------------------------------------------------------------
bool ContentComparator::add_content(std::string key, Videos videos)
{
  if (m_map.find(key) != m_map.end())
    return false;

  m_map.insert({key, VideoStudy{videos}});
  return true;
}
//-----------------------------------------------------------------------
const VideoCreatorComparison ContentComparator::analyze() const
{
  VideoCreatorComparison comparison{m_map};
  comparison.analyze();
  return comparison;
}

} // namespace ktube
