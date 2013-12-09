# TODO
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kbookmarks_SRCS
  kbookmark.cpp
  kbookmarkimporter.cpp
  kbookmarkmanager.cpp
  kbookmarkmanageradaptor.cpp
  kbookmarkmenu.cpp
  kbookmarkowner.cpp
  konqbookmarkmenu.cpp
  kbookmarkimporter_opera.cpp
  kbookmarkimporter_ie.cpp
  kbookmarkimporter_ns.cpp
  kbookmarkdombuilder.cpp
  kbookmarkdialog.cpp
)

add_library(KF5Bookmarks ${kbookmarks_SRCS})
generate_export_header(KF5Bookmarks BASE_NAME KBookmarks)
add_library(KF5::Bookmarks ALIAS KF5Bookmarks)

target_link_libraries(KF5Bookmarks PUBLIC Qt5::Widgets Qt5::Xml)
target_link_libraries(KF5Bookmarks PRIVATE
    Qt5::DBus # dbus usage in kbookmarkmanager.cpp
    KF5::ConfigWidgets # for KStandardAction
    KF5::XmlGui # for KActionCollection
    KF5::IconThemes # for SmallIcon
)

set_target_properties(KF5Bookmarks PROPERTIES VERSION   ${KBOOKMARKS_VERSION_STRING}
                                              SOVERSION ${KBOOKMARKS_SOVERSION}
                                              EXPORT_NAME Bookmarks
)

install(TARGETS KF5Bookmarks EXPORT KF5BookmarksTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kbookmark.h
  kbookmarkexporter.h
  kbookmarkimporter.h
  kbookmarkmanager.h
  kbookmarkmenu.h
  kbookmarkowner.h
  kbookmarkimporter_opera.h
  kbookmarkimporter_ie.h
  kbookmarkimporter_ns.h
  kbookmarkdombuilder.h
  konqbookmarkmenu.h
  kbookmarkdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
