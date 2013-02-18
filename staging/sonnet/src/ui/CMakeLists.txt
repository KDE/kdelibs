project(sonnetui)

include_directories(
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_BINARY_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
    ${CMAKE_CURRENT_BINARY_DIR}/../core
)

set(sonnetui_SRCS
    configdialog.cpp
    configwidget.cpp
    dialog.cpp
    dictionarycombobox.cpp
    highlighter.cpp
)

set(sonnetui_UI
    configui.ui
    sonnetui.ui
)

set(sonnetui_HDRS
    sonnet/dialog.h
    sonnet/highlighter.h
    sonnet/configdialog.h
    sonnet/configwidget.h
    sonnet/dictionarycombobox.h
)

qt4_wrap_ui(sonnetui_SRCS ${sonnetui_UI})

add_library(sonnetui ${sonnetui_SRCS})
generate_export_header(sonnetui)

target_link_libraries(sonnetui ${QT_QTCORE_LIBRARY} ${QT_QTWIDGETS_LIBRARY} sonnetcore)

set_target_properties(sonnetui PROPERTIES VERSION ${ECM_VERSION_STRING}
                                          SOVERSION ${ECM_SOVERSION}
)

install(TARGETS sonnetui EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES ${sonnetui_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/sonnetui_export.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/sonnet COMPONENT Devel)

