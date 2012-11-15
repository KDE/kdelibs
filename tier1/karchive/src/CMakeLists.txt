# Configure checks for compression/
include(ConfigureChecks.cmake)
configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/inqt5)

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

include_directories(${QT_MKSPECS_DIR}/default) # for qplatformdefs.h
set(karchive_SRCS ${karchive_SRCS}
 inqt5/qmimedatabase.cpp
 inqt5/qmimeglobpattern.cpp
 inqt5/qmimemagicrule.cpp
 inqt5/qmimemagicrulematcher.cpp
 inqt5/qmimeprovider.cpp
 inqt5/qmimetype.cpp
 inqt5/qmimetypeparser.cpp
 inqt5/qstandardpaths.cpp
 inqt5/qsavefile.cpp
)
if (UNIX)
   set(karchive_SRCS ${karchive_SRCS} inqt5/qstandardpaths_unix.cpp)
elseif(WIN32)
   set(karchive_SRCS ${karchive_SRCS} inqt5/qstandardpaths_win.cpp)
elseif(APPLE)
   set(karchive_SRCS ${karchive_SRCS} inqt5/qstandardpaths_mac.cpp)
endif()

qt4_add_resources( karchive_SRCS inqt5/mimetypes.qrc )

add_library(karchive-qt4 ${LIBRARY_TYPE} ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})
generate_export_header(karchive-qt4)

target_link_libraries(karchive-qt4 ${QT_QTCORE_LIBRARY} ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY})

set_target_properties(karchive-qt4 PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

### Why isn't all this in cmake ??
# Figure out what the default install directory for libraries should be.
# This is the same logic as in cmake's GNUInstallDirs.cmake.
set(_LIBDIR_DEFAULT "lib")
# Override this default 'lib' with 'lib64' iff:
#  - we are on Linux system but NOT cross-compiling
#  - we are NOT on debian
#  - we are on a 64 bits system
# reason is: amd64 ABI: http://www.x86-64.org/documentation/abi.pdf
# For Debian with multiarch, use 'lib/${CMAKE_LIBRARY_ARCHITECTURE}' if
# CMAKE_LIBRARY_ARCHITECTURE is set (which contains e.g. "i386-linux-gnu"
# See http://wiki.debian.org/Multiarch
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CROSSCOMPILING)
  if (EXISTS "/etc/debian_version") # is this a debian system ?
      if(CMAKE_LIBRARY_ARCHITECTURE)
        set(_LIBDIR_DEFAULT "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
      endif()
  else() # not debian, rely on CMAKE_SIZEOF_VOID_P:
    if(NOT DEFINED CMAKE_SIZEOF_VOID_P)
      message(AUTHOR_WARNING
        "Unable to determine default LIB_INSTALL_LIBDIR directory because no target architecture is known. "
        "Please enable at least one language before including KDEInstallDirs.")
    else()
      if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(_LIBDIR_DEFAULT "lib64")
      endif()
    endif()
  endif()
endif()


set(BIN_INSTALL_DIR "bin")
set(LIB_INSTALL_DIR "${_LIBDIR_DEFAULT}")
set(INSTALL_TARGETS_DEFAULT_ARGS  RUNTIME DESTINATION "${BIN_INSTALL_DIR}"
                                  LIBRARY DESTINATION "${LIB_INSTALL_DIR}"
                                  ARCHIVE DESTINATION "${LIB_INSTALL_DIR}" COMPONENT Devel )
# on the Mac support an extra install directory for application bundles
if(APPLE)
  set(INSTALL_TARGETS_DEFAULT_ARGS  ${INSTALL_TARGETS_DEFAULT_ARGS}
                                    BUNDLE DESTINATION "${BUNDLE_INSTALL_DIR}" )
endif(APPLE)

install(TARGETS karchive-qt4 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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

