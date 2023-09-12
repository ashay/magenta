#pragma once

#include <filesystem>
#include <optional>

#include "json.hpp"

struct config {
  uint32_t port;
  std::filesystem::path docRoot;
  std::filesystem::path templatePath;
};

bool validateConfiguration(const nlohmann::json &configJson,
                           bool silent = false);

std::optional<struct config>
validateAndLoadConfiguration(const std::filesystem::path &configPath);
