
project(kjsembed-kjscmd)

include_directories(
    ${CMAKE_SOURCE_DIR}/kjs
    ${CMAKE_BINARY_DIR}/kjs
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
)


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )

kde4_automoc(${kjscmd_SRCS})

qt4_add_resources(kjscmd_SRCS kjscmd.qrc )

kde4_add_executable(kjscmd ${kjscmd_SRCS})

target_link_libraries(kjscmd ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} kjsembed )

install(TARGETS kjscmd DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





