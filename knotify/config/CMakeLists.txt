


########### next target ###############

set(knotifyconfig_LIB_SRCS
knotifyconfigactionswidget.cpp
knotifyconfigelement.cpp
knotifyeventlist.cpp
knotifyconfigwidget.cpp
)

kde4_automoc(${knotifyconfig_LIB_SRCS})

set( knotifyconfig_UI knotifyconfigactionswidgetbase.ui )

kde4_add_ui_files(knotifyconfig_LIB_SRCS ${knotifyconfig_UI} )

kde4_add_library(knotifyconfig SHARED ${knotifyconfig_LIB_SRCS})

target_link_libraries(knotifyconfig  ${KDE4_KDECORE_LIBS} kdeui kio)

set_target_properties(knotifyconfig PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS knotifyconfig  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############
install_files( /include FILES knotifyconfigwidget.h )





