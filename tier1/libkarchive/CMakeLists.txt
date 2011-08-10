project(karchive)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${KDE4_KDECORE_INCLUDES}
)

# Configure checks for compression/
include(ConfigureChecks.cmake)
configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

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
  kfilterbase.cpp
  kfilterdev.cpp
  kgzipfilter.cpp
  klimitediodevice.cpp
  ktar.cpp
  kzip.cpp
)

kde4_add_library(karchive ${LIBRARY_TYPE} ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})

generate_export_header(karchive
  DEPRECATED_NAME KDE_DEPRECATED
)

target_link_libraries(karchive ${QT_QTCORE_LIBRARY} ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY} ${KDE4_KDECORE_LIBS})

set_target_properties(karchive PROPERTIES VERSION  ${KDE_NON_GENERIC_LIB_VERSION}
                                          SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS karchive EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  karchive.h
  kar.h
  kfilterbase.h
  kfilterdev.h
  ktar.h
  kzip.h

  ${CMAKE_CURRENT_BINARY_DIR}/karchive_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

add_subdirectory(tests)
