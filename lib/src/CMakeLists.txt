add_library(server config.cc html.cc http.cc util.cc)

target_include_directories(server PUBLIC
  ${PROJECT_SOURCE_DIR}/lib/include
  ${PROJECT_SOURCE_DIR}/external/json
)

target_link_libraries(server PUBLIC md4c mongoose)

# Set stricter warning flags for the magenta server library.
if(MSVC)
  target_compile_options(server PRIVATE /W4 /WX)
else()
  target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
