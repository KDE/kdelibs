project(oxygenPP)

include(${QT_USE_FILE})

set( oxygenPP_SRCS main.cpp )

kde4_moc_headers(oxygenPP opp.h)
kde4_add_executable(oxygenPP ${oxygenPP_SRCS})

target_link_libraries( oxygenPP  ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${X11_LIBRARIES} ${X11_Xrender_LIB} )

install( TARGETS oxygenPP DESTINATION ${BIN_INSTALL_DIR} )

