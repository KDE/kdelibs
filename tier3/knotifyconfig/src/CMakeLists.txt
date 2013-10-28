configure_file(knotify-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/knotify-config.h )

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
add_library(KF5::KNotifyConfig ALIAS KNotifyConfig)

target_link_libraries(KNotifyConfig
        LINK_PUBLIC
            Qt5::Widgets
        LINK_PRIVATE
            KF5::KI18n
            KF5::KIOWidgets     # KUrlRequester
            ${PHONON_LIBS}
            Qt5::DBus)

set_target_properties(KNotifyConfig PROPERTIES
   VERSION      ${KNOTIFYCONFIG_VERSION_STRING}
   SOVERSION    ${KNOTIFYCONFIG_SOVERSION}
)
install(TARGETS KNotifyConfig EXPORT KNotifyConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_export.h knotifyconfigwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}  COMPONENT Devel)






