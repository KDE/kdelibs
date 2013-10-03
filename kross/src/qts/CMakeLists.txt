
include(ECMMarkAsTest)

########### next target ###############
set(krossqtsplugin_LIB_SRCS plugin.cpp)
add_library(krossqtsplugin ${krossqtsplugin_LIB_SRCS})
target_link_libraries(krossqtsplugin Qt5::UiTools ${KCoreAddons_LIBRARIES} KConfigCore KI18n KrossCore Qt5::Script Qt5::Widgets Qt5::Gui)
if (NOT CYGWIN)
    set_target_properties(krossqtsplugin PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
endif (NOT CYGWIN)

install(TARGETS krossqtsplugin DESTINATION ${QT_PLUGIN_INSTALL_DIR}/script)

########### next target ###############

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(krossqtstest_SRCS main.cpp)
add_executable(krossqtstest ${krossqtstest_SRCS})
ecm_mark_as_test(krossqtstest)
target_link_libraries(krossqtstest krossqtsplugin KrossCore Qt5::Script ${KCoreAddons_LIBRARIES} KI18n Qt5::Widgets)
# set(krossqtsplugintest_SRCS main.cpp)
# target_link_libraries(krossqtsplugintest ${KCoreAddons_LIBRARIES})
# kde4_add_executable(krossqtsplugintest ${krossqtsplugintest_SRCS})
# #install(TARGETS krossqtsplugintest DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(krossqts_PART_SRCS interpreter.cpp script.cpp)
add_library(krossqts MODULE ${krossqts_PART_SRCS})
target_link_libraries(krossqts ${KCoreAddons_LIBRARIES} KConfigCore KI18n KrossCore Qt5::Script Qt5::Widgets Qt5::Gui)

install(TARGETS krossqts DESTINATION ${QT_PLUGIN_INSTALL_DIR})
