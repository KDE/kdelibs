
set(krossui_LIB_SRCS
   model.cpp
   view.cpp
   plugin.cpp
#   guiclient.cpp
   )

add_library(KrossUi ${krossui_LIB_SRCS})
target_link_libraries(KrossUi KrossCore
   KF5::KWidgetsAddons
   KF5::KIOWidgets
   KF5::KParts
   KF5::KI18n
   KF5::KIconThemes
)
set_target_properties(KrossUi PROPERTIES VERSION ${KROSS_VERSION_STRING}
                                         SOVERSION ${KROSS_SOVERSION}
                     )

target_include_directories(KrossUi PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

generate_export_header(KrossUi)
add_library(KF5::KrossUi ALIAS KrossUi)

install(TARGETS KrossUi EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

#install(FILES guiclient.h model.h view.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/krossui_export.h model.h view.h plugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui COMPONENT Devel)
