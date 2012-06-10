project(tetest)
 
find_package(KDE4 REQUIRED)
include_directories(${KDE4_INCLUDES})

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


########### next target ###############

set(hello_SRCS main.cc )
kde4_add_executable(tetest ${hello_SRCS})
target_link_libraries(tetest ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} kdeui )


########### install files ###############
install(TARGETS tetest DESTINATION ${BIN_INSTALL_DIR})




