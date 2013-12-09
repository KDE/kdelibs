project(kjsembed-kjscmd)

set(kjscmd_SRCS kjscmd.cpp )
qt5_add_resources(kjscmd_SRCS kjscmd.qrc)

# This is a developer tool, not intended for normal user installs
add_executable(kjscmd ${kjscmd_SRCS})
target_link_libraries(kjscmd
    Qt5::Widgets # QApplication::lastWindowClosed
    KF5::JsEmbed
)

install(TARGETS kjscmd EXPORT KF5JsEmbedTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
