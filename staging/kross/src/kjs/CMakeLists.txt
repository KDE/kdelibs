
########### next target ###############

set(krosskjs_PART_SRCS
   kjsscript.cpp 
   kjsinterpreter.cpp
)

add_library(krosskjs MODULE ${krosskjs_PART_SRCS})
target_link_libraries(krosskjs KrossCore KJS KJsEmbed KI18n Qt5::Widgets)
install(TARGETS krosskjs DESTINATION ${QT_PLUGIN_INSTALL_DIR})
