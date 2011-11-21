set(kgui_OPTIONAL_SRCS )
set(kgui_OPTIONAL_LIBS )

set(libkgui_SRCS
    kcolorscheme.cpp
    kcolorspaces.cpp
    kcolorutils.cpp
    kcolormimedata.cpp
    ${kgui_OPTIONAL_SRCS}
)

include_directories(
    ${KDE4_KIO_INCLUDES}
    ${CMAKE_BINARY_DIR}/kdecore/ # kde_export.h (required by kde_file.h)
    ${CMAKE_SOURCE_DIR}/kdecore/util/ # kde_file.h (need to be ported to qfile/qfileinfo/qdir)
    ${CMAKE_SOURCE_DIR}/kdecore/kernel/ # K_GLOBAL_STATIC
    ${CMAKE_SOURCE_DIR}/libinqt5/src/
    ${CMAKE_BINARY_DIR}/libinqt5/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

add_library(kgui ${LIBRARY_TYPE} ${libkgui_SRCS})
generate_export_header(kgui)

target_link_libraries(kgui ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${kgui_OPTIONAL_LIBS} kdecore kcoreaddons inqt5)
set_target_properties(kgui PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kgui ${ECM_TARGET_DEFAULT_ARGS})

install(FILES
    kcolorscheme.h
    kcolorspaces.h
    kcolorutils.h
    kcolormimedata.h
    ${CMAKE_CURRENT_BINARY_DIR}/kgui_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
