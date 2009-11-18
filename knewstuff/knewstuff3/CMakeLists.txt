project(knewstuff3)

# Try to find Attica
macro_optional_find_package(LibAttica)
macro_bool_to_01(LIBATTICA_FOUND HAVE_LIBATTICA)
macro_log_feature(LIBATTICA_FOUND "libattica" "A library to access Open Collaboration Service providers" "http://kde.org" FALSE "" "Required to access OSC providers in get hot new stuff.")
configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/config-knewstuff3.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-knewstuff3.h
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)

include_directories(${LIBATTICA_INCLUDE_DIRS} ${KDE4_KIO_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${LIBATTICA_LIBRARY_DIRS})

set(knewstuff3_SRCS
    author.cpp
    downloaddialog.cpp
    entry.cpp
    knewstuffbutton.cpp
    knewstuffaction.cpp
    core/cache.cpp
    core/engine.cpp
    core/installation.cpp
    core/provider.cpp
    core/security.cpp
    core/xmlloader.cpp
    staticxml/staticxmlprovider.cpp
    ui/imageloader.cpp
    ui/itemsmodel.cpp
    ui/itemsviewdelegate.cpp
    ui/progressindicator.cpp
    ui/uploaddialog.cpp
)

if(LIBATTICA_FOUND)
    set(knewstuff3_SRCS ${knewstuff3_SRCS} attica/atticaprovider.cpp)
endif(LIBATTICA_FOUND)

kde4_add_ui_files(knewstuff3_SRCS
    downloaddialog.ui
    ui/UploadDialog.ui
)

kde4_add_library(knewstuff3 SHARED ${knewstuff3_SRCS} )

target_link_libraries(knewstuff3
    ${KDE4_KIO_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${LIBATTICA_LIBRARIES}
)

target_link_libraries(knewstuff3 LINK_INTERFACE_LIBRARIES kdeui kdecore ${QT_QTGUI_LIBRARY} )

set_target_properties(knewstuff3 PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS knewstuff3 EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  author.h
  downloaddialog.h
  entry.h
  knewstuff_export.h
  knewstuffaction.h
  knewstuffbutton.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)

