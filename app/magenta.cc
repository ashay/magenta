#include <filesystem>
#include <iostream>
#include <string>

#include "args.hpp"
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

  uint32_t portNumber;
  std::filesystem::path docRoot;
  std::filesystem::path templatePath;

  magenta()
      : portNumber(8080), docRoot(std::filesystem::current_path() / "docs"),
        templatePath(std::filesystem::current_path() / "template.html") {}

  template <class F> void parse(F f) {
    f(portNumber, "--port", "-p",
      args::help("HTTP port number to listen for connections (8000 if not "
                 "specified)"));

    f(docRoot, "--doc-root", "-d",
      args::help("Path to document root ('docs' if not specified)"));

    f(templatePath, "--template-path", "-t",
      args::help("Path to file that contains the HTML template "
                 "('template.html' if not specified)"));
  }

  int run() {
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

    std::cout << "Listening for connections on port " << portNumber
              << ", with document root at '" << docRoot.string()
              << "' and template file '" << templatePath.string() << "' ..."
              << std::endl;

    startWebServer(docRoot, std::move(*maybeTemplateText),
                   std::move(*maybeNotFoundHtml), portNumber);

    std::cout << "No longer listening for connections." << std::endl;
    return static_cast<int>(Err::NONE);
  }
};

int main(int argc, const char *argv[]) {
  return args::parse<magenta>(argc, argv);
}
