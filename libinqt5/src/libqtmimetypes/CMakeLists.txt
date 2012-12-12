cmake_minimum_required(VERSION 2.8.10.1)

project(QtMimeTypes)

find_package(ECM 0.0.5 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMVersion)

# -- Maintained by scripty
ecm_version(5 0 0)
# --

include(ECMQtFramework)

find_package(Qt5Transitional REQUIRED Core)

include(FeatureSummary)

# Qt itself isn't that strict, and frowns at Q_FOREACH
remove_definitions(-DQT_NO_KEYWORDS)

# used by the (non-generated) export header.
add_definitions(-DQMIME_LIBRARY)

include_directories(${QT_MKSPECS_DIR}/default) # for qplatformdefs.h

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/include )

set(SOURCES
 qmimedatabase.cpp
 qmimeglobpattern.cpp
 qmimemagicrule.cpp
 qmimemagicrulematcher.cpp
 qmimeprovider.cpp
 qmimetype.cpp
 qmimetypeparser.cpp
)

qt4_add_resources( SOURCES mimetypes.qrc )

add_library( QtMimeTypes ${SOURCES} )

target_link_libraries( QtMimeTypes LINK_PUBLIC inqt5 ${QT_QTCORE_LIBRARY} )

set_target_properties( QtMimeTypes PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                              SOVERSION ${ECM_SOVERSION})

install(TARGETS QtMimeTypes EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install(FILES qmime_global.h qmimedatabase.h qmimetype.h DESTINATION ${INCLUDE_INSTALL_DIR}/inqt5)
install(FILES include/QMimeDatabase include/QMimeType DESTINATION ${INCLUDE_INSTALL_DIR}/inqt5)
