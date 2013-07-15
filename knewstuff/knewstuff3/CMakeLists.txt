project(knewstuff3)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

find_package(LibAttica NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "Support for Get Hot New Stuff"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

include_directories(
   ${LibAttica_INCLUDE_DIRS}
)

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
    attica/atticaprovider.cpp

    upload/atticahelper.cpp
    uploaddialog.cpp
)

kde4_add_ui_files(knewstuff3_SRCS
    downloadwidget.ui
    uploaddialog.ui
)

kde4_add_library(knewstuff3 ${LIBRARY_TYPE} ${knewstuff3_SRCS} )

target_include_directories(knewstuff3 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(knewstuff3
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KCoreAddons_LIBRARIES}
    KWidgets
    ${LibAttica_LIBRARIES}
    ${KArchive_LIBRARIES}
    KI18n
)

target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore KCompletion Qt5::Widgets)

generate_export_header(knewstuff3)

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
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff3_export.h
  knewstuffaction.h
  knewstuffbutton.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)
