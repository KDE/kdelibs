set(console_SRCS
   main.cpp
   console.cpp
   kjs_object_model.cpp
   numberedtextview.cpp
)

qt5_wrap_ui(console_SRCS jsconsole.ui)
qt5_add_resources(console_SRCS console.qrc )
add_executable(kjsconsole ${console_SRCS})
target_link_libraries(kjsconsole KF5::JsEmbed Qt5::Widgets)

install(TARGETS kjsconsole DESTINATION ${BIN_INSTALL_DIR} )
