#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <locale>
#include <optional>
#include <string>

/// Enum class that holds possible error values.
enum class Err {
  NONE = 0,
  CMD_ARGS,
  FILE_IO,
  HTML_CONVERSION,
  TEMPLATE_FILL,
};

/// Read contents of file located at `path`.  Returns none on failure.
std::optional<std::string> fetchFileContents(const std::filesystem::path &path);

static inline void lTrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

static inline void rTrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

static inline void trim(std::string &s) {
  rTrim(s);
  lTrim(s);
}

static inline std::string copyAndLTrim(std::string s) {
  lTrim(s);
  return s;
}

static inline std::string copyAndRTrim(std::string s) {
  rTrim(s);
  return s;
}

static inline std::string copyAndTrim(std::string s) {
  trim(s);
  return s;
}
