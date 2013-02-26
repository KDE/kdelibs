
get_filename_component(_kf5_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} ${_kf5_install_prefix}/share/cmake/modules)

find_package(KDE4Internal REQUIRED)

set(KDE4_KDECORE_LIBRARY ${KDE4_KDECORE_LIBS} ${KDE4_KDE4SUPPORT_LIBS})


list(APPEND KDE4_KDEUI_LIBS ${KDE4_KDE4SUPPORT_LIBS}  KDE4__kwidgetsaddons KDE4__ItemModels KDE4__sonnetcore KDE4__kguiaddons KDE4__kwidgets KDE4__KWindowSystem KDE4__KArchive)

list(APPEND KDE4_KIO_LIBS ${KDE4_KDEUI_LIBS} ${KDE4_KDE4SUPPORT_LIBS} KDE4__kiocore KDE4__kwidgetsaddons)
list(APPEND KDE4_KFILE_LIBS ${KDE4_KDE4SUPPORT_LIBS} KDE4__kguiaddons KDE4__KArchive  KDE4__kwidgetsaddons)
list(APPEND KDE4_KPARTS_LIBS ${KDE4_KDE4SUPPORT_LIBS} KDE4__kguiaddons KDE4__KArchive ${KDE4_KDEUI_LIBS}  KDE4__kwidgetsaddons)
list(APPEND KDE4_KHTML_LIBS KDE4__kguiaddons KDE4__KArchive ${KDE4_KDEUI_LIBS}  KDE4__kwidgetsaddons)

set(KDE4_KIO_LIBRARY ${KDE4_KIO_LIBS})
set(KDE4_KDEUI_LIBRARY ${KDE4_KDEUI_LIBS})

set(KF5Transitional_FOUND True)
