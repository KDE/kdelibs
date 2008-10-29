include_directories(
    ${CMAKE_SOURCE_DIR}/kross
    ${KDE4_KDECORE_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    ${KDE4_KIO_INCLUDES}
)

set(kross_SRCS main.cpp )
kde4_add_executable(kross ${kross_SRCS})
target_link_libraries(kross  ${KDE4_KDEUI_LIBS} krosscore )
install(TARGETS kross ${INSTALL_TARGETS_DEFAULT_ARGS})
