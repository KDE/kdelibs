project(kdedglobalaccel)

###############################################################################
### KDED Global Accel Daemon
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=181)

include_directories( ${KDE4_KDEUI_INCLUDES} )

set(kded_globalaccel_SRCS
    kdedglobalaccel.cpp
    component.cpp
    globalshortcut.cpp
    globalshortcutsregistry.cpp
    globalshortcutcontext.cpp)

if ( Q_WS_X11 )
  set( kded_globalaccel_SRCS ${kded_globalaccel_SRCS} kglobalaccel_x11.cpp )
endif ( Q_WS_X11 )
if ( Q_WS_MAC )
  set( kded_globalaccel_SRCS ${kded_globalaccel_SRCS} kglobalaccel_mac.cpp )
endif ( Q_WS_MAC )
if ( Q_WS_WIN )
  set( kded_globalaccel_SRCS ${kded_globalaccel_SRCS} kglobalaccel_win.cpp )
endif ( Q_WS_WIN )
if ( Q_WS_QWS )
  set( kded_globalaccel_SRCS ${kded_globalaccel_SRCS} kglobalaccel_qws.cpp )
endif ( Q_WS_QWS )

kde4_add_plugin( kded_globalaccel ${kded_globalaccel_SRCS} )
target_link_libraries(kded_globalaccel ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

if(Q_WS_MAC)
   target_link_libraries(kded_globalaccel ${CARBON_LIBRARY})
endif(Q_WS_MAC)


install( TARGETS kded_globalaccel DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kdedglobalaccel.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
