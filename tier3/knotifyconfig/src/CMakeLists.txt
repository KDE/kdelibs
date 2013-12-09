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

add_library(KF5NotifyConfig ${knotifyconfig_LIB_SRCS})
generate_export_header(KF5NotifyConfig BASE_NAME KNotifyConfig)
add_library(KF5::NotifyConfig ALIAS KF5NotifyConfig)

target_link_libraries(KF5NotifyConfig
        PUBLIC
            Qt5::Widgets
        PRIVATE
            KF5::I18n
            KF5::KIOWidgets     # KUrlRequester
            ${PHONON_LIBS}
            Qt5::DBus)

set_target_properties(KF5NotifyConfig PROPERTIES
   VERSION      ${KNOTIFYCONFIG_VERSION_STRING}
   SOVERSION    ${KNOTIFYCONFIG_SOVERSION}
   EXPORT_NAME  NotifyConfig
)
install(TARGETS KF5NotifyConfig EXPORT KF5NotifyConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/knotifyconfig_export.h knotifyconfigwidget.h  DESTINATION  ${INCLUDE_INSTALL_DIR}  COMPONENT Devel)






