#include <iostream>
#include <string>
#include <vector>

#include "json.hpp"
#include "server.h"

struct stats {
  uint32_t passCount;
  std::vector<std::string> failedList;
};

inline void check(const char *id, bool condition, struct stats &stats) {
  if (condition) {
    stats.passCount += 1;
  } else {
    stats.failedList.emplace_back(id);
  }
}

void testLoadConfig(struct stats &stats) {
  const auto dir = std::filesystem::path{ARTIFACTS_PATH};

  check(
      "empty config",
      [] {
        nlohmann::json config;
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing port in config",
      [&dir] {
        nlohmann::json config;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing docRoot in config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["templatePath"] = dir / "template.html";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing templatePath in config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "docRoot in config points to non-existent path",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir / "foo.html";
        config["core"]["templatePath"] = dir / "template.html";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "docRoot in config does not point to a directory",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir / "template.html";
        config["core"]["templatePath"] = dir / "template.html";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "templatePath in config points to non-existent path",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "foo.html";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "templatePath in config does not point to a regular file or symlink",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir;
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "valid base config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";
        return validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing `kind` field in auth config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        config["auth"] = {};
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "unsupported `kind` value in auth config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        config["auth"]["kind"] = "foobar";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing `providers` list in oauthv2 config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        config["auth"]["kind"] = "oauthv2";
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing `name` field in provider config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        nlohmann::json provider0;
        provider0["clientIdEnvVar"] = "CLIENT0_ID";
        provider0["clientSecretEnvVar"] = "CLIENT0_SECRET";

        nlohmann::json provider1;
        provider1["clientIdEnvVar"] = "CLIENT1_ID";
        provider1["clientSecretEnvVar"] = "CLIENT1_SECRET";

        config["auth"]["kind"] = "oauthv2";
        config["auth"]["providers"] = {provider0, provider1};
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "unsupported `name` value in provider config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        nlohmann::json provider0;
        provider0["name"] = "guthub";
        provider0["clientIdEnvVar"] = "CLIENT0_ID";
        provider0["clientSecretEnvVar"] = "CLIENT0_SECRET";

        nlohmann::json provider1;
        provider1["name"] = "guthub";
        provider1["clientIdEnvVar"] = "CLIENT1_ID";
        provider1["clientSecretEnvVar"] = "CLIENT1_SECRET";

        config["auth"]["kind"] = "oauthv2";
        config["auth"]["providers"] = {provider0, provider1};
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing `clientIdEnvVar` field in provider config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        nlohmann::json provider0;
        provider0["name"] = "github";
        provider0["clientSecretEnvVar"] = "CLIENT0_SECRET";

        nlohmann::json provider1;
        provider1["name"] = "github";
        provider1["clientSecretEnvVar"] = "CLIENT1_SECRET";

        config["auth"]["kind"] = "oauthv2";
        config["auth"]["providers"] = {provider0, provider1};
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "missing `clientSecretEnvVar` field in provider config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        nlohmann::json provider0;
        provider0["name"] = "github";
        provider0["clientIdEnvVar"] = "CLIENT0_ID";

        nlohmann::json provider1;
        provider1["name"] = "github";
        provider1["clientIdEnvVar"] = "CLIENT1_ID";

        config["auth"]["kind"] = "oauthv2";
        config["auth"]["providers"] = {provider0, provider1};
        return !validateConfiguration(config, /* silent */ true);
      }(),
      stats);

  check(
      "valid github-based config",
      [&dir] {
        nlohmann::json config;
        config["core"]["port"] = 808;
        config["core"]["docRoot"] = dir;
        config["core"]["templatePath"] = dir / "template.html";

        nlohmann::json provider0;
        provider0["name"] = "github";
        provider0["clientIdEnvVar"] = "CLIENT0_ID";
        provider0["clientSecretEnvVar"] = "CLIENT0_SECRET";

        nlohmann::json provider1;
        provider1["name"] = "github";
        provider1["clientIdEnvVar"] = "CLIENT1_ID";
        provider1["clientSecretEnvVar"] = "CLIENT1_SECRET";

        config["auth"]["kind"] = "oauthv2";
        config["auth"]["providers"] = {provider0, provider1};
        return validateConfiguration(config, /* silent */ true);
      }(),
      stats);
}

void testRenderText(struct stats &stats) {
  check("blank template non-empty",
        renderText(R"(Hello, World!)", "", /* silent */ true) != std::nullopt,
        stats);

  check(
      "blank template",
      [] {
        auto result = renderText(R"(# Hello, World!)", "", /* silent */ true);
        return *result == "";
      }(),
      stats);

  check("markdown non-empty",
        renderText(R"(# Hello, World!)", "{{ body }}", /* silent */ true) !=
            std::nullopt,
        stats);

  check(
      "markdown",
      [] {
        auto result =
            renderText(R"(# Hello, World!)", "{{ body }}", /* silent */ true);
        return *result == "<h1>Hello, World!</h1>\n";
      }(),
      stats);

  check(
      "markdown with template",
      [] {
        auto result = renderText(R"(# Hello, World!)",
                                 "<body>{{ body }}</body>", /* silent */ true);
        return *result == "<body><h1>Hello, World!</h1>\n</body>";
      }(),
      stats);

  check(
      "markdown with template and repeated placeholder",
      [] {
        auto result =
            renderText(R"(# Hello, World!)",
                       "<body>{{ body }}{{ body }}</body>", /* silent */ true);
        return *result ==
               "<body><h1>Hello, World!</h1>\n<h1>Hello, World!</h1>\n</body>";
      }(),
      stats);
}

void testFetchFileContents(struct stats &stats) {
  const auto dir = std::filesystem::path{ARTIFACTS_PATH};

  check("read non-existent file",
        fetchFileContents(dir / "foo-bar.md") == std::nullopt, stats);

  check(
      "read valid file",
      [&dir] {
        auto result = fetchFileContents(dir / "hello.md");
        return *result == "# Hello!\n\nText.\n";
      }(),
      stats);

  check(
      "read symlink",
      [&dir] {
        auto result = fetchFileContents(dir / "symlink.md");
        return *result == "# Hello!\n\nText.\n";
      }(),
      stats);
}

void testRenderFile(struct stats &stats) {
  const auto dir = std::filesystem::path{ARTIFACTS_PATH};

  check("non-existent file",
        renderFile(dir / "foo-bar.md", "{{ body }}", /* silent */ true) ==
            std::nullopt,
        stats);

  check(
      "valid file",
      [&dir] {
        auto result =
            renderFile(dir / "hello.md", "{{ body }}", /* silent */ true);
        return *result == "<h1>Hello!</h1>\n<p>Text.</p>\n";
      }(),
      stats);

  check(
      "symlink",
      [&dir] {
        auto result =
            renderFile(dir / "symlink.md", "{{ body }}", /* silent */ true);
        return *result == "<h1>Hello!</h1>\n<p>Text.</p>\n";
      }(),
      stats);
}

void testRenderDirectory(struct stats &stats) {
  const auto dir = std::filesystem::path{ARTIFACTS_PATH};

  check("non-existent dir",
        renderDirectory("-", dir / "foo-bar", "{{ body }}",
                        /* silent */ true) == std::nullopt,
        stats);

  check("directory without trailing '/'",
        renderDirectory("foo", dir, "{{ body }}", /* silent */ true) ==
            std::nullopt,
        stats);

  check(
      "valid dir",
      [&dir] {
        auto result =
            renderDirectory("foo/", dir, "{{ body }}", /* silent */ true);
        return *result ==
               R"(<h1>foo/</h1>
<table>
<thead>
<tr>
<th></th>
</tr>
</thead>
<tbody>
<tr>
<td><a href="foo/..">..</a></td>
</tr>
<tr>
<td><a href="foo/z-sample-dir">z-sample-dir</a></td>
</tr>
<tr>
<td><a href="foo/hello.md">hello.md</a></td>
</tr>
<tr>
<td><a href="foo/symlink.md">symlink.md</a></td>
</tr>
<tr>
<td><a href="foo/template.html">template.html</a></td>
</tr>
</tbody>
</table>
)";
      }(),
      stats);
}

int main() {
  auto allStats = stats{};

  testLoadConfig(allStats);
  testRenderText(allStats);
  testFetchFileContents(allStats);
  testRenderFile(allStats);
  testRenderDirectory(allStats);

  std::cout << "passed: " << allStats.passCount << "    "
            << "failed: " << allStats.failedList.size() << std::endl;

  for (const auto &testName : allStats.failedList) {
    std::cout << "  FAILED: " << testName << "\n";
  }

  return !allStats.failedList.empty();
}
