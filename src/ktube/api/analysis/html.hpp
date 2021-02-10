#pragma once

#include <HTML/HTML.h>
#include "ktube/common/types.hpp"
#include "ktube/common/util.hpp"

namespace ktube {
/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░░ CONSTANTS ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/
namespace constants
{
  const std::string HTML_STYLE{
    ".container{background-color:#333;}.container h1,.container h2,.container th{color:#ef5e3f;}"
    ".container td{color: #FFF;}}"
  };
  const std::string HTML_COL_HEADER_STYLE{
    "color:#ef5e3f; padding: 12px;text-align: center;"
  };
  const std::string HTML_COL_VALUE_STYLE{
    "color: #000; padding: 4px; text-align: center"
  };
} // namespace constants


/**
  ┌───────────────────────────────────────────────────────────┐
  │░░░░░░░░░░░░░░░░░░░░░░░░░░ FUNCTIONS ░░░░░░░░░░░░░░░░░░░░░░░│
  └───────────────────────────────────────────────────────────┘
*/

/**
 * keywords_from_string
 *
 * @param
 * @returns
 */
inline std::vector<std::string> keywords_from_string(std::string s)
{
  std::vector<std::string> keywords;

  if (s.size() > 2)
  {
    s = s.substr(1, s.size() - 2);
    size_t start;
    size_t end{0};
    auto delim = ",";

    while ((start = s.find_first_not_of(delim, end)) != std::string::npos)
    {
      end = s.find(delim, start);
      keywords.push_back(s.substr(start, end - start));
    }
  }
  return keywords;
}

/**
 * youtube_id_to_url
 *
 * @param
 * @returns
 */
inline std::string youtube_id_to_url(std::string id)
{
  return std::string{
      "https://youtube.com/watch?v=" + id};
}

/**
 * youtube_title_link
 *
 * @param title
 * @param video_id
 * @return HTML::Link
 */
inline std::string youtube_title_link(std::string title, std::string video_id) {
  return HTML::Link{title, youtube_id_to_url(video_id)}.toString();
}

/**
 * tags_to_string
 *
 * @param
 * @returns
 */
inline std::string tags_to_string(std::vector<std::string> tags)
{
  std::string s{};
  s.reserve(tags.size() * 9);

  for (const auto &tag : tags)
    s += '#' + tag + "  ";

  return s;
}

/**
 * counts_to_html
 *
 * @param
 * @returns
 */
inline std::string counts_to_html(std::vector<FollowerCount> counts) {
  HTML::Document document{"KIQ Analytics"};

  document.addAttribute("lang", "en");
  document.head() << HTML::Meta("utf-8")
                  << HTML::Meta("viewport", "width=device-width, initial-scale=1, shrink-to-fit=no");
  document.head() << HTML::Style(".navbar{margin-bottom:20px;}");
  document.body().cls("follower-counts");

  HTML::Div main{"container"};
  main << HTML::Header1("KIQ Analytics");
  main << HTML::Header2("Follower Counts") << HTML::Break() << HTML::Break();

  HTML::Table table{};
  table.cls("table");
  table << HTML::Caption{"Results"};
  table << (HTML::Row() <<
    HTML::ColHeader("Name")  << HTML::ColHeader("Platform") <<
    HTML::ColHeader("Count") << HTML::ColHeader("CountΔ")   << HTML::ColHeader("TimeΔ")
  );

  for (const auto& count : counts)
    table << (HTML::Row() << HTML::Col(count.name) << HTML::Col(count.platform) << HTML::Col(count.value) << HTML::Col(count.delta_v) << HTML::Col(count.delta_t));

  main     << std::move(table);
  document << std::move(main);

  return SanitizeOutput(document.toString());
}

/**
 * videos_to_html
 *
 * @param
 * @returns
 */
inline std::string channel_videos_to_html(const std::vector<ChannelInfo> &channels)
{
  HTML::Document document{"KIQ Analytics"};

  document.addAttribute("lang", "en");
  document.head() << HTML::Meta("utf-8")
                  << HTML::Meta("viewport", "width=device-width, initial-scale=1, shrink-to-fit=no");
  document.head() << HTML::Style(constants::HTML_STYLE);
  document.body().cls("videos");

  HTML::Div main{"container"};
  main.style("background-color:#FFF;");
  main << HTML::Header1("KIQ Analytics").style("color:#ef5e3f; padding: 12px;text-align: center;");
  main << HTML::Header2("Video Statistics").style("color:#ef5e3f; padding: 12px;text-align: center;");

  HTML::Table table{};
  table.cls("table");
  table << (HTML::Row() << HTML::ColHeader("Channel")    .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Subscribers").style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Title")      .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Time")       .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Views")      .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Likes")      .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Boos")       .style(constants::HTML_COL_HEADER_STYLE)
                        << HTML::ColHeader("Comments")   .style(constants::HTML_COL_HEADER_STYLE));

  for (const auto& channel : channels)
  {
    for (const auto& video : channel.videos)
    {
      table << (HTML::Row()
                << HTML::Col(channel.name)             .style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(channel.stats.subscribers).style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(youtube_title_link(video.title, video.id)).style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(video.time)               .style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(video.stats.views)        .style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(video.stats.likes)        .style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(video.stats.dislikes)     .style(constants::HTML_COL_VALUE_STYLE)
                << HTML::Col(video.stats.comments)     .style(constants::HTML_COL_VALUE_STYLE)
      );
      table << (HTML::Row()
                << HTML::Col(tags_to_string(video.stats.keywords))
                                                        .style("color: #333; padding: 8px;")
                                                        .addAttribute("rowspan", 1)
                                                        .addAttribute("colspan", 10)
      );
    }
  }

  main     << std::move(table);
  main     << HTML::Break() << HTML::Break();
  document << std::move(main);

  return SanitizeOutput(document.toString());
}

/**
 * channels_to_html
 *
 * TODO: Decide if we want to keep this
 *
 * @param
 * @returns
 */
inline std::string channels_to_html(const std::vector<ChannelInfo> &channels)
{
  HTML::Document document{"KIQ Analytics"};

  // document.addAttribute("lang", "en");
  // document.head() << HTML::Meta("utf-8")
  //                 << HTML::Meta("viewport", "width=device-width, initial-scale=1, shrink-to-fit=no");
  // document.head() << HTML::Style(constants::HTML_STYLE);
  // document.body().cls("videos");

  // HTML::Div main{"container"};
  // main.style("background-color:#FFF;");
  // main << HTML::Header1("KIQ Analytics").style("color:#ef5e3f; padding: 12px;text-align: center;");
  // main << HTML::Header2("Video Statistics").style("color:#ef5e3f; padding: 12px;text-align: center;");

  // HTML::Table table{};
  // table.cls("table");
  // table << (HTML::Row() << HTML::ColHeader("ID").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Title").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Time").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Views").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Likes").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Dislikes").style("color:#ef5e3f; padding: 4px;")
  //                       << HTML::ColHeader("Comments").style("color:#ef5e3f; padding: 4px;"));

  // for (const auto &video : videos)
  // {
  //   table << (HTML::Row()
  //             << HTML::Col(video.id).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.title).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.time).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.stats.views).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.stats.likes).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.stats.dislikes).style("color: #000; padding: 8px;")
  //             << HTML::Col(video.stats.comments).style("color: #000; padding: 8px;"));
  //   table << (HTML::Row()
  //             << HTML::Col(tags_to_string(video.stats.keywords)).style("color: #333; padding: 8px;").addAttribute("rowspan", 1).addAttribute("colspan", 10));
  // }

  // main << std::move(table);
  // main << HTML::Break() << HTML::Break();
  // document << std::move(main);

  return SanitizeOutput(document.toString());
}

} // namespace ktube
