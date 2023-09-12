#pragma once

#include <filesystem>

#include "args.hpp"
#include "server.h"

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

  int run();
};

int realMain(int argc, const char *argv[]);
