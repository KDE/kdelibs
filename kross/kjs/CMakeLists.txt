include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KJS_INCLUDES}
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${CMAKE_BINARY_DIR}/kjsembed/kjsembed
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${ki18n_SOURCE_DIR}/src
    ${ki18n_BINARY_DIR}/src
)

########### next target ###############

set(krosskjs_PART_SRCS
   kjsscript.cpp 
   kjsinterpreter.cpp
)

kde4_add_plugin(krosskjs ${krosskjs_PART_SRCS})
target_link_libraries(krosskjs ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} krosscore kjs kjsembed ki18n)
install(TARGETS krosskjs DESTINATION ${PLUGIN_INSTALL_DIR})
