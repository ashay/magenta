#include <iostream>
#include <string>
#include <vector>

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
</tbody>
</table>
)";
      }(),
      stats);
}

int main() {
  auto allStats = stats{};

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
