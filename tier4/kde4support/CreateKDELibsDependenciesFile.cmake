# this file is included by kdelibs/CMakeLists.txt and contains the cmake code
# which creates the KDELibsDepencies.cmake file, which is installed by kdelibs

######## write dependency file which will be installed #########

# write versions

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake
"# this file was generated during the kdelibs build process
set(KDE_VERSION_MAJOR ${KDE_VERSION_MAJOR})
set(KDE_VERSION_MINOR ${KDE_VERSION_MINOR})
set(KDE_VERSION_RELEASE ${KDE_VERSION_RELEASE})
set(KDE_VERSION \"${KDE_VERSION}\")
set(KDE_VERSION_STRING \"${KDE_VERSION_STRING}\")
set(KDE_DEFAULT_HOME \"${KDE_DEFAULT_HOME}\")
")

# write installdirs

# we need the absolute directories where stuff will be installed too
# but since the variables which contain the destinations can be relative
# or absolute paths, we need this macro to make them all absolute, Alex
macro(MAKE_INSTALL_PATH_ABSOLUTE out in)
   if (IS_ABSOLUTE "${in}")    # IS_ABSOLUTE is new since cmake 2.4.8
      set(${out} "${in}")
   else (IS_ABSOLUTE "${in}")
      set(${out} "\${KDE4_INSTALL_DIR}/${in}")
   endif (IS_ABSOLUTE "${in}")
endmacro(MAKE_INSTALL_PATH_ABSOLUTE out in)

set(KDE4_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
make_install_path_absolute(KDE4_LIB_INSTALL_DIR     ${LIB_INSTALL_DIR})
make_install_path_absolute(KDE4_IMPORTS_INSTALL_DIR     ${IMPORTS_INSTALL_DIR})
make_install_path_absolute(KDE4_LIBEXEC_INSTALL_DIR ${LIBEXEC_INSTALL_DIR})
make_install_path_absolute(KDE4_INCLUDE_INSTALL_DIR ${INCLUDE_INSTALL_DIR})
make_install_path_absolute(KDE4_BIN_INSTALL_DIR     ${BIN_INSTALL_DIR})
make_install_path_absolute(KDE4_SBIN_INSTALL_DIR    ${SBIN_INSTALL_DIR})
make_install_path_absolute(KDE4_DATA_INSTALL_DIR    ${DATA_INSTALL_DIR})
make_install_path_absolute(KDE4_HTML_INSTALL_DIR    ${HTML_INSTALL_DIR})
make_install_path_absolute(KDE4_CONFIG_INSTALL_DIR  ${CONFIG_INSTALL_DIR})
make_install_path_absolute(KDE4_ICON_INSTALL_DIR    ${ICON_INSTALL_DIR})
make_install_path_absolute(KDE4_KCFG_INSTALL_DIR    ${KCFG_INSTALL_DIR})
make_install_path_absolute(KDE4_LOCALE_INSTALL_DIR  ${LOCALE_INSTALL_DIR})
#make_install_path_absolute(KDE4_MIME_INSTALL_DIR    ${MIME_INSTALL_DIR})
make_install_path_absolute(KDE4_SOUND_INSTALL_DIR   ${SOUND_INSTALL_DIR})
make_install_path_absolute(KDE4_TEMPLATES_INSTALL_DIR    ${TEMPLATES_INSTALL_DIR})
make_install_path_absolute(KDE4_WALLPAPER_INSTALL_DIR    ${WALLPAPER_INSTALL_DIR})
make_install_path_absolute(KDE4_KCONF_UPDATE_INSTALL_DIR    ${KCONF_UPDATE_INSTALL_DIR})
make_install_path_absolute(KDE4_AUTOSTART_INSTALL_DIR    ${AUTOSTART_INSTALL_DIR})
make_install_path_absolute(KDE4_XDG_APPS_INSTALL_DIR        ${XDG_APPS_INSTALL_DIR})
make_install_path_absolute(KDE4_XDG_DIRECTORY_INSTALL_DIR   ${XDG_DIRECTORY_INSTALL_DIR})
make_install_path_absolute(KDE4_SYSCONF_INSTALL_DIR ${SYSCONF_INSTALL_DIR})
make_install_path_absolute(KDE4_MAN_INSTALL_DIR     ${MAN_INSTALL_DIR})
#make_install_path_absolute(KDE4_INFO_INSTALL_DIR    ${INFO_INSTALL_DIR})
make_install_path_absolute(KDE4_DBUS_INTERFACES_DIR ${DBUS_INTERFACES_INSTALL_DIR})
make_install_path_absolute(KDE4_DBUS_SERVICES_DIR   ${DBUS_SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4_SERVICES_INSTALL_DIR ${SERVICES_INSTALL_DIR})
make_install_path_absolute(KDE4_SERVICETYPES_INSTALL_DIR ${SERVICETYPES_INSTALL_DIR})

file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake  "
if (NOT KDE4_INSTALL_DIR)
   set(KDE4_INSTALL_DIR         \"${CMAKE_INSTALL_PREFIX}\")
endif (NOT KDE4_INSTALL_DIR)

set(KDE4_LIB_INSTALL_DIR     \"${KDE4_LIB_INSTALL_DIR}\")
set(KDE4_IMPORTS_INSTALL_DIR     \"${KDE4_IMPORTS_INSTALL_DIR}\")
set(KDE4_LIBEXEC_INSTALL_DIR \"${KDE4_LIBEXEC_INSTALL_DIR}\")
set(KDE4_INCLUDE_INSTALL_DIR \"${KDE4_INCLUDE_INSTALL_DIR}\")
set(KDE4_BIN_INSTALL_DIR     \"${KDE4_BIN_INSTALL_DIR}\")
set(KDE4_SBIN_INSTALL_DIR    \"${KDE4_SBIN_INSTALL_DIR}\")
set(KDE4_DATA_INSTALL_DIR    \"${KDE4_DATA_INSTALL_DIR}\")
set(KDE4_HTML_INSTALL_DIR    \"${KDE4_HTML_INSTALL_DIR}\")
set(KDE4_CONFIG_INSTALL_DIR  \"${KDE4_CONFIG_INSTALL_DIR}\")
set(KDE4_ICON_INSTALL_DIR    \"${KDE4_ICON_INSTALL_DIR}\")
set(KDE4_KCFG_INSTALL_DIR    \"${KDE4_KCFG_INSTALL_DIR}\")
set(KDE4_LOCALE_INSTALL_DIR  \"${KDE4_LOCALE_INSTALL_DIR}\")
set(KDE4_SOUND_INSTALL_DIR   \"${KDE4_SOUND_INSTALL_DIR}\")
set(KDE4_TEMPLATES_INSTALL_DIR \"${KDE4_TEMPLATES_INSTALL_DIR}\")
set(KDE4_WALLPAPER_INSTALL_DIR \"${KDE4_WALLPAPER_INSTALL_DIR}\")
set(KDE4_KCONF_UPDATE_INSTALL_DIR \"${KDE4_KCONF_UPDATE_INSTALL_DIR}\")
set(KDE4_AUTOSTART_INSTALL_DIR \"${KDE4_AUTOSTART_INSTALL_DIR}\")
set(KDE4_XDG_APPS_INSTALL_DIR        \"${KDE4_XDG_APPS_INSTALL_DIR}\")
set(KDE4_XDG_DIRECTORY_INSTALL_DIR   \"${KDE4_XDG_DIRECTORY_INSTALL_DIR}\")
set(KDE4_SYSCONF_INSTALL_DIR \"${KDE4_SYSCONF_INSTALL_DIR}\")
set(KDE4_MAN_INSTALL_DIR     \"${KDE4_MAN_INSTALL_DIR}\")
set(KDE4_DBUS_INTERFACES_DIR \"${KDE4_DBUS_INTERFACES_DIR}\")
set(KDE4_DBUS_SERVICES_DIR   \"${KDE4_DBUS_SERVICES_DIR}\")
set(KDE4_SERVICES_INSTALL_DIR \"${KDE4_SERVICES_INSTALL_DIR}\")
set(KDE4_SERVICETYPES_INSTALL_DIR \"${KDE4_SERVICETYPES_INSTALL_DIR}\")

set(KDE4_TARGET_PREFIX ${KDE4_TARGET_PREFIX} )

")


# write library dependencies

if (WIN32)
   file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake"
"if (NOT QT_INSTALL_DIR)
   set (QT_INSTALL_DIR \"${QT_INSTALL_DIR}\")
endif (NOT QT_INSTALL_DIR)
")
   # replace hard coded paths by variables
   # !!! the following stuff does not work because export_library_dependencies
   # !!! does not wait until the file is written completely
#        export_library_dependencies(${CMAKE_BINARY_DIR}/tmpdeps.cmake)
#        file(READ ${CMAKE_BINARY_DIR}/tmpdeps.cmake _deps)
#        string(REPLACE "${QT_INSTALL_DIR}" "\${QT_INSTALL_DIR}" __deps "${_deps}")
#        file (APPEND ${CMAKE_BINARY_DIR}/KDELibsDependencies.cmake "${__deps}")
endif(WIN32)

# This variable defines whether KPty::login/logout have been built with
# utempter support so that they don't require special user permissions
# in order to work properly. Used by kwrited.
if(HAVE_UTEMPTER)
    set(KDE4_KPTY_BUILT_WITH_UTEMPTER 1)
else()
    set(KDE4_KPTY_BUILT_WITH_UTEMPTER 0)
endif()
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake" "set(KDE4_KPTY_BUILT_WITH_UTEMPTER ${KDE4_KPTY_BUILT_WITH_UTEMPTER})")

# Append stuff needed by the KAuth framework
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/KDELibsDependencies.cmake" "

set(KDE4_AUTH_BACKEND_NAME \"${KAUTH_BACKEND_NAME}\")
set(KDE4_AUTH_HELPER_BACKEND_NAME \"${KAUTH_HELPER_BACKEND_NAME}\")
set(KDE4_AUTH_POLICY_FILES_INSTALL_DIR \"${KAUTH_POLICY_FILES_INSTALL_DIR}\" CACHE STRING \"The location where policy files generated by KAuth will be installed, if the installed backend needs to\")
")
