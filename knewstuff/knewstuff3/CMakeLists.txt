project(knewstuff3)

find_package(Attica REQUIRED)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

include_directories(${ATTICA_INCLUDE_DIRS} ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)
link_directories(${ATTICA_LIBRARY_DIRS})

set(knewstuff3_core_SRCS
   core/author.cpp
   core/cache.cpp
   core/category.cpp
   core/entry.cpp
   core/installation.cpp
   core/ktranslatable.cpp
   core/provider.cpp
   core/security.cpp
   staticxml/staticxmlprovider.cpp
   attica/atticaprovider.cpp
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
  ui/imageloader.cpp
  ui/qstarframe.cpp
  ui/qnowtooltip.cpp
  ui/progressindicator.cpp
  ui/uploaddialog.cpp
  ui/providerdialog.cpp
  ui/knewstuffbutton.cpp
  ui/knewstuffaction.cpp
)

set(knewstuff3_SRCS
  client.cpp
  engine.cpp
  xmlloader.cpp
)

kde4_add_ui_files(knewstuff3_ui_SRCS ui/DownloadDialog.ui ui/UploadDialog.ui)

kde4_add_library(knewstuff3 SHARED ${knewstuff3_core_SRCS} ${knewstuff3_ui_SRCS} ${knewstuff3_SRCS})

target_link_libraries(knewstuff3 ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} ${ATTICA_LIBRARIES} )
target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY} )

set_target_properties(knewstuff3 PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knewstuff3 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  core/author.h
  core/category.h
  core/ktranslatable.h
  core/installation.h
  core/provider.h
  core/entry.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/core COMPONENT Devel
)

#install(FILES dxs/dxsengine.h DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/dxs)

install(FILES
  ui/knewstuffaction.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/ui COMPONENT Devel
)

install(FILES
  client.h
  engine.h
  knewstuff_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)


