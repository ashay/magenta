set(ARTIFACTS_DIR "artifacts")
cmake_path(ABSOLUTE_PATH ARTIFACTS_DIR
  BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  NORMALIZE
  OUTPUT_VARIABLE ARTIFACTS_PATH
)
add_compile_definitions("ARTIFACTS_PATH=\"${ARTIFACTS_PATH}\"")

add_executable(test-driver driver.cc)
target_link_libraries(test-driver server)

add_custom_target(check-server
  test-driver
  DEPENDS test-driver
)
