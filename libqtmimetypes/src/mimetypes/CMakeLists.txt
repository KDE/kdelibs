# used by the (non-generated) export header.
# Do not port to generate_export_header, this framework might go into Qt, using qmake rather than cmake.
add_definitions(-DQMIME_LIBRARY)


include_directories(${QT_MKSPECS_DIR}/default) # for qplatformdefs.h

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../include/QtMimeTypes )

set(SOURCES
 qmimedatabase.cpp
 mimetypeparser.cpp
 qmimeglobpattern.cpp
 qmimemagicrule.cpp
 qmimemagicrulematcher.cpp
 qmimeprovider.cpp
 qmimetype.cpp
)

qt4_add_resources( SOURCES mimetypes.qrc )

add_library( QtMimeTypes ${LIBRARY_TYPE} ${SOURCES} )

target_link_libraries( QtMimeTypes LINK_PUBLIC ${INQT5_LIBRARY} ${QT_QTCORE_LIBRARY} )

set_target_properties( QtMimeTypes PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION})

install(TARGETS QtMimeTypes ${ECM_TARGET_DEFAULT_ARGS})
