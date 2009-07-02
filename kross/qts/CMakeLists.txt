include_directories(${CMAKE_SOURCE_DIR} ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES})

########### next target ###############
set(krossqtsplugin_LIB_SRCS plugin.cpp)
kde4_add_library(krossqtsplugin SHARED ${krossqtsplugin_LIB_SRCS})
target_link_libraries(krossqtsplugin ${QT_QTUITOOLS_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} krosscore ${QT_QTSCRIPT_LIBRARY})
if (NOT CYGWIN)
    set_target_properties(krossqtsplugin PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
endif (NOT CYGWIN)

install(TARGETS krossqtsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/script)

########### next target ###############

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/kross ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KDE4_KIO_INCLUDES})
set(krossqtstest_SRCS main.cpp)
kde4_add_executable(krossqtstest TEST ${krossqtstest_SRCS})
target_link_libraries(krossqtstest ${KDE4_KDEUI_LIBS} krossqtsplugin ${QT_QTSCRIPT_LIBRARY})
# set(krossqtsplugintest_SRCS main.cpp)
# target_link_libraries(krossqtsplugintest ${KDE4_KDECORE_LIBS} )
# kde4_add_executable(krossqtsplugintest ${krossqtsplugintest_SRCS})
# #install(TARGETS krossqtsplugintest DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

#include_directories(${CMAKE_SOURCE_DIR}/kross ${KDE4_KJS_INCLUDES} ${CMAKE_SOURCE_DIR}/kjsembed ${KDE4_KDECORE_INCLUDES} ${KDE4_KDEUI_INCLUDES} ${KDE4_KIO_INCLUDES})
set(krossqts_PART_SRCS interpreter.cpp script.cpp)
kde4_add_plugin(krossqts ${krossqts_PART_SRCS})
target_link_libraries(krossqts ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} krosscore ${QT_QTSCRIPT_LIBRARY})

install(TARGETS krossqts DESTINATION ${PLUGIN_INSTALL_DIR})
