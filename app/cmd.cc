#include <iostream>
#include <optional>
#include <string>

#include "args.hpp"
#include "cmd.hpp"
#include "json.hpp"
#include "server.h"

struct config {
  uint32_t port;
  std::filesystem::path docRoot;
  std::filesystem::path templatePath;
};

std::optional<struct config>
validateAndLoadConfiguration(const std::filesystem::path &configPath) {
  if (!std::filesystem::exists(configPath)) {
    std::cerr << "config file path points to non-existent file: '"
              << configPath.string() << "'" << std::endl;
    return std::nullopt;
  }

  if (!std::filesystem::is_regular_file(configPath) &&
      std::filesystem::is_symlink(configPath)) {
    std::cerr
        << "config file path does not point to a regular file or symlink: '"
        << configPath.string() << "'" << std::endl;
    return std::nullopt;
  }

  auto maybeConfig = fetchFileContents(configPath);
  if (!maybeConfig) {
    std::cerr << "failed to load configuration from file: '"
              << configPath.string() << "'" << std::endl;
    return std::nullopt;
  }

  auto configJson = nlohmann::json::parse(*maybeConfig);
  auto docRoot =
      configJson["core"]["docRoot"].template get<std::filesystem::path>();
  auto templatePath =
      configJson["core"]["templatePath"].template get<std::filesystem::path>();

  if (!std::filesystem::exists(docRoot)) {
    std::cerr << "document root points to non-existent path: '"
              << docRoot.string() << "'" << std::endl;
    return std::nullopt;
  }

  if (!std::filesystem::is_directory(docRoot)) {
    std::cerr << "document root does not point to a directory: '"
              << docRoot.string() << "'" << std::endl;
    return std::nullopt;
  }

  if (!std::filesystem::exists(templatePath)) {
    std::cerr << "template path points to non-existent path: '"
              << templatePath.string() << "'" << std::endl;
    return std::nullopt;
  }

  if (!std::filesystem::is_regular_file(templatePath) &&
      !std::filesystem::is_symlink(templatePath)) {
    std::cerr
        << "template path does not point to a regular file or a symlink: '"
        << templatePath.string() << "'" << std::endl;
    return std::nullopt;
  }

  return config{
      configJson["core"]["port"],
      docRoot,
      templatePath,
  };
}

std::optional<std::string> load404Page(const std::filesystem::path &docRoot,
                                       const std::string &templateText) {
  auto path = docRoot / "404.md";
  return std::filesystem::exists(path)
             ? renderFile(path, templateText)
             : renderText("# 404 Not Found", templateText);
}

int magenta::run() {
  if (!std::filesystem::exists(configPath)) {
    std::cerr << "config file path points to non-existent path: '"
              << configPath.string() << "'" << std::endl;
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
