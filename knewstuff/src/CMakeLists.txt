project(knewstuff3)

set(_INCDIR ${CMAKE_CURRENT_BINARY_DIR}/include/knewstuff3)
file(MAKE_DIRECTORY ${_INCDIR})

file(GLOB _KNEWSTUFF3HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
message(STATUS _KNEWSTUFF3HEADERS=${_KNEWSTUFF3HEADERS})
foreach(_KNEWSTUFF3HEADER ${_KNEWSTUFF3HEADERS})
    string(REGEX MATCH ".*_p\\.h" _ISPRIVATE ${_KNEWSTUFF3HEADER})
    #message(STATUS "_ISPRIVATE=${_ISPRIVATE} _KIOCOREHEADER=${_KIOCOREHEADER}")
    if (NOT _ISPRIVATE)
        if (NOT EXISTS ${_INCDIR}/${_KNEWSTUFF3HEADER})
            file(WRITE ${_INCDIR}/${_KNEWSTUFF3HEADER} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KNEWSTUFF3HEADER}\"\n")
        endif()
    endif()
endforeach()
if (NOT EXISTS ${_INCDIR}/knewstuff3_export.h)
    file(WRITE ${_INCDIR}/knewstuff3_export.h "#include \"${CMAKE_CURRENT_BINARY_DIR}/knewstuff3_export.h\"\n")
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(LibAttica 1.0.0 NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in knewstuff3"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

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

add_library(knewstuff3 ${knewstuff3_SRCS} )

target_include_directories(knewstuff3 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>")
target_link_libraries(knewstuff3
  LINK_PRIVATE
    ${KDE4_KIO_LIBS}
    KF5::KCoreAddons
    ${LibAttica_LIBRARIES}
    KF5::KArchive
    KF5::KI18n
    ItemViews
    KF5::KIconThemes
    KWindowSystem
    KTextWidgets
    XmlGui
)

target_link_libraries(knewstuff3 LINK_PUBLIC KCompletion Qt5::Widgets)

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
