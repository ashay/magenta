#pragma once

#include <filesystem>
#include <string>

/// Entry point into the wikiweb library.  Start servicing HTTP connections
/// that arrive on port `portNumber` to render pages at the `docRoot` document
/// root using the HTML body template in `templateText`.  Show `notFoundHtml`
/// for 404 pages.
void startWebServer(const std::filesystem::path &docRoot,
                    std::string templateText, std::string notFoundHtml,
                    uint32_t portNumber);
