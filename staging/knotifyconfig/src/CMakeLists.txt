configure_file(knotify-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/knotify-config.h )

find_package(Phonon4Qt5 4.6.60 NO_MODULE)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(HAVE_PHONON ${Phonon4Qt5_FOUND})

include_directories(BEFORE ${PHONON_INCLUDES})

########### next target ###############

set(knotifyconfig_LIB_SRCS
knotifyconfigactionswidget.cpp
knotifyconfigelement.cpp
knotifyeventlist.cpp
knotifyconfigwidget.cpp
)


set( knotifyconfig_UI knotifyconfigactionswidgetbase.ui )

qt5_wrap_ui(knotifyconfig_LIB_SRCS ${knotifyconfig_UI} )

add_library(KNotifyConfig ${knotifyconfig_LIB_SRCS})
generate_export_header(KNotifyConfig)

# Needs KIO for KUrlRequester
target_link_libraries(KNotifyConfig
        LINK_PUBLIC
            Qt5::Widgets
        LINK_PRIVATE
            KI18n
            ${KDE4_KIO_LIBS}
            ${PHONON_LIBS}
            KCoreAddons
            Qt5::DBus)
target_link_libraries(KNotifyConfig  LINK_INTERFACE_LIBRARIES Qt5::Widgets)

set_target_properties(KNotifyConfig PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS KNotifyConfig EXPORT KNotifyConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_export.h knotifyconfigwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}  COMPONENT Devel)






