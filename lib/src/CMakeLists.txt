add_library(server
  html.cc
  http.cc
  util.cc

  ${PROJECT_SOURCE_DIR}/external/md4c/entity.c
  ${PROJECT_SOURCE_DIR}/external/md4c/md4c.c
  ${PROJECT_SOURCE_DIR}/external/md4c/md4c-html.c
  ${PROJECT_SOURCE_DIR}/external/mongoose/mongoose.c
)

target_include_directories(server PRIVATE
  ${PROJECT_SOURCE_DIR}/external/md4c
  ${PROJECT_SOURCE_DIR}/external/mongoose
)

target_include_directories(server PUBLIC
  ${PROJECT_SOURCE_DIR}/lib/include
)

if(LINUX)
  # So that we can use arc4random() instead of rand() in mongoose.c
  target_link_libraries(server PUBLIC bsd)
endif()
