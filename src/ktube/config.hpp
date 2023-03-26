#pragma once

#include <string>
#include <vector>

namespace ktube {
enum class Language
{
  english = 0x00,
  korean = 0x01
};
//-----------------------------------------------------------------------
struct ExecuteConfig
{
  std::string              message;
  std::vector<std::string> file_paths;
  std::string              description;
  bool                     execute_bot;
  std::string              username;
  Language                 language = Language::korean;
};
//-----------------------------------------------------------------------
static ExecuteConfig ParseRuntimeArguments(int argc, char** argv)
{
  ExecuteConfig config{};

  for (int i = 1; i < argc; i++)
  {
    std::string argument = SanitizeInput(argv[i]);
    if (argument.find("--header") == 0)
    {
      config.message = argument.substr(9);
      continue;
    }
    else
    if (argument.find("--description") == 0)
    {
      config.description = argument.substr(14);
      continue;
    }
    else
    if (argument.find("--filename") == 0)
    {
      config.file_paths.emplace_back(argument.substr(11));
      continue;
    }
    else
    if (argument.find("--username") == 0)
    {
      config.username = argument.substr(11);
      continue;
    }
    else
    if (argument.find("--execute_bot") == 0)
    {
      config.execute_bot = (argument.substr(14).compare("true") == 0);
      continue;
    }
    else
    if (argument.find("--language") == 0)
    {
      config.language = (argument.substr(11) == "english") ? Language::english : Language::korean;
    }
  }

  return config;
}
//-----------------------------------------------------------------------
struct BotStats
{
  uint32_t tx_msg{};
  uint32_t rx_msg{};
  uint32_t tx_err{};
  uint32_t rx_err{};

  std::string to_string() const
  {
    return "\nTX msg: " + std::to_string(tx_msg) +
           "\nRX msg: " + std::to_string(rx_msg) +
           "\nTX err: " + std::to_string(tx_err) +
           "\nRX err: " + std::to_string(rx_err);
  }
};
} // ns ktube
