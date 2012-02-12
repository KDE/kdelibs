
get_filename_component(_kf5_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH} ${_kf5_install_prefix}/share/cmake/modules)

find_package(KDE4Internal REQUIRED)

set(KDE4_KDECORE_LIBRARY ${KDE4_KDECORE_LIBS} ${KDE4_KDE4SUPPORT_LIBS})
