
include(ECMMarkAsTest)

########### next target ###############
set(krossqtsplugin_LIB_SRCS plugin.cpp)
kde4_add_library(krossqtsplugin ${LIBRARY_TYPE} ${krossqtsplugin_LIB_SRCS})
target_link_libraries(krossqtsplugin ${QT_QTUITOOLS_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KCoreAddons_LIBRARIES} ki18n krosscore ${QT_QTSCRIPT_LIBRARY})
if (NOT CYGWIN)
    set_target_properties(krossqtsplugin PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
endif (NOT CYGWIN)

install(TARGETS krossqtsplugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/script)

########### next target ###############

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(krossqtstest_SRCS main.cpp)
add_executable(krossqtstest ${krossqtstest_SRCS})
ecm_mark_as_test(krossqtstest)
target_link_libraries(krossqtstest ${KDE4_KDEUI_LIBS} krossqtsplugin krosscore ${QT_QTSCRIPT_LIBRARY} ${KCoreAddons_LIBRARIES} ki18n)
# set(krossqtsplugintest_SRCS main.cpp)
# target_link_libraries(krossqtsplugintest ${KDE4_KDECORE_LIBS} )
# kde4_add_executable(krossqtsplugintest ${krossqtsplugintest_SRCS})
# #install(TARGETS krossqtsplugintest DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(krossqts_PART_SRCS interpreter.cpp script.cpp)
kde4_add_plugin(krossqts ${krossqts_PART_SRCS})
target_link_libraries(krossqts ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} ki18n krosscore ${QT_QTSCRIPT_LIBRARY})

install(TARGETS krossqts DESTINATION ${PLUGIN_INSTALL_DIR})
