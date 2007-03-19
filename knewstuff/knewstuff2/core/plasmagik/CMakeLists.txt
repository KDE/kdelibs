include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

 set(plasmagik_SRCS
     test.cpp
     plasmagik.cpp
     )

kde4_add_ui_files(plasmagik_SRCS plasmagikdesigned.ui)
kde4_automoc(${plasmagik_SRCS})
kde4_add_executable(plasmagik ${plasmagik_SRCS})

target_link_libraries(plasmagik ${KDE4_KIO_LIBS} knewstuff)

install(TARGETS plasmagik DESTINATION ${BIN_INSTALL_DIR})
