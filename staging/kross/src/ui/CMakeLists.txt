
set(krossui_LIB_SRCS
   model.cpp
   view.cpp
   plugin.cpp
#   guiclient.cpp
   )

add_library(KrossUi ${krossui_LIB_SRCS})
target_link_libraries(KrossUi KrossCore KParts ${KDE4_KIO_LIBS} KF5::KCoreAddons KI18n XmlGui KIconThemes)
set_target_properties(KrossUi PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                         SOVERSION ${GENERIC_LIB_SOVERSION}
                     )

target_include_directories(KrossUi PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

generate_export_header(KrossUi)
add_library(KF5::KrossUi ALIAS KrossUi)

install(TARGETS KrossUi EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

#install(FILES guiclient.h model.h view.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/krossui_export.h model.h view.h plugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui COMPONENT Devel)
