project(kjsembed-kjscmd)

set(kjscmd_SRCS kjscmd.cpp )
qt5_add_resources(kjscmd_SRCS kjscmd.qrc)

add_executable(kjscmd ${kjscmd_SRCS})
target_link_libraries(kjscmd
    Qt5::Widgets # QApplication::lastWindowClosed
    KJsEmbed
)

install(TARGETS kjscmd EXPORT KJsEmbedTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
