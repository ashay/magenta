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
    struct mg_http_serve_opts opts = {.root_dir = auxData.docRoot.c_str()};
    mg_http_serve_file(connection, message, path.c_str(), &opts);
    return true;
  }

  auto maybeHtml = renderFile(path, auxData.templateText);
  if (!maybeHtml) {
    mg_http_reply(connection, 500, "Content-Type: text/html\r\n",
                  "failed to render page for URI: %.*s", uri.length(),
                  uri.c_str());
    return false;
  }

  mg_http_reply(connection, 200, "Content-Type: text/html\r\n",
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
    mg_http_reply(connection, 500, "Content-Type: text/html\r\n",
                  "failed to render page for URI: %.*s", uri.length(),
                  uri.c_str());
    return false;
  }

  mg_http_reply(connection, 200, "Content-Type: text/html\r\n",
                maybeHtml->c_str(), maybeHtml->length());
  return true;
}

static void reqFn(struct mg_connection *connection, int ev, void *evData,
                  void *fnData) {
  if (ev != MG_EV_HTTP_MSG) {
    return;
  }

  struct mg_http_message *message =
      static_cast<struct mg_http_message *>(evData);
  auto uri = std::string{message->uri.ptr, message->uri.len};

  auto uriPath = std::filesystem::path{uri}.lexically_normal();
  auto cleanedUri = uriPath.string();
  if (cleanedUri.length() > 0 && cleanedUri.back() == '/') {
    cleanedUri.pop_back();
  }

  auto auxData = static_cast<auxInfo *>(fnData);
  auto fsPath = auxData->docRoot;
  fsPath += std::filesystem::path{cleanedUri}.make_preferred();

  if (!std::filesystem::exists(fsPath)) {
    std::cerr << "file not found " << fsPath << std::endl;
    mg_http_reply(connection, 404, "Content-Type: text/html\r\n",
                  auxData->notFoundHtml.c_str(),
                  auxData->notFoundHtml.length());
    return;
  }

  if (std::filesystem::is_directory(fsPath)) {
    if (std::filesystem::exists(fsPath / "index.md")) {
      fsPath /= "index.md";
    } else {
      handleDirectoryRequest(cleanedUri, fsPath, *auxData, connection);
      return;
    }
  }

  handleFileRequest(uri, fsPath, *auxData, connection, message);
}

// Handle interrupts, like Ctrl-C
static int s_signo;

static void signal_handler(int signo) { s_signo = signo; }

void startWebServer(const std::filesystem::path &docRoot,
                    std::string templateText, std::string notFoundHtml,
                    uint32_t portNumber) {
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  auto auxData =
      auxInfo{docRoot, std::move(templateText), std::move(notFoundHtml)};
  auto endPoint = std::string{"http://localhost:"} + std::to_string(portNumber);
  mg_http_listen(&mgr, endPoint.c_str(), reqFn, &auxData);

  while (s_signo == 0)
    mg_mgr_poll(&mgr, 1000);

  mg_mgr_free(&mgr);
}
