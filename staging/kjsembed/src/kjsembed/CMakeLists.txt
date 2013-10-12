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

add_library(KJsEmbed ${KJsEmbed_LIB_SRCS})
target_include_directories(KJsEmbed PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")

generate_export_header(KJsEmbed)

target_link_libraries(KJsEmbed
    LINK_PUBLIC  KF5::KI18n KJS
    LINK_PRIVATE
        #all these are required for its respective bindings
        Qt5::Widgets
        Qt5::UiTools
        Qt5::Xml
        Qt5::Svg
)
add_library(KF5::KJsEmbed ALIAS KJsEmbed)

set_target_properties(KJsEmbed PROPERTIES VERSION   "${KJSEMBED_VERSION_STRING}"
                                          SOVERSION "${KJSEMBED_SOVERSION}")

install(TARGETS KJsEmbed EXPORT KJsEmbedTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${KJsEmbed_HEADERS}
                ${CMAKE_CURRENT_BINARY_DIR}/kjsembed_export.h
                DESTINATION ${INCLUDE_INSTALL_DIR}/KJsEmbed COMPONENT Devel)





