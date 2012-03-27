include_directories(
 ${KDE4_KDECORE_INCLUDES}
 ${CMAKE_CURRENT_SOURCE_DIR}/icons
 ${CMAKE_CURRENT_SOURCE_DIR}/widgets
 ${CMAKE_CURRENT_SOURCE_DIR}/utils
 ${DBUSMENUQT_INCLUDE_DIR}
 ${kdeguiaddons_BINARY_DIR}/src
 ${kdeguiaddons_SOURCE_DIR}/src/colors # kcolorscheme
 ${kauth_BINARY_DIR}/src
 ${kauth_SOURCE_DIR}/src # kauthaction
 ${kwindowsystem_BINARY_DIR}/src
 ${kwindowsystem_SOURCE_DIR}/src # netwm
)

set(kwidgets_LIB_SRCS
 widgets/khbox.cpp
 widgets/kvbox.cpp
 widgets/kseparator.cpp
 widgets/kurllabel.cpp
 widgets/kpushbutton.cpp
 dialogs/kdialog.cpp
 icons/kicon.cpp
 icons/kiconloader.cpp
 icons/kicontheme.cpp
 icons/kiconengine.cpp
 icons/kiconeffect.cpp
 utils/kguiitem.cpp
 utils/kstandardguiitem.cpp
 utils/kglobalsettings.cpp
)

add_library(kwidgets ${LIBRARY_TYPE} ${kwidgets_LIB_SRCS})

generate_export_header(kwidgets)

target_link_libraries(kwidgets ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY}
    ${QT_QTSVG_LIBRARY} ${KDEUI_EXTRA_LIBS} ${X11_LIBRARIES} kdeguiaddons kwindowsystem kauth)

if(X11_Xcursor_FOUND)
    target_link_libraries(kwidgets ${X11_Xcursor_LIB})
endif(X11_Xcursor_FOUND)

install(TARGETS kwidgets EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
 ${CMAKE_CURRENT_BINARY_DIR}/kwidgets_export.h
 widgets/khbox.h
 widgets/kvbox.h
 widgets/kseparator.h
 widgets/kurllabel.h
 widgets/kpushbutton.h
 dialogs/kdialog.h
 icons/kicon.h
 icons/kiconloader.h
 icons/kicontheme.h
 icons/kiconeffect.h
 utils/kguiitem.h
 utils/kstandardguiitem.h
 utils/kglobalsettings.h

 DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
