
include_directories(
 ${KDE4_KDEUI_INCLUDES} # kmessagebox.h
 ${kconfig_BINARY_DIR}/src/core
 ${kconfig_SOURCE_DIR}/src/core
)

set(plugin_SRCS frameworkintegrationplugin.cpp)

add_library(frameworkintegrationplugin MODULE ${plugin_SRCS})
set_target_properties(frameworkintegrationplugin PROPERTIES PREFIX "") # remove lib prefix

target_link_libraries(frameworkintegrationplugin kdeui)

install(TARGETS frameworkintegrationplugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/kf5)



### ktranscript plugin

# kjs hash stuff for transcript plugin
set(CREATE_HASH_TABLE ${kjs_SOURCE_DIR}/src/kjs/create_hash_table)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h
    COMMAND ${PERL_EXECUTABLE} ${CREATE_HASH_TABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/ktranscript.cpp
            > ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ktranscript.cpp
)
macro_add_file_dependencies(${CMAKE_CURRENT_SOURCE_DIR}/ktranscript.cpp ${CMAKE_CURRENT_BINARY_DIR}/ktranscript.lut.h)

remove_definitions(-DQT_NO_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

include_directories(${ki18n_SOURCE_DIR}/src ${ki18n_BINARY_DIR}/src)
include_directories(${kjs_SOURCE_DIR}/src ${kjs_SOURCE_DIR}/src/kjs ${kjs_BINARY_DIR}/src ${kjs_BINARY_DIR}/src/kjs)
set(ktranscript_LIB_SRCS
    ktranscript.cpp
)

add_library(ktranscript MODULE ${ktranscript_LIB_SRCS})
set_target_properties(ktranscript PROPERTIES PREFIX "") # remove lib prefix
target_link_libraries(ktranscript kjs ki18n ${QT_QTCORE_LIBRARY})

install(TARGETS ktranscript DESTINATION ${PLUGIN_INSTALL_DIR})
