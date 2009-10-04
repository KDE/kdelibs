project(knewstuff3)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

include_directories(${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(knewstuff3_core_SRCS
   core/ktranslatable.cpp
   core/author.cpp
   core/category.cpp
#   core/entryhandler.cpp
#   core/entryloader.cpp
   core/feed.cpp
#   core/providerhandler.cpp
#   core/providerloader.cpp
   core/installation.cpp
   core/security.cpp
)

set(knewstuff3_ui_SRCS
  ui/kdxsrating.cpp
  ui/kdxscomment.cpp
  ui/kdxscomments.cpp
  ui/kdxschanges.cpp
  ui/kdxstranslation.cpp
  ui/downloaddialog.cpp
  ui/itemsmodel.cpp
  ui/itemsviewdelegate.cpp
  ui/qasyncimage.cpp
  ui/qstarframe.cpp
  ui/qnowtooltip.cpp
  ui/qprogressindicator.cpp
  ui/uploaddialog.cpp
  ui/providerdialog.cpp
  ui/knewstuffbutton.cpp
  ui/knewstuffaction.cpp
)

set(knewstuff3_SRCS
  client.cpp
  engine.cpp
  entry.cpp
  provider.cpp
)

kde4_add_ui_files(knewstuff3_ui_SRCS ui/DownloadDialog.ui ui/UploadDialog.ui)

kde4_add_library(knewstuff3 SHARED ${knewstuff3_core_SRCS} ${knewstuff3_ui_SRCS} ${knewstuff3_SRCS})

target_link_libraries(knewstuff3 ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY})
target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY} )

set_target_properties(knewstuff3 PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knewstuff3 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  core/author.h
  entry.h
  core/category.h
  core/ktranslatable.h
  core/installation.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/core COMPONENT Devel
)

#install(FILES dxs/dxsengine.h DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/dxs)

install(FILES
  ui/knewstuffaction.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/ui COMPONENT Devel
)

install(FILES
  client.h
  entry.h
  knewstuff_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)


