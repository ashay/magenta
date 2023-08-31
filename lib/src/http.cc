#include <csignal>
#include <functional>
#include <iostream>
#include <optional>
#include <string>

#include "html.h"
#include "http.h"
#include "mongoose.h"
#include "util.h"

struct auxInfo {
  std::filesystem::path docRoot;
  std::string templateText;
  std::string notFoundHtml;
};

static const auto codeOk = 200;
static const auto codeRedirect = 302;
static const auto codeNotFound = 404;
static const auto codeInternalError = 500;

static bool handleFileRequest(const std::string &uri,
                              const std::filesystem::path &path,
                              const struct auxInfo &auxData,
                              struct mg_connection *connection,
                              struct mg_http_message *message) {
  switch (std::filesystem::status(path).type()) {
  case std::filesystem::file_type::regular:
  case std::filesystem::file_type::symlink:
    break;
  default:
    assert(false && "Invalid request, expected regular file or symlink");
  }

  // Non-Markdown files pass through without any rendering.
  if (path.extension() != ".md") {
    auto pathString = path.string();
    auto docRootString = auxData.docRoot.string();

    auto opts = mg_http_serve_opts{};
    opts.root_dir = docRootString.c_str();
    mg_http_serve_file(connection, message, pathString.c_str(), &opts);
    return true;
  }

  auto maybeHtml = renderFile(path, auxData.templateText);
  if (!maybeHtml) {
    mg_http_reply(connection, codeInternalError, "Content-Type: text/html\r\n",
                  "failed to render page for URI: %.*s", uri.length(),
                  uri.c_str());
    return false;
  }

  mg_http_reply(connection, codeOk, "Content-Type: text/html\r\n",
                maybeHtml->c_str(), maybeHtml->length());
  return true;
}

static bool handleDirectoryRequest(const std::string &uri,
                                   const std::filesystem::path &path,
                                   const struct auxInfo &auxData,
                                   struct mg_connection *connection) {
  switch (std::filesystem::status(path).type()) {
  case std::filesystem::file_type::directory:
    break;
  default:
    assert(false && "Invalid request, expected directory");
  }

  auto maybeHtml = renderDirectory(uri, path, auxData.templateText);
  if (!maybeHtml) {
    mg_http_reply(connection, codeInternalError, "Content-Type: text/html\r\n",
                  "failed to render page for URI: %.*s", uri.length(),
                  uri.c_str());
    return false;
  }

  mg_http_reply(connection, codeOk, "Content-Type: text/html\r\n",
                maybeHtml->c_str(), maybeHtml->length());
  return true;
}

static void responseFn(struct mg_connection *connection, int ev, void *evData,
                       void *fnData) {
  if (ev != MG_EV_HTTP_MSG) {
    return;
  }

  struct mg_http_message *message =
      static_cast<struct mg_http_message *>(evData);
  auto uri = std::string{message->uri.ptr, message->uri.len};
  auto uriPath = std::filesystem::path{uri}.lexically_normal();
  auto normalUri = uriPath.string();

  auto auxData = static_cast<auxInfo *>(fnData);
  auto fsPath = auxData->docRoot;
  fsPath += uriPath.make_preferred();

  if (!std::filesystem::exists(fsPath)) {
    std::cerr << "file not found " << fsPath << std::endl;
    mg_http_reply(connection, codeNotFound, "Content-Type: text/html\r\n",
                  auxData->notFoundHtml.c_str(),
                  auxData->notFoundHtml.length());
    return;
  }

  // If this URI points to a directory, then make sure it always ends in a '/',
  // so that relative paths always refer to the URI directory instead of the
  // parent directory.
  if (normalUri.length() > 0 && normalUri.back() != '/' &&
      std::filesystem::is_directory(fsPath)) {
    auto redirectMsg = std::string{"Location: "} + normalUri + "/\r\n";
    mg_http_reply(connection, codeRedirect, redirectMsg.c_str(), "");
    return;
  }

  if (std::filesystem::is_directory(fsPath)) {
    if (std::filesystem::exists(fsPath / "index.md")) {
      fsPath /= "index.md";
    } else {
      handleDirectoryRequest(normalUri, fsPath, *auxData, connection);
      return;
    }
  }

  handleFileRequest(uri, fsPath, *auxData, connection, message);
}

class signalHandler {
private:
  static inline std::function<void(int)> handlerFunc = nullptr;

  static void handler(int signal) { handlerFunc(signal); }

public:
  static void init(std::function<void(int)> func) {
    handlerFunc = func;
    std::signal(SIGINT, handler);
    std::signal(SIGTERM, handler);
  }
};

void startWebServer(const std::filesystem::path &docRoot,
                    std::string templateText, std::string notFoundHtml,
                    uint32_t portNumber) {
  // Handle interrupts, like Ctrl-C
  auto sigNo = 0;
  signalHandler::init([&sigNo](int number) { sigNo = number; });

  auto mgr = mg_mgr{};
  mg_mgr_init(&mgr);

  auto auxData =
      auxInfo{docRoot, std::move(templateText), std::move(notFoundHtml)};
  auto endPoint = std::string{"http://localhost:"} + std::to_string(portNumber);
  mg_http_listen(&mgr, endPoint.c_str(), responseFn, &auxData);

  const auto timeoutMs = 1000;
  while (sigNo == 0)
    mg_mgr_poll(&mgr, timeoutMs);

  mg_mgr_free(&mgr);
}
