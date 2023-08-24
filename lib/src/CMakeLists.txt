# Build md4c as a library.
add_library(md4c
  ${PROJECT_SOURCE_DIR}/external/md4c/entity.c
  ${PROJECT_SOURCE_DIR}/external/md4c/md4c.c
  ${PROJECT_SOURCE_DIR}/external/md4c/md4c-html.c
)

target_include_directories(md4c PUBLIC
  ${PROJECT_SOURCE_DIR}/external/md4c
)

# Build mongoose as a library.
add_library(mongoose
  ${PROJECT_SOURCE_DIR}/external/mongoose/mongoose.c
)

target_include_directories(mongoose PUBLIC
  ${PROJECT_SOURCE_DIR}/external/mongoose
)

# Build the magenta server library.
add_library(server html.cc http.cc util.cc)
target_include_directories(server PUBLIC ${PROJECT_SOURCE_DIR}/lib/include)
target_link_libraries(server PUBLIC md4c mongoose)

# Set stricter warning flags for the magenta server library.
if(MSVC)
  target_compile_options(server PRIVATE /W4 /WX)
else()
  target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
