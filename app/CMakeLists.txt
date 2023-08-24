add_executable(magenta magenta.cc)

target_include_directories(magenta PRIVATE
  ${PROJECT_SOURCE_DIR}/external/args
)

target_link_libraries(magenta server)

if(MSVC)
  target_compile_options(magenta PRIVATE /W4 /WX)
else()
  target_compile_options(magenta PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
