project(KParts)

add_subdirectory( tests )
add_subdirectory( autotests )

########### next target ###############

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

add_library(KParts SHARED ${KParts_LIB_SRCS})
generate_export_header(KParts)

target_include_directories(KParts PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")
target_link_libraries(KParts LINK_PUBLIC KIOCore KService XmlGui
                             LINK_PRIVATE KCoreAddons KIOWidgets KI18n KWindowSystem KNotifications KIconThemes KTextWidgets KJobWidgets)

if(HAVE_NEPOMUK)
  target_link_libraries(KParts LINK_PRIVATE nepomuk nepomukutils)
endif(HAVE_NEPOMUK)

set_target_properties(KParts PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                        SOVERSION ${GENERIC_LIB_SOVERSION}
                     )

install(TARGETS KParts EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES kpart.desktop krop.desktop krwp.desktop browserview.desktop   DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kparts_export.h part.h plugin.h partmanager.h mainwindow.h event.h
         browserextension.h historyprovider.h browserinterface.h
         browserrun.h statusbarextension.h browseropenorsavequestion.h
         scriptableextension.h textextension.h htmlextension.h fileinfoextension.h
         listingextension.h
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kparts COMPONENT Devel )
