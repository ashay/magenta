#include <filesystem>
#include <iostream>
#include <string>

#include "args.hpp"
#include "json.hpp"
#include "server.h"

static std::optional<std::string>
load404Page(const std::filesystem::path &docRoot,
            const std::string &templateText) {
  auto path = docRoot / "404.md";
  return std::filesystem::exists(path)
             ? renderFile(path, templateText)
             : renderText("# 404 Not Found", templateText);
}

struct magenta {
  static const int success = static_cast<int>(Err::NONE);
  static const int failure = static_cast<int>(Err::CMD_ARGS);

  static const char *help() {
    return "Web server for rendering wiki-like documents.";
  }

  std::filesystem::path configPath;

  magenta() : configPath(std::filesystem::current_path() / "config.json") {}

  template <class F> void parse(F f) {
    f(configPath, "--config-path", "-c",
      args::help(
          "Path to configuration file (`$PWD/config.json` if not specified"));
  }

  int run() {
    if (!std::filesystem::exists(configPath)) {
      std::cerr << "config file path points to non-existent file: '"
                << configPath.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    if (!std::filesystem::is_regular_file(configPath) &&
        std::filesystem::is_symlink(configPath)) {
      std::cerr
          << "config file path does not point to a regular file or symlink: '"
          << configPath.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    auto maybeConfig = fetchFileContents(configPath);
    if (!maybeConfig) {
      std::cerr << "failed to load configuration from file: '"
                << configPath.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    auto config = nlohmann::json::parse(*maybeConfig);
    auto port = config["core"]["port"];
    auto docRoot =
        config["core"]["docRoot"].template get<std::filesystem::path>();
    auto templatePath =
        config["core"]["templatePath"].template get<std::filesystem::path>();

    if (!std::filesystem::exists(docRoot)) {
      std::cerr << "document root points to non-existent directory: '"
                << docRoot.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    if (!std::filesystem::is_directory(docRoot)) {
      std::cerr << "document root does not point to a directory: '"
                << docRoot.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    auto maybeTemplateText = fetchFileContents(templatePath);
    if (!maybeTemplateText) {
      std::cerr << "failed to load template from template file: '"
                << templatePath.string() << "'" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    auto maybeNotFoundHtml = load404Page(docRoot, *maybeTemplateText);
    if (!maybeNotFoundHtml) {
      std::cerr << "failed to load 404 page content" << std::endl;
      return static_cast<int>(Err::FILE_IO);
    }

    std::cout << "Listening for connections on port " << port
              << ", with document root at '" << docRoot.string()
              << "' and template file '" << templatePath.string() << "' ..."
              << std::endl;

    startWebServer(docRoot, std::move(*maybeTemplateText),
                   std::move(*maybeNotFoundHtml), port);

    std::cout << "No longer listening for connections." << std::endl;
    return static_cast<int>(Err::NONE);
  }
};

int main(int argc, const char *argv[]) {
  return args::parse<magenta>(argc, argv);
}
