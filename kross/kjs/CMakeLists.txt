include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

########### next target ###############

set(krosskjs_PART_SRCS
   kjsscript.cpp 
   kjsinterpreter.cpp
)

kde4_automoc(${krosskjs_PART_SRCS})

kde4_add_plugin(krosskjs ${krosskjs_PART_SRCS})

target_link_libraries(krosskjs  ${KDE4_KDECORE_LIBS} krosscore kjs kjsembed)

install(TARGETS krosskjs DESTINATION ${PLUGIN_INSTALL_DIR})
