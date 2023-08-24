#include <fstream>
#include <streambuf>

#include "util.h"

std::optional<std::string>
fetchFileContents(const std::filesystem::path &path) {
  auto stream = std::ifstream{path};
  if (!stream) {
    return {};
  }

  return std::string{(std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>()};
}
