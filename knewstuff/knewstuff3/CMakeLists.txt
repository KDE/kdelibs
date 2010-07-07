project(knewstuff3)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

include_directories(${KDE4_INCLUDES} ${KDE4_KIO_INCLUDES}  ${CMAKE_CURRENT_SOURCE_DIR}/.. ${LIBATTICA_INCLUDE_DIR})

set(knewstuff3_SRCS
    downloaddialog.cpp
    downloadwidget.cpp
    downloadmanager.cpp
    entry.cpp
    knewstuffbutton.cpp
    knewstuffaction.cpp
    core/author.cpp
    core/cache.cpp
    core/engine.cpp
    core/entryinternal.cpp
    core/installation.cpp
    core/provider.cpp
    core/security.cpp
    core/xmlloader.cpp
    staticxml/staticxmlprovider.cpp
    ui/entrydetailsdialog.cpp
    ui/imageloader.cpp
    ui/imagepreviewwidget.cpp
    ui/itemsmodel.cpp
    ui/itemsview.cpp
    ui/itemsviewdelegate.cpp
    ui/itemsgridviewdelegate.cpp
    ui/itemsviewbasedelegate.cpp
    ui/progressindicator.cpp
    ui/ratingpainter.cpp
    ui/ratingwidget.cpp
    attica/atticaprovider.cpp

    upload/atticahelper.cpp
    uploaddialog.cpp
)

kde4_add_ui_files(knewstuff3_SRCS
    downloadwidget.ui
    uploaddialog.ui
)

kde4_add_library(knewstuff3 SHARED ${knewstuff3_SRCS} )

target_link_libraries(knewstuff3
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
)

target_link_libraries(knewstuff3 ${LIBATTICA_LIBRARIES})
target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY})

set_target_properties(knewstuff3 PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS knewstuff3 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  downloaddialog.h
  downloadwidget.h
  downloadmanager.h
  uploaddialog.h
  entry.h
  knewstuff_export.h
  knewstuffaction.h
  knewstuffbutton.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)

install(FILES uploaddialog.h DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel)

