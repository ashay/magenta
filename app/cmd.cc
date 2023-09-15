#include <iostream>
#include <optional>
#include <string>

#include "args.hpp"
#include "cmd.hpp"
#include "server.h"

std::optional<std::string> load404Page(const std::filesystem::path &docRoot,
                                       const std::string &templateText) {
  auto path = docRoot / "404.md";
  return std::filesystem::exists(path)
             ? renderFile(path, templateText)
             : renderText("# 404 Not Found", templateText);
}

bool copyDefaultConfig(const std::filesystem::path &configPath) {
  // Check if the default config file exists.
  const auto defaultConfigPath =
      std::filesystem::current_path() / ".default.config.json";

  if (!std::filesystem::exists(defaultConfigPath)) {
    std::cerr << "config file path points to non-existent path: '"
              << configPath.string()
              << "' and I couldn't find the default config file: '"
              << defaultConfigPath.string() << "'" << std::endl;
    return false;
  }

  // Copy from `defaultConfigPath` to `configPath`.
  auto errCode = std::error_code{};
  std::filesystem::copy(defaultConfigPath, configPath, errCode);
  if (errCode) {
    std::cerr << "config file path points to non-existent path: '"
              << configPath.string()
              << "' and I couldn't copy contents of the default config file: '"
              << defaultConfigPath.string() << "'" << std::endl;
    return false;
  }

  std::cout << "Copied default configuration file '"
            << defaultConfigPath.string() << "' to '" << configPath.string()
            << "'" << std::endl;
  return true;
}

int magenta::run() {
  if (!std::filesystem::exists(configPath) && !copyDefaultConfig(configPath)) {
    return static_cast<int>(Err::FILE_IO);
  }

  if (!std::filesystem::is_regular_file(configPath) &&
      !std::filesystem::is_symlink(configPath)) {
    std::cerr
        << "config file path does not point to a regular file or symlink: '"
        << configPath.string() << "'" << std::endl;
    return static_cast<int>(Err::FILE_IO);
  }

  auto maybeConfig = validateAndLoadConfiguration(configPath);
  if (!maybeConfig) {
    return static_cast<int>(Err::FILE_IO);
  }

  auto maybeTemplateText = fetchFileContents(maybeConfig->templatePath);
  if (!maybeTemplateText) {
    std::cerr << "failed to load template from template file: '"
              << maybeConfig->templatePath.string() << "'" << std::endl;
    return static_cast<int>(Err::FILE_IO);
  }

  auto maybeNotFoundHtml =
      load404Page(maybeConfig->docRoot, *maybeTemplateText);
  if (!maybeNotFoundHtml) {
    std::cerr << "failed to load 404 page content" << std::endl;
    return static_cast<int>(Err::FILE_IO);
  }

  std::cout << "Listening for connections on port " << maybeConfig->port
            << ", with document root at '" << maybeConfig->docRoot.string()
            << "' and template file '" << maybeConfig->templatePath.string()
            << "' ..." << std::endl;

  startWebServer(maybeConfig->docRoot, std::move(*maybeTemplateText),
                 std::move(*maybeNotFoundHtml), maybeConfig->port);

  std::cout << "No longer listening for connections." << std::endl;
  return static_cast<int>(Err::NONE);
}

int realMain(int argc, const char *argv[]) {
  return args::parse<magenta>(argc, argv);
}
