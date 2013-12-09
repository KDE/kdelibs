set(KJsEmbed_LIB_SRCS
   kjseglobal.cpp
   binding_support.cpp
   static_binding.cpp
   variant_binding.cpp
   object_binding.cpp
   builtins.cpp
   fileio.cpp
   jseventmapper.cpp
   eventproxy.cpp
   slotproxy.cpp
   jseventutils.cpp
   qobject_binding.cpp
   kjsembed.cpp
   value_binding.cpp
   iosupport.cpp
   qwidget_binding.cpp
   qaction_binding.cpp
   qlayout_binding.cpp
   qpainter_binding.cpp
   settings.cpp
   svg_binding.cpp
   filedialog_binding.cpp
   application.cpp
   color.cpp dom.cpp font.cpp image.cpp pen.cpp
   pixmap.cpp point.cpp rect.cpp size.cpp url.cpp
   brush.cpp
   QBrush_bind.cpp
   quiloader_binding.cpp
)

add_library(KF5JsEmbed ${KJsEmbed_LIB_SRCS})
target_include_directories(KF5JsEmbed PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")

generate_export_header(KF5JsEmbed BASE_NAME KJsEmbed)

target_link_libraries(KF5JsEmbed
    PUBLIC  KF5::I18n KF5::JS
    PRIVATE
        #all these are required for its respective bindings
        Qt5::Widgets
        Qt5::UiTools
        Qt5::Xml
        Qt5::Svg
)
add_library(KF5::JsEmbed ALIAS KF5JsEmbed)

set_target_properties(KF5JsEmbed PROPERTIES VERSION   "${KJSEMBED_VERSION_STRING}"
                                            SOVERSION "${KJSEMBED_SOVERSION}"
                                            EXPORT_NAME JsEmbed
)

install(TARGETS KF5JsEmbed EXPORT KF5JsEmbedTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${KJsEmbed_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/kjsembed_export.h
                DESTINATION ${INCLUDE_INSTALL_DIR}/KJsEmbed COMPONENT Devel)





