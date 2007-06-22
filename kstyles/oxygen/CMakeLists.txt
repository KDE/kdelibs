project(kstyle-oxygen)

add_subdirectory( config )

########### next target ###############

set( oxygen_SRCS
     oxygen.cpp
     tileset.cpp
     stylehint.cpp
     sizefromcontents.cpp
     qsubcmetrics.cpp
     pixelmetric.cpp
     stdpix.cpp
     drawcomplexcontrol.cpp
     drawcontrol.cpp
     drawprimitive.cpp
     dynamicbrush.cpp
     oxrender.cpp
   )

qt4_add_resources(oxygen.qrc oxygen.qrc)

kde4_automoc( ${oxygen_SRCS} )
kde4_add_plugin(kstyle-oxygen ${oxygen_SRCS} )

target_link_libraries( kstyle-oxygen
	${QT_QT3SUPPORT_LIBRARY}
	${QT_QTCORE_LIBRARY}
	${QT_QTGUI_LIBRARY}
	${QT_QTOPENGL_LIBRARY}
	${X11_Xrender_LIB}
	${X11_XTest_LIB}
	${OPENGL_LIBRARIES}
)

install( TARGETS kstyle-oxygen DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/ )

########### install files ###############

install( FILES oxygen.themerc DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )

