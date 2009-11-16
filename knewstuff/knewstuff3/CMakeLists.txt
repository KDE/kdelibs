project(knewstuff3)

find_package(Attica REQUIRED)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

include_directories(${ATTICA_INCLUDE_DIRS} ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/..)
link_directories(${ATTICA_LIBRARY_DIRS})

set(knewstuff3_SRCS
    author.cpp
    client.cpp
    entry.cpp
    core/cache.cpp
    core/engine.cpp
    core/installation.cpp
    core/provider.cpp
    core/security.cpp
    core/xmlloader.cpp
    staticxml/staticxmlprovider.cpp
    attica/atticaprovider.cpp
    ui/downloaddialog.cpp
    ui/itemsmodel.cpp
    ui/itemsviewdelegate.cpp
    ui/imageloader.cpp
    ui/progressindicator.cpp
    ui/uploaddialog.cpp
    #ui/providerdialog.cpp
    ui/knewstuffbutton.cpp
    ui/knewstuffaction.cpp
)

kde4_add_ui_files(knewstuff3_ui_SRCS ui/DownloadDialog.ui ui/UploadDialog.ui)

kde4_add_library(knewstuff3 SHARED ${knewstuff3_core_SRCS} ${knewstuff3_ui_SRCS} ${knewstuff3_SRCS})

target_link_libraries(knewstuff3
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${ATTICA_LIBRARIES}
)

target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY} )

set_target_properties(knewstuff3 PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knewstuff3 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ui/knewstuffaction.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3/ui COMPONENT Devel
)

install(FILES
  author.h
  client.h
  entry.h
  knewstuff_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)

