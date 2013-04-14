set(HAVE_BZIP2_SUPPORT ${BZIP2_FOUND})
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
    set(NEED_BZ2_PREFIX 1)
endif(BZIP2_FOUND AND BZIP2_NEED_PREFIX)

set(HAVE_XZ_SUPPORT ${LIBLZMA_FOUND})

configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(BZIP2_FOUND)
   include_directories(${BZIP2_INCLUDE_DIR})
   set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kbzip2filter.cpp )
   set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif()

if(LIBLZMA_FOUND)
   include_directories(${LIBLZMA_INCLUDE_DIRS})
   set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kxzfilter.cpp k7zip.cpp )
   set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${LIBLZMA_LIBRARIES})
endif()


set(karchive_SRCS
   karchive.cpp
   kar.cpp
   kcompressiondevice.cpp
   kfilterbase.cpp
   kfilterdev.cpp
   kgzipfilter.cpp
   klimitediodevice.cpp
   knonefilter.cpp
   ktar.cpp
   kzip.cpp
)

add_library(KArchive ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})
generate_export_header(KArchive)

target_link_libraries(KArchive LINK_PUBLIC Qt5::Core)
target_link_libraries(KArchive ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY} )

set_target_properties(KArchive PROPERTIES VERSION   ${KARCHIVE_VERSION_STRING}
                                          SOVERSION ${KARCHIVE_SOVERSION}
)

install(TARGETS KArchive EXPORT KArchiveTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  karchive.h
  kar.h
  kcompressiondevice.h
  kfilterbase.h
  kfilterdev.h
  ktar.h
  kzip.h

  ${CMAKE_CURRENT_BINARY_DIR}/karchive_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)


if(LIBLZMA_FOUND)
   install( FILES k7zip.h
            DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
   )
endif()
