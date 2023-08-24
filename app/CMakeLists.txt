add_executable(magenta magenta.cc)

target_include_directories(magenta PRIVATE
  ${PROJECT_SOURCE_DIR}/external/args
)

target_link_libraries(magenta server)
