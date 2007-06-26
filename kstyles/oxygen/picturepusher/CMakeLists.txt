project(oxygenPP)

include(${QT_USE_FILE})

set( oxygenPP_SRCS main.cpp )

set( oxygenPP_MOC_HDRS opp.h )

qt4_wrap_cpp(oxygenPP_MOC_SRCS ${oxygenPP_MOC_HDRS} )

kde4_add_executable(oxygenPP ${oxygenPP_SRCS} ${oxygenPP_MOC_SRCS} )

target_link_libraries( oxygenPP  ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${X11_LIBRARIES} ${X11_Xrender_LIB} )

install( TARGETS oxygenPP DESTINATION ${BIN_INSTALL_DIR} )

