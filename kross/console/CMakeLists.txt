include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

set(kross_SRCS main.cpp )
kde4_add_executable(kross ${kross_SRCS})
if(WIN32)
    target_link_libraries(kross  ${KDE4_KDEUI_LIBS} krosscore )
else(WIN32)
    target_link_libraries(kross  ${KDE4_KDECORE_LIBS} krosscore )
endif(WIN32)
install(TARGETS kross DESTINATION ${BIN_INSTALL_DIR})
