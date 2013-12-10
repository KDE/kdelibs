include_directories(${PHONON_INCLUDE_DIR})

set(kfileaudiopreview_PART_SRCS
   kfileaudiopreview.cpp
   mediacontrols.cpp
)

add_library(kfileaudiopreview MODULE ${kfileaudiopreview_PART_SRCS})

target_link_libraries(kfileaudiopreview
   ${PHONON_LIBRARIES}
   KF5::KIOFileWidgets    # KPreviewWidgetBase
   KF5::I18n
)

install(TARGETS kfileaudiopreview
        DESTINATION ${PLUGIN_INSTALL_DIR})
