#include <iostream>

#include "config.h"
#include "util.h"

bool validateCoreConfiguration(const nlohmann::json &core,
                               bool silent = false) {
  if (!core.contains("port")) {
    if (!silent) {
      std::cerr << "`port` value missing from core configuration" << std::endl;
    }
    return false;
  }

  if (!core.contains("docRoot")) {
    if (!silent) {
      std::cerr << "`docRoot` value missing from core configuration"
                << std::endl;
    }
    return false;
  }

  if (!core.contains("templatePath")) {
    if (!silent) {
      std::cerr << "`templatePath` value missing from core configuration"
                << std::endl;
    }
    return false;
  }

  auto docRoot = core["docRoot"].template get<std::filesystem::path>();
  auto templatePath =
      core["templatePath"].template get<std::filesystem::path>();

  if (!std::filesystem::exists(docRoot)) {
    if (!silent) {
      std::cerr << "document root points to non-existent path: '"
                << docRoot.string() << "'" << std::endl;
    }
    return false;
  }

  if (!std::filesystem::is_directory(docRoot)) {
    if (!silent) {
      std::cerr << "document root does not point to a directory: '"
                << docRoot.string() << "'" << std::endl;
    }
    return false;
  }

  if (!std::filesystem::exists(templatePath)) {
    if (!silent) {
      std::cerr << "template path points to non-existent path: '"
                << templatePath.string() << "'" << std::endl;
    }
    return false;
  }

  if (!std::filesystem::is_regular_file(templatePath) &&
      !std::filesystem::is_symlink(templatePath)) {
    if (!silent) {
      std::cerr
          << "template path does not point to a regular file or a symlink: '"
          << templatePath.string() << "'" << std::endl;
    }
    return false;
  }

  return true;
}

bool validateAuthConfiguration(const nlohmann::json &auth,
                               bool silent = false) {
  if (!auth.contains("kind")) {
    if (!silent) {
      std::cerr << "missing `kind` field for auth configuration" << std::endl;
    }
    return false;
  }

  auto kind = auth["kind"].template get<std::string>();
  if (kind != "oauthv2") {
    if (!silent) {
      std::cerr << "unsupported `kind` value for auth configuration: '" << kind
                << "'" << std::endl;
    }
    return false;
  }

  if (!auth.contains("providers")) {
    if (!silent) {
      std::cerr << "missing `providers` field for auth configuration"
                << std::endl;
    }
    return false;
  }

  for (const auto &provider : auth["providers"]) {
    if (!provider.contains("name")) {
      if (!silent) {
        std::cerr << "missing `name` field for provider configuration"
                  << std::endl;
      }
      return false;
    }

    auto providerName = provider["name"].template get<std::string>();
    if (providerName != "github") {
      if (!silent) {
        std::cerr << "unsupported `name` value for provider configuration: '"
                  << providerName << "'" << std::endl;
      }
      return false;
    }

    if (!provider.contains("clientIdEnvVar")) {
      if (!silent) {
        std::cerr << "missing `clientIdEnvVar` field for provider configuration"
                  << std::endl;
      }
      return false;
    }

    if (!provider.contains("clientSecretEnvVar")) {
      if (!silent) {
        std::cerr
            << "missing `clientSecretEnvVar` field for provider configuration"
            << std::endl;
      }
      return false;
    }
  }

  return true;
}

bool validateConfiguration(const nlohmann::json &configJson, bool silent) {
  if (!configJson.contains("core")) {
    if (!silent) {
      std::cerr << "core configuration missing in the configuration file"
                << std::endl;
    }
    return false;
  }

  if (!validateCoreConfiguration(configJson["core"], silent)) {
    return false;
  }

  if (configJson.contains("auth") &&
      !validateAuthConfiguration(configJson["auth"], silent)) {
    return false;
  }

  return true;
}

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

  auto parseJson =
      [](const std::string &config) -> std::optional<nlohmann::json> {
    try {
      return nlohmann::json::parse(config);
    } catch (...) {
      return std::nullopt;
    }
  };

  auto maybeConfigJson = parseJson(*maybeConfig);
  if (!maybeConfigJson) {
    std::cerr << "invalid JSON in configuration file: '" << configPath.string()
              << "'" << std::endl;
    return std::nullopt;
  }

  auto configJson = *maybeConfigJson;
  if (!validateConfiguration(configJson)) {
    return std::nullopt;
  }

  return config{
      configJson["core"]["port"],
      configJson["core"]["docRoot"].template get<std::filesystem::path>(),
      configJson["core"]["templatePath"].template get<std::filesystem::path>(),
  };
}
