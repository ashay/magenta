#pragma once

#include <filesystem>
#include <optional>
#include <string>

/// Given some markdown text and an HTML body template, translate the markdown
/// text into HTML and embed it into the template.  Returns none on failure and
/// does not print errors on the console if `silent` is true.
std::optional<std::string> renderText(const std::string &markDownText,
                                      const std::string &templateText,
                                      bool silent = false);

/// Given a path to a file that contains markdown text and an HTML body
/// template, translate the markdown text into HTML and embed it into the
/// template.  Returns none on failure and does not print errors on the console
/// if `silent` is true.
std::optional<std::string> renderFile(const std::filesystem::path &path,
                                      const std::string &templateText,
                                      bool silent = false);

/// Render the directory contents as an HTML page.  Returns none on failure and
/// does not print errors on the console if `silent` is true.
std::optional<std::string> renderDirectory(const std::string &uri,
                                           const std::filesystem::path &path,
                                           const std::string &templateText,
                                           bool silent = false);
