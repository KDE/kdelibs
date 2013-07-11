configure_file(knotify-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/knotify-config.h )

find_package(Phonon4Qt5 4.6.60 NO_MODULE)

macro_bool_to_01(Phonon4Qt5_FOUND HAVE_PHONON)

include_directories(BEFORE ${PHONON_INCLUDES})

########### next target ###############

set(knotifyconfig_LIB_SRCS
knotifyconfigactionswidget.cpp
knotifyconfigelement.cpp
knotifyeventlist.cpp
knotifyconfigwidget.cpp
)


set( knotifyconfig_UI knotifyconfigactionswidgetbase.ui )

kde4_add_ui_files(knotifyconfig_LIB_SRCS ${knotifyconfig_UI} )

kde4_add_library(knotifyconfig ${LIBRARY_TYPE} ${knotifyconfig_LIB_SRCS})
generate_export_header(knotifyconfig)

# Needs KIO for KUrlRequester
target_link_libraries(knotifyconfig  ${KDE4_KIO_LIBS} ${PHONON_LIBS} ${KCoreAddons_LIBRARIES} KWidgets ki18n)
target_link_libraries(knotifyconfig  LINK_INTERFACE_LIBRARIES kdecore Qt5::Widgets)

set_target_properties(knotifyconfig PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS knotifyconfig EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_export.h knotifyconfigwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}  COMPONENT Devel)






