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

add_library(KParts ${KParts_LIB_SRCS})
generate_export_header(KParts EXPORT_FILE_NAME ${KParts_BINARY_DIR}/kparts/kparts_export.h)
add_library(KF5::KParts ALIAS KParts)

target_include_directories(KParts PUBLIC "$<BUILD_INTERFACE:${KParts_BINARY_DIR}>")
target_link_libraries(KParts PUBLIC  KF5::KIOCore # essential to the technology
                                     KF5::XmlGui # essential to the technology
                                     KF5::KTextWidgets # needed for KFind, as interface
                             PRIVATE KF5::KIOWidgets #browserextension.cpp uses kurifilter.h
                                     KF5::KI18n #few uses of i18n and i18nc, can be probably stripped down
                                     KF5::KNotifications #needed by browseropenorsavequestion.cpp
                                     KF5::KIconThemes #only used by KPart::iconLoader()
                      )

if(HAVE_NEPOMUK)
    target_link_libraries(KParts PRIVATE nepomuk nepomukutils)
endif(HAVE_NEPOMUK)

set_target_properties(KParts PROPERTIES VERSION   ${KPARTS_VERSION_STRING}
                                        SOVERSION ${KPARTS_SOVERSION})


install(TARGETS KParts EXPORT KPartsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES ${KParts_BINARY_DIR}/kparts/kparts_export.h
               ${KParts_HEADERS}
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )

