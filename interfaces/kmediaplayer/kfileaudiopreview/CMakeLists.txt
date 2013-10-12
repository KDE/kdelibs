
include_directories(${PHONON_INCLUDE_DIR})

########### next target ###############

set(kfileaudiopreview_PART_SRCS
kfileaudiopreview.cpp
mediacontrols.cpp
)


add_library(kfileaudiopreview MODULE ${kfileaudiopreview_PART_SRCS})

target_link_libraries(kfileaudiopreview
   ${KDE4_KIO_LIBS}
   KIOFileWidgets    # KPreviewWidgetBase
   ${PHONON_LIBRARIES}
   KF5::KI18n
)

install(TARGETS kfileaudiopreview  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############


