include_directories(
${kcoreaddons_BINARY_DIR}/src
${kcoreaddons_SOURCE_DIR}/src/caching
${kcoreaddons_SOURCE_DIR}/src/io
${kcoreaddons_SOURCE_DIR}/src/jobs
${kcoreaddons_SOURCE_DIR}/src/text
${kcoreaddons_SOURCE_DIR}/src/kernel
${kcoreaddons_SOURCE_DIR}/src/randomness
${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
)       

set(kactions_SRCS
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
 ${CMAKE_CURRENT_BINARY_DIR}/kactions_export.h
 DESTINATION ${INCLUDE_INSTALL_DIR}
 )

set(kglobalaccel_component_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kactions_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface )
install(FILES ${kglobalaccel_component_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

set(kglobalaccel_xml ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts/org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES INCLUDE "kglobalshortcutinfo_p.h")
qt4_add_dbus_interface(kactions_SRCS ${kglobalaccel_xml} kglobalaccel_interface )
install(FILES ${kglobalaccel_xml} DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_library(kactions ${LIBRARY_TYPE} ${kactions_SRCS})

generate_export_header(kactions)

set_target_properties(kactions PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(kactions kdecore kdecoreaddons ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kactions EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
