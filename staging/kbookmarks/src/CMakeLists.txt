# TODO
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(kbookmarks_SRCS
  kbookmark.cc
  kbookmarkimporter.cc
  kbookmarkmanager.cc
  kbookmarkmanageradaptor.cc
  kbookmarkmenu.cc
  konqbookmarkmenu.cc
  kbookmarkimporter_opera.cc
  kbookmarkimporter_ie.cc
  kbookmarkimporter_ns.cc
  kbookmarkdombuilder.cc
  kbookmarkdialog.cc
)

add_library(KBookmarks ${kbookmarks_SRCS})
generate_export_header(KBookmarks)

target_link_libraries(KBookmarks LINK_PUBLIC Qt5::Core Qt5::Widgets Qt5::Xml KDE4Support)
target_link_libraries(KBookmarks LINK_PRIVATE ${KCoreAddons_LIBRARIES} ${KWidgetsAddons_LIBRARIES} KIOCore KIOWidgets KConfigWidgets XmlGui KIconThemes)
# KIOCore for KIO::iconNameForUrl
# KIOWidgets for KRun
# KConfigWidgets for KStandardAction
# XMLGui for KActionCollection
# KIconThemes for SmallIcon
# KDE4Support for KDialog < - FIXME

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
  kbookmarkimporter_opera.h
  kbookmarkimporter_ie.h
  kbookmarkimporter_ns.h
  kbookmarkdombuilder.h
  konqbookmarkmenu.h
  kbookmarkdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/kbookmarks_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
