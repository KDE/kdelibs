project(karchive)

find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "ZLib" "Support for gzip compressed files and data streams" "http://www.zlib.net" TRUE "" "Required by the core KDE libraries and some criti cal kioslaves")

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${KDE4_KDECORE_INCLUDES}
)

add_subdirectory(src)
add_subdirectory(autotests)

