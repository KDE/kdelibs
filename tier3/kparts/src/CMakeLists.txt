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

include(ECMGenerateHeaders)
ecm_generate_headers(
    Part Plugin PartManager MainWindow Event
    BrowserExtension HistoryProvider BrowserInterface
    BrowserRun StatusBarExtension BrowserOpenOrSaveQuestion
    ScriptableExtension TextExtension HtmlExtension FileInfoExtension
    ListingExtension
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KParts DESTINATION  ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
set(KParts_HEADERS
    part.h plugin.h partmanager.h mainwindow.h event.h
    browserextension.h historyprovider.h browserinterface.h
    browserrun.h statusbarextension.h browseropenorsavequestion.h
    scriptableextension.h textextension.h htmlextension.h fileinfoextension.h
    listingextension.h
)

add_library(KF5Parts ${KParts_LIB_SRCS})
generate_export_header(KF5Parts EXPORT_FILE_NAME ${KParts_BINARY_DIR}/kparts/kparts_export.h BASE_NAME KParts)
add_library(KF5::Parts ALIAS KF5Parts)

target_include_directories(KF5Parts PUBLIC "$<BUILD_INTERFACE:${KParts_BINARY_DIR}>")
target_link_libraries(KF5Parts PUBLIC  KF5::KIOWidgets #browserrun.h uses krun.h
                                       KF5::XmlGui # essential to the technology
                                       KF5::TextWidgets # needed for KFind, as interface
                               PRIVATE KF5::I18n #few uses of i18n and i18nc, can be probably stripped down
                                       KF5::Notifications #needed by browseropenorsavequestion.cpp
                                       KF5::IconThemes #only used by KPart::iconLoader()
                      )

if(HAVE_NEPOMUK)
    target_link_libraries(KF5Parts PRIVATE nepomuk nepomukutils)
endif()

set_target_properties(KF5Parts PROPERTIES VERSION   ${KPARTS_VERSION_STRING}
                                          SOVERSION ${KPARTS_SOVERSION}
                                          EXPORT_NAME Parts
)


install(TARGETS KF5Parts EXPORT KF5PartsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES ${KParts_BINARY_DIR}/kparts/kparts_export.h
               ${KParts_HEADERS}
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )

install( FILES kde_terminal_interface.h DESTINATION  ${INCLUDE_INSTALL_DIR} )

