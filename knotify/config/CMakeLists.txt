
include_directories(BEFORE ${KDE4_PHONON_INCLUDES})

########### next target ###############

set(knotifyconfig_LIB_SRCS
knotifyconfigactionswidget.cpp
knotifyconfigelement.cpp
knotifyeventlist.cpp
knotifyconfigwidget.cpp
)


set( knotifyconfig_UI knotifyconfigactionswidgetbase.ui )

kde4_add_ui_files(knotifyconfig_LIB_SRCS ${knotifyconfig_UI} )

kde4_add_library(knotifyconfig SHARED ${knotifyconfig_LIB_SRCS})

# Needs KIO for KUrlRequester
target_link_libraries(knotifyconfig  ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS}  )
target_link_libraries(knotifyconfig  LINK_INTERFACE_LIBRARIES kdecore ${QT_QTGUI_LIBRARY})

set_target_properties(knotifyconfig PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knotifyconfig EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES knotifyconfig_export.h knotifyconfigwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}  COMPONENT Devel)






