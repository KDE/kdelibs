
set(krossui_LIB_SRCS
   model.cpp
   view.cpp
   plugin.cpp
#   guiclient.cpp
   )

add_library(KF5KrossUi ${krossui_LIB_SRCS})
target_link_libraries(KF5KrossUi
   KF5::KrossCore
   KF5::WidgetsAddons
   KF5::KIOWidgets
   KF5::Parts
   KF5::I18n
   KF5::IconThemes
)
set_target_properties(KF5KrossUi PROPERTIES VERSION ${KROSS_VERSION_STRING}
                                            SOVERSION ${KROSS_SOVERSION}
                                            EXPORT_NAME KrossUi
                     )

target_include_directories(KF5KrossUi PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>")

generate_export_header(KF5KrossUi BASE_NAME KrossUi)
add_library(KF5::KrossUi ALIAS KF5KrossUi)

install(TARGETS KF5KrossUi EXPORT KF5KrossTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

#install(FILES guiclient.h model.h view.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/krossui_export.h model.h view.h plugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kross/ui COMPONENT Devel)
