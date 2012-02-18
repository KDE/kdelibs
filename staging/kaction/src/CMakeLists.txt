cmake_minimum_required(VERSION 2.8.5)

project(kdeui_kactions)

find_package(extra-cmake-modules 0.0.2 REQUIRED)


include(ECMVersion)


#include_directories(${KDE4_INCLUDES})


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${EXTRA_CMAKE_MODULES_MODULE_PATH})

#generate_export_header(kdeui_kactions)

include_directories(
${kcoreaddons_BINARY_DIR}/src
${kcoreaddons_SOURCE_DIR}/src/caching
${kcoreaddons_SOURCE_DIR}/src/io
${kcoreaddons_SOURCE_DIR}/src/jobs
${kcoreaddons_SOURCE_DIR}/src/text
${kcoreaddons_SOURCE_DIR}/src/kernel
${kcoreaddons_SOURCE_DIR}/src/randomness
)       


set(kdeui_kactions_SRCS
 actions/kactioncollection.cpp
 actions/kactioncategory.cpp
 actions/kaction.cpp
 actions/kactionmenu.cpp
 actions/kcodecaction.cpp
 actions/kdualaction.cpp
 actions/kfontaction.cpp
 actions/kfontsizeaction.cpp
 actions/kpastetextaction.cpp
 actions/krecentfilesaction.cpp
 actions/kselectaction.cpp
 actions/kstandardaction.cpp
 actions/ktoggleaction.cpp
 actions/ktoggletoolbaraction.cpp
 actions/ktogglefullscreenaction.cpp
 actions/ktoolbarlabelaction.cpp
 actions/ktoolbarpopupaction.cpp
 actions/ktoolbarspaceraction.cpp
 shortcuts/kshortcut.cpp
 shortcuts/kshortcutschemeshelper.cpp
 shortcuts/kstandardshortcut.cpp
 shortcuts/kglobalaccel.cpp
 shortcuts/kglobalshortcutinfo.cpp
 shortcuts/kglobalshortcutinfo_dbus.cpp
 shortcuts/kacceleratormanager.cpp
 shortcuts/kcheckaccelerators.cpp
 shortcuts/kgesture.cpp
 shortcuts/kgesturemap.cpp
 # ${CMAKE_SOURCE_DIR}/kdeui/shortcuts/kgesturemap.cpp
 #${CMAKE_SOURCE_DIR}/kdeui/shortcuts/kgesture.cpp
)

install( FILES
 shortcuts/kshortcut.h
 shortcuts/kstandardshortcut.h
 shortcuts/kglobalaccel.h
 shortcuts/kglobalshortcutinfo.h
 shortcuts/kglobalshortcutinfo_p.h
 shortcuts/kacceleratormanager.h
 shortcuts/kgesture.h
 actions/kactioncollection.h
 actions/kactioncategory.h
 actions/kaction.h
 actions/kactionmenu.h
 actions/kstandaraction.h
 actions/kdualaction.h
 actions/kcodecaction.h
 actions/kfontaction.h
 actions/fontsizeaction.h
 actions/kpastetextaction.h
 actions/krecentfilesaction.h
 actions/kselectaction.h
 actions/kstandardaction.h
 actions/ktoggleaction.h
 actions/ktoggletoolbaraction.h
 actions/ktogglefullscreenaction.h
 actions/ktoolbarlabelaction.h
 actions/ktoolbarpopupaction.h
 actions/ktoolbarspaceraction.h
 DESTINATION ${INCLUDE_INSTALL_DIR}
 )


add_library(kdeui_kactions ${LIBRARY_TYPE} ${kdeui_kactions_SRCS})

generate_export_header(kdeui_kactions)

set_target_properties(kdeui_kactions PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(kdeui_kactions kdeui kdecore kdecoreaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDEUI_EXTRA_LIBS})

install(TARGETS kdeui_kactions EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


