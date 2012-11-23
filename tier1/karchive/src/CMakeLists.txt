# Configure checks for compression/
include(ConfigureChecks.cmake)
configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inqt5)

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

target_link_libraries(karchive-qt4 ${QT_QTCORE_LIBRARY} ${karchive_OPTIONAL_LIBS} ${ZLIB_LIBRARY})

set(KARCHIVE_QT4_VERSION_STRING 1.0.0)

set_target_properties(karchive-qt4 PROPERTIES VERSION "${KARCHIVE_QT4_VERSION_STRING}"
                                          SOVERSION 1
                                          DEFINE_SYMBOL KARCHIVE_QT4_MAKEDLL
)
generate_export_header(karchive-qt4 BASE_NAME karchive EXPORT_FILE_NAME karchive_qt4_export.h)

include(GNUInstallDirs)


install(TARGETS karchive-qt4 EXPORT KArchive_Qt4Targets
                             RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
                             LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                             ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Devel )

install( FILES
  karchive.h
  kar.h
  kcompressiondevice.h
  kfilterbase.h
  kfilterdev.h
  ktar.h
  kzip.h

  ${CMAKE_CURRENT_BINARY_DIR}/karchive_qt4_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

if(LIBLZMA_FOUND)
   install( FILES k7zip.h
            DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
   )
endif()


set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/KArchive_Qt4" )

install(EXPORT KArchive_Qt4Targets DESTINATION ${CMAKECONFIG_INSTALL_DIR} NAMESPACE KArchive::)

include(CMakePackageConfigHelpers)
configure_package_config_file(../cmake/KArchive_Qt4Config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KArchive_Qt4Config.cmake
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
                              NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/KArchive_Qt4ConfigVersion.cmake
                                  VERSION ${KARCHIVE_QT4_VERSION_STRING}
                                  COMPATIBILITY SameMajorVersion )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KArchive_Qt4Config.cmake ${CMAKE_CURRENT_BINARY_DIR}/KArchive_Qt4ConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
