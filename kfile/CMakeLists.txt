project(kfile)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION "5.2")
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Widgets Xml Test)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)

include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

#configure_file(config-kfile.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kfile.h )

find_package(X11)
set(HAVE_X11 ${X11_FOUND})
if(X11_FOUND)
    set_package_properties(X11_Xrender PROPERTIES DESCRIPTION "X Rendering Extension (libXrender)"
                           URL "http://www.x.org" TYPE RECOMMENDED
                           PURPOSE "Support for compositing, rendering operations, and alpha-blending")
    set(HAVE_XRENDER ${X11_Xrender_FOUND})
endif()

if(Soprano_FOUND)
include_directories(${SOPRANO_INCLUDE_DIR})
endif(Soprano_FOUND)

add_subdirectory(tests)

set(kfile_LIB_SRCS
  kdirselectdialog.cpp
)

add_library(kfile ${kfile_LIB_SRCS})
generate_export_header(kfile)
target_link_libraries(kfile
   ${KDE4_KIO_LIBS}
   KIOFileWidgets
   ${ZLIB_LIBRARY}
   ${Solid_LIBRARIES}
   ${X11_LIBRARIES}
   KF5::KCoreAddons
   ${KWidgetsAddons_LIBRARIES}
   KI18n
   KIconThemes
   XmlGui
   KNotifications
   ItemViews
   KBookmarks
)

if(X11_Xrender_FOUND)
  target_link_libraries(kfile ${X11_Xrender_LIB} )
endif(X11_Xrender_FOUND)

target_link_libraries(kfile LINK_INTERFACE_LIBRARIES kio Qt5::Widgets KF5::KCoreAddons KCompletion KBookmarks)

set_target_properties(kfile PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                       SOVERSION ${GENERIC_LIB_SOVERSION}
                     )

install(TARGETS kfile EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
  kdirselectdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/kfile_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

############ module used by KFileDialog ##########
set(kfilemodule_SRCS
  kfilemodule.cpp
)
add_library(kfilemodule MODULE ${kfilemodule_SRCS})
target_link_libraries(kfilemodule ${KDE4_KFILE_LIBS} KIOFileWidgets ${KDE4_KIO_LIBS} KF5::KCoreAddons )
install(TARGETS kfilemodule DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kfilemodule.desktop DESTINATION ${SERVICES_INSTALL_DIR})
