#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <regex>
#include <sstream>
#include <unordered_map>

#include "html.h"
#include "md4c-html.h"
#include "util.h"

struct replaceInfo {
  long long position;
  long long length;
  std::string value;
};

static std::vector<replaceInfo> computeReplacements(
    const std::string &templateText,
    const std::unordered_map<std::string, const char *> &values) {
  auto foldFn = [&values](std::vector<replaceInfo> replacements,
                          std::smatch match) {
    auto key = copyAndTrim(std::string{match[1]});
    if (auto search = values.find(key); search != values.end()) {
      replacements.push_back({static_cast<long long>(match.position()),
                              static_cast<long long>(match.length()),
                              search->second});
      return replacements;
    }
    return replacements;
  };

  auto placeHolderRegex = std::regex{R"(\{\{([^\}]*)\}\})"};
  auto begin = std::sregex_iterator(templateText.begin(), templateText.end(),
                                    placeHolderRegex);

  return std::accumulate(begin, std::sregex_iterator(),
                         std::vector<replaceInfo>{}, foldFn);
}

static std::optional<std::string>
fillTemplate(const std::string &templateText,
             const std::unordered_map<std::string, const char *> &values) {

  auto cursor = 0LL;
  auto stream = std::stringstream{};
  for (const auto &info : computeReplacements(templateText, values)) {
    // First, add characters from the last cursor to info.position.
    stream << templateText.substr(cursor, info.position - cursor);
    cursor = info.position;

    // Then add the replacement string.
    stream << info.value;

    // Finally, skip the cursor ahead based on placeholder string.
    cursor += info.length;
  }

  // Lastly, add all characters from the last matched placeholder to the end of
  // the string.
  stream << templateText.substr(cursor, std::string::npos);
  return stream.str();
}

static std::optional<std::string>
translateMarkDownToHtml(const std::string &text) {
  auto processOutput = [](const MD_CHAR *text, MD_SIZE size, void *userData) {
    auto stream = static_cast<std::stringstream *>(userData);
    *stream << std::string{text, size};
  };

  auto flags = MD_FLAG_COLLAPSEWHITESPACE | MD_FLAG_TABLES | MD_FLAG_TASKLISTS |
               MD_FLAG_STRIKETHROUGH | MD_FLAG_NOHTMLSPANS |
               MD_FLAG_NOHTMLBLOCKS | MD_FLAG_NOINDENTEDCODEBLOCKS;

  auto stream = std::stringstream{};
  auto status =
      md_html(text.c_str(), static_cast<MD_SIZE>(text.length()), processOutput,
              static_cast<void *>(&stream), flags, MD_HTML_FLAG_XHTML);
  if (status == 0) {
    return stream.str();
  }

  return {};
}

std::optional<std::string> renderText(const std::string &markDownText,
                                      const std::string &templateText,
                                      bool silent) {
  auto maybeHtml = translateMarkDownToHtml(markDownText);
  if (!maybeHtml) {
    if (!silent) {
      std::cerr << "failed to convert markdown to HTML for file: <stdin>"
                << std::endl;
    }
    return {};
  }

  auto values = std::unordered_map<std::string, const char *>{
      {"body", maybeHtml->c_str()},
  };

  auto maybeFilled = fillTemplate(templateText, values);
  if (!maybeFilled) {
    if (!silent) {
      std::cerr << "failed to populate HTML template for file: <stdin>"
                << std::endl;
    }
    return {};
  }

  return *maybeFilled;
}

std::optional<std::string> renderFile(const std::filesystem::path &path,
                                      const std::string &templateText,
                                      bool silent) {
  if (std::filesystem::status(path).type() !=
          std::filesystem::file_type::regular &&
      std::filesystem::status(path).type() !=
          std::filesystem::file_type::symlink) {
    if (!silent) {
      std::cerr << "not a regular file or symlink: " << path << std::endl;
    }
    return {};
  }

  auto maybeContent = fetchFileContents(path);
  if (!maybeContent) {
    if (!silent) {
      std::cerr << "failed to read file: " << path << std::endl;
    }
    return {};
  }

  return renderText(*maybeContent, templateText);
}

struct dirEntry {
  std::string uri;
  std::string name;
};

std::ostream &operator<<(std::ostream &os, const dirEntry &entry) {
  os << "| [" << entry.name << "](" << entry.uri << "/" << entry.name << ") | "
     << "|" << std::endl;
  return os;
}

std::optional<std::string> renderDirectory(const std::string &uri,
                                           const std::filesystem::path &path,
                                           const std::string &templateText,
                                           bool silent) {
  if (std::filesystem::status(path).type() !=
      std::filesystem::file_type::directory) {
    if (!silent) {
      std::cerr << "not a directory: " << path << std::endl;
    }
    return {};
  }

  auto foldFn = [&uri](std::vector<dirEntry> acc,
                       const std::filesystem::directory_entry &entry) {
    acc.push_back({uri, entry.path().filename().string()});
    return acc;
  };

  auto it = std::filesystem::directory_iterator(path);
  auto entries =
      std::accumulate(std::filesystem::begin(it), std::filesystem::end(it),
                      std::vector<dirEntry>{}, foldFn);

  std::sort(entries.begin(), entries.end(),
            [](const dirEntry &left, const dirEntry &right) {
              return left.name < right.name;
            });

  auto stream = std::stringstream{};
  stream << "# " << uri << std::endl;
  stream << "| |" << std::endl;
  stream << "|----------|" << std::endl;

  stream << dirEntry{uri, ".."};
  for (const auto &entry : entries) {
    stream << entry;
  }

  return renderText(stream.str(), templateText);
}
