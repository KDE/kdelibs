set(KParts_LIB_SRCS
    part.cpp
    plugin.cpp
    partmanager.cpp
    mainwindow.cpp
    event.cpp
    browserextension.cpp
    historyprovider.cpp
    browserinterface.cpp
    browserrun.cpp
    browseropenorsavequestion.cpp
    statusbarextension.cpp
    scriptableextension.cpp
    textextension.cpp
    htmlextension.cpp
    fileinfoextension.cpp
    listingextension.cpp
)

set(KParts_HEADERS
    part.h plugin.h partmanager.h mainwindow.h event.h
    browserextension.h historyprovider.h browserinterface.h
    browserrun.h statusbarextension.h browseropenorsavequestion.h
    scriptableextension.h textextension.h htmlextension.h fileinfoextension.h
    listingextension.h
)

foreach(_KPARTS_HEADER ${KParts_HEADERS})
    set(HEADER_NAME ${KParts_BINARY_DIR}/kparts/${_KPARTS_HEADER})
    if (NOT EXISTS ${HEADER_NAME})
        file(WRITE ${HEADER_NAME} "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_KPARTS_HEADER}\"\n")
    endif()
endforeach()

add_library(KParts ${KParts_LIB_SRCS})
generate_export_header(KParts EXPORT_FILE_NAME ${KParts_BINARY_DIR}/kparts/kparts_export.h)
add_library(KF5::KParts ALIAS KParts)

target_include_directories(KParts PUBLIC "$<BUILD_INTERFACE:${KParts_BINARY_DIR}>")
target_link_libraries(KParts LINK_PUBLIC  KIOCore # essential to the technology
                                          XmlGui # essential to the technology
                                          KTextWidgets # needed for KFind, as interface
                             LINK_PRIVATE KIOWidgets #browserextension.cpp uses kurifilter.h
                                          KI18n #few uses of i18n and i18nc, can be probably stripped down
                                          KNotifications #needed by browseropenorsavequestion.cpp
                                          KIconThemes #only used by KPart::iconLoader()
                      )

if(HAVE_NEPOMUK)
    target_link_libraries(KParts LINK_PRIVATE nepomuk nepomukutils)
endif(HAVE_NEPOMUK)

set_target_properties(KParts PROPERTIES VERSION   ${KPARTS_VERSION_STRING}
                                        SOVERSION ${KPARTS_SOVERSION})


install(TARGETS KParts EXPORT KPartsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES ${KParts_BINARY_DIR}/kparts/kparts_export.h
               ${KParts_HEADERS}
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )

