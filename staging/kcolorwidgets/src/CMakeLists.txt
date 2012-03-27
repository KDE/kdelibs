include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${DBUSMENUQT_INCLUDE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${kwidgets_BINARY_DIR}/src
 ${kwidgets_SOURCE_DIR}/src/dialogs # kdialog
 ${kwidgets_SOURCE_DIR}/src/utils
 ${kwidgets_SOURCE_DIR}/src/icons

)

set(kcolorwidgets_LIB_SRCS
 kcolorbutton.cpp
 kcolorcombo.cpp
 kcolordialog.cpp
 kcolorvalueselector.cpp
 khuesaturationselect.cpp
 kselector.cpp
 kxyselector.cpp
)

add_library(kcolorwidgets ${LIBRARY_TYPE} ${kcolorwidgets_LIB_SRCS})

generate_export_header(kcolorwidgets)

target_link_libraries(kcolorwidgets kdeguiaddons kwidgets kwindowsystem ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kcolorwidgets EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kcolorwidgets_export.h
 kcolorbutton.h
 kcolorcombo.h
 kcolordialog.h
 kcolorvalueselector.h
 khuesaturationselect.h
 kcolorchoosermode.h
 kselector.h
 kxyselector.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install( FILES 40.colors Oxygen.colors Web.colors
        Royal.colors Rainbow.colors DESTINATION ${CONFIG_INSTALL_DIR}/colors)

if (NOT Q_WS_X11)
    # install our copy of the rgb.txt named colors list on systems without X11
    install(FILES colors/rgb.txt DESTINATION ${DATA_INSTALL_DIR}/kcolorwidgets)
endif (NOT Q_WS_X11)

