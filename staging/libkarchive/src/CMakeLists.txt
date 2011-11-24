# Configure checks for compression/
include(ConfigureChecks.cmake)
configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# For KSaveFile - TODO put that into Qt
include_directories( ${kcoreaddons_SOURCE_DIR}/src/io
                     ${kcoreaddons_BINARY_DIR}/src )
add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(BZIP2_FOUND)
   include_directories(${BZIP2_INCLUDE_DIR})
   set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kbzip2filter.cpp )
   set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif(BZIP2_FOUND)

if(LIBLZMA_FOUND)
   include_directories(${LIBLZMA_INCLUDE_DIRS})
   set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kxzfilter.cpp )
   set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${LIBLZMA_LIBRARIES})
endif(LIBLZMA_FOUND)


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

kf5_add_library(karchive ${LIBRARY_TYPE} ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})

# Uses kdecore for KMimeType only, TODO, port to QMimeType
target_link_libraries(karchive ${QT_QTCORE_LIBRARY} ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY} kdecore kcoreaddons)

set_target_properties(karchive PROPERTIES VERSION  ${GENERIC_LIB_VERSION}
                                          SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS karchive ${INSTALL_TARGETS_DEFAULT_ARGS})

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

