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
