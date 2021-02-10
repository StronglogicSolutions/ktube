#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <ctype.h>
#include <chrono>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <nlohmann/json.hpp>

namespace constants {
static const char* SIMPLE_DATE_FORMAT{"%Y-%m-%dT%H:%M:%S"};
} // namespace constants

/**
 * SaveToFile
 */
inline void SaveToFile(std::string data, std::string path) {
  std::ofstream o{path};
  o << data;
}

/**
 * ReadFromFile
 */
inline std::string ReadFromFile(std::string path) {
  std::ifstream f{path};
  std::stringstream fs{};
  fs << f.rdbuf();
  return fs.str();
}

inline std::string system_read(std::string command) {
  std::system(std::string{command + "> file.txt"}.c_str());
  std::stringstream stream{};
  stream << std::ifstream("file.txt").rdbuf();
  return stream.str();
}

/**
 * Poor man's log
 *
 * @param
 */
template<typename T>
inline void log(T s) {
  std::cout << s << std::endl;
}

inline std::string SanitizeOutput(const std::string& s) {
  std::string o{};

  for (const char& c : s) {
    if (c == '\'')
      o += "\'";
    // else
    // if (c == '"')
    //   o += "\\\"";
    else
    if (c == '(')
      o += "&#x28;";
    else
    if (c == ')')
      o += "&#x29;";
    else
      o += c;
  }

  return o;
}

/**
 * SanitizeJSON
 *
 * Helper function to remove escaped double quotes from a string
 *
 * @param   [in] {std::string}
 * @returns [in] {std::string}
 */
inline std::string SanitizeJSON(std::string s) {
  s.erase(
    std::remove(s.begin(), s.end(),'\"'),
    s.end()
  );
  return s;
}

/**
 * SanitizeInput
 *
 * Helper function to remove quotes from a string
 *
 * @param   [in] {std::string}
 * @returns [in] {std::string}
 */
inline std::string SanitizeInput(std::string s) {
  s.erase(
    std::remove_if(s.begin(), s.end(), [](char c){
      return c == '\'' || c == '\"';
    }),
  s.end());

  return s;
}

/**
 * SanitizeJSON
 *
 * Helper function to remove escaped double quotes from a string
 *
 * @param [in] {std::string&} A reference to a string object
 */
inline std::string StripLineBreaks(std::string s) {
  s.erase(
    std::remove(s.begin(), s.end(),'\n'),
    s.end()
  );

  return s;
}

/**
 * CreateStringWithBreaks
 *
 * @param
 * @param
 * @returns
 */
inline std::string CreateStringWithBreaks(const std::string &in, const size_t every_n) {
  std::string out{};
  out.reserve(in.size() + in.size() / every_n);
  for(std::string::size_type i = 0; i < in.size(); i++) {
    (isascii(static_cast<uint8_t>(in[i]))) ?
    (!(i % every_n) && i) ? out.push_back('\n') : out.push_back(in[i]) :
    (void)(out);
  }
  return out;
}

/**
 * to_unixtime
 *
 * @param
 * @returns
 */
inline const std::time_t to_unixtime(const char* datetime) {
  std::tm            t{};
  std::istringstream ss{datetime};

  ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

  return mktime(&t);
}

/**
 * to_readable_time
 *
 * @param
 * @returns
 */
inline std::string to_readable_time(const char* datetime) {
  uint8_t            buffer_size{24};
  std::tm            t{};
  std::istringstream ss{datetime};
  char               b[buffer_size];
  ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

  strftime(b, buffer_size, "%B %d %H:%M:%S", &t);

  return std::string{b};
}

/**
 * to_readable_time
 *
 * @param
 * @returns
 */
inline std::string to_readable_time(const std::string datetime) {
  uint8_t            buffer_size{24};
  std::tm            t{};
  std::istringstream ss{datetime};
  char               b[buffer_size];
  ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

  strftime(b, buffer_size, "%B %d %H:%M:%S", &t);

  return std::string{b};
}

inline std::string get_simple_datetime() {
  uint8_t            buffer_size{24};
  char               b[buffer_size];
  auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  struct tm tm{};
  if (::gmtime_r(&now, &tm))
    if (std::strftime(b, sizeof(b), constants::SIMPLE_DATE_FORMAT, &tm))
      return std::string{b};
  throw std::runtime_error{"Failed to get current date as string"};
}

// template <typename T = float>
inline std::string human_readable_duration(std::chrono::duration<int64_t> delta) {
  using namespace std;
  using namespace std::chrono;
  using days = duration<int, ratio<86400>>;

  std::stringstream ss{};

  char fill = ss.fill();
  ss.fill('0');
  auto d = duration_cast<days>(delta);
  delta -= d;
  auto h = duration_cast<hours>(delta);
  delta -= h;
  auto m = duration_cast<minutes>(delta);
  delta -= m;
  auto s = duration_cast<seconds>(delta);

  ss  << setw(2) << d.count() << "d:"
      << setw(2) << h.count() << "h:"
      << setw(2) << m.count() << "m:"
      << setw(2) << s.count() << 's';

  ss.fill(fill);

  return ss.str();
};


// template <typename T = float>
inline std::string delta_to_string(std::chrono::duration<int64_t, std::nano> d) {
  return human_readable_duration(std::chrono::duration_cast<std::chrono::seconds>(d));
}

// template <typename T = float>
inline std::chrono::duration<int64_t, std::nano> get_datetime_delta(std::string dt1, std::string dt2) {
  std::tm            t{};
  std::istringstream ss{dt1};
  ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

  std::chrono::time_point tp_1 = std::chrono::system_clock::from_time_t(mktime(&t));

  ss.clear();

  ss.str(dt2);
  ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

  std::chrono::time_point tp_2 = std::chrono::system_clock::from_time_t(mktime(&t));

  std::chrono::duration<int64_t, std::nano> elapsed = tp_1 - tp_2;
  return elapsed;
}

// template <typename T = float>
inline std::string datetime_delta_string(std::string dt1, std::string dt2) {
  std::chrono::duration<int64_t, std::nano> datetime_delta = get_datetime_delta(dt1, dt2);
  return delta_to_string(datetime_delta);
}

#endif // __UTIL_HPP__
