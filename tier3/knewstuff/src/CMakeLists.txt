project(KNewStuff)

set(_INCDIR ${CMAKE_CURRENT_BINARY_DIR}/include/KNewStuff)
file(MAKE_DIRECTORY ${_INCDIR})

file(GLOB _KNEWSTUFF3HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
foreach(_KNEWSTUFF3HEADER ${_KNEWSTUFF3HEADERS})
    string(REGEX MATCH ".*_p\\.h" _ISPRIVATE ${_KNEWSTUFF3HEADER})
    #message(STATUS "_ISPRIVATE=${_ISPRIVATE} _KIOCOREHEADER=${_KIOCOREHEADER}")
    if (NOT _ISPRIVATE)
        if (NOT EXISTS ${_INCDIR}/${_KNEWSTUFF3HEADER})
            file(WRITE ${_INCDIR}/${_KNEWSTUFF3HEADER} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KNEWSTUFF3HEADER}\"\n")
        endif()
    endif()
endforeach()
if (NOT EXISTS ${_INCDIR}/knewstuff_export.h)
    file(WRITE ${_INCDIR}/knewstuff_export.h "#include \"${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h\"\n")
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(LibAttica 1.0.0 NO_MODULE)
set_package_properties(LibAttica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in KNewStuff"
                       URL "https://projects.kde.org/attica"
                       TYPE REQUIRED
                      )

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

set(KNewStuff_SRCS
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

qt5_wrap_ui(KNewStuff_SRCS
    downloadwidget.ui
    uploaddialog.ui
)

add_library(KNewStuff ${KNewStuff_SRCS} )
add_library(KF5::KNewStuff ALIAS KNewStuff)

target_include_directories(KNewStuff PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR}>")
target_link_libraries(KNewStuff
  PUBLIC
    Qt5::Widgets
    ${LibAttica_LIBRARIES}     # For interacting with ocs providers, public for uploaddialog slots
    KF5::XmlGui                # For KActionCollection in knewstuffaction.h
  PRIVATE
    KF5::KIOCore               # For network fetching operations
    KF5::KIOWidgets            # For KUrlRequester
    KF5::KArchive              # For decompressing archives
    KF5::KI18n                 # For translations
    KF5::ItemViews             # For buttons on download dialog
    KF5::KIconThemes           # For KIcon
    KF5::KTextWidgets          # For KTextEdit in upload dialog
)

generate_export_header(KNewStuff)

set_target_properties(KNewStuff PROPERTIES
   VERSION ${KNEWSTUFF_VERSION_STRING}
   SOVERSION ${KNEWSTUFF_SOVERSION}
)

install(TARGETS KNewStuff EXPORT KNewStuffTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  downloaddialog.h
  downloadwidget.h
  downloadmanager.h
  uploaddialog.h
  entry.h
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
  knewstuffaction.h
  knewstuffbutton.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff3 COMPONENT Devel
)
