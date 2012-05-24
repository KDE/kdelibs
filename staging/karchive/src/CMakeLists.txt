# Configure checks for compression/
include(ConfigureChecks.cmake)
configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

if (NOT QT5_BUILD)
   include_directories(
      ${QtMimeTypes_SOURCE_DIR}/include/QtMimeTypes
   )
endif()

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

add_library(karchive ${LIBRARY_TYPE} ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})
generate_export_header(karchive)

target_link_libraries(karchive ${QT_QTCORE_LIBRARY} ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY} ${QtMimeTypes_LIBRARY} kdeqt5staging)

set_target_properties(karchive PROPERTIES VERSION  ${GENERIC_LIB_VERSION}
                                          SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS karchive EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
