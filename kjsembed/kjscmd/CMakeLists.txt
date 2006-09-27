
project(kjsembed-kjscmd)

include_directories(
    ${CMAKE_SOURCE_DIR}/kjs
    ${CMAKE_BINARY_DIR}/kjs
    ${CMAKE_SOURCE_DIR}/kjsembed
    ${KDE4_KDECORE_INCLUDES}
)


########### next target ###############

set(kjscmd_SRCS kjscmd.cpp )

kde4_automoc(${kjscmd_SRCS})

qt4_add_resources(kjscmd_SRCS kjscmd.qrc )

kde4_add_executable(kjs ${kjscmd_SRCS})

target_link_libraries(kjs  ${KDE4_KDECORE_LIBS} kjsembed )

install(TARGETS kjs DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############





