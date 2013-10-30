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

add_library(KBookmarks ${kbookmarks_SRCS})
generate_export_header(KBookmarks)
add_library(KF5::KBookmarks ALIAS KBookmarks)

target_link_libraries(KBookmarks PUBLIC Qt5::Widgets Qt5::Xml)
target_link_libraries(KBookmarks PRIVATE
                                        KF5::KCoreAddons
                                        KF5::KWidgetsAddons
                                        KConfigWidgets # for KStandardAction
                                        XmlGui # for KActionCollection
                                        KF5::KIconThemes # for SmallIcon
)

set_target_properties(KBookmarks PROPERTIES VERSION   ${KBOOKMARKS_VERSION_STRING}
                                           SOVERSION ${KBOOKMARKS_SOVERSION}
)

install(TARGETS KBookmarks EXPORT KBookmarksTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

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
