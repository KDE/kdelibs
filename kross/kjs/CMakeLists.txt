
########### next target ###############

set(krosskjs_PART_SRCS
   kjsscript.cpp 
   kjsinterpreter.cpp
)

kde4_add_plugin(krosskjs ${krosskjs_PART_SRCS})
target_link_libraries(krosskjs krosscore KJS kjsembed KI18n Qt5::Widgets)
install(TARGETS krosskjs DESTINATION ${PLUGIN_INSTALL_DIR})
