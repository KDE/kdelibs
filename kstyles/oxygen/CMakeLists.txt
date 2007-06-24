project(oxygen)

add_subdirectory( config )

if(X11_Xrender_FOUND)
   add_subdirectory( picturepusher )
endif(X11_Xrender_FOUND)

include(${QT_USE_FILE})

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
     oxrender.cpp
     gradients.cpp
     styleanimator.cpp
     visualframe.cpp
   )


set( oxygen_MOC_HDRS
     eventkiller.h
     oxygen.h
     styleanimator.h
     visualframe.h
)

qt4_wrap_cpp(oxygen_MOC_SRCS ${oxygen_MOC_HDRS})

kde4_add_plugin(oxygen ${oxygen_SRCS} ${oxygen_MOC_SRCS})

target_link_libraries( oxygen
                       ${QT_QT3SUPPORT_LIBRARY}
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
		       ${X11_LIBRARIES}
)

if(X11_Xrender_FOUND)
    target_link_libraries(oxygen ${X11_Xrender_LIB} )
endif(X11_Xrender_FOUND)

if(X11_XTest_FOUND)
      target_link_libraries(oxygen ${X11_XTest_LIB} )
endif(X11_XTest_FOUND)


install( TARGETS oxygen DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles/ )

########### install files ###############

install( FILES oxygen.themerc DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes )
