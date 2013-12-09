if (WIN32)
  set(kdeinit_SRCS kinit_win.cpp ../klauncher_cmds.cpp )
  set(kdeinit_LIBS psapi)
else ()
  set(kdeinit_SRCS kinit.cpp proctitle.cpp ../klauncher_cmds.cpp )
  set(kdeinit_LIBS "")
endif ()

include_directories(${KInit_BINARY_DIR}) # for kinit_version.h

# on win32 kdeinit5 has to be a console application
# to be able to catch stderr based --verbose output
add_executable(kdeinit5 ${kdeinit_SRCS})
if (APPLE)
  # this has to be GUI on OSX because it launches GUI apps and need a quartz context
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.template)
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.kdeinit5")
  set_target_properties(kdeinit5 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Init")
else ()
  ecm_mark_nongui_executable(kdeinit5)
endif ()

target_link_libraries(kdeinit5 ${kdeinit_LIBS} ${KINIT_SOCKET_LIBRARY}
    Qt5::Gui #QFont::initialize
    KF5::Service #KLibrary
    KF5::WindowSystem # KStartupInfo
    KF5::Crash # KCrash::loadedByKdeinit
    KF5::I18n #i18n() used on some error messages
)

if (X11_FOUND)
  target_link_libraries(kdeinit5 ${X11_X11_LIB})
endif()

install(TARGETS kdeinit5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

if (NOT WIN32)
  # kdeinit5_shutdown
  add_executable(kdeinit5_shutdown ../wrapper.cpp)
  ecm_mark_nongui_executable(kdeinit5_shutdown)

  target_link_libraries(kdeinit5_shutdown  ${KINIT_SOCKET_LIBRARY} Qt5::Core)

  install(TARGETS kdeinit5_shutdown ${INSTALL_TARGETS_DEFAULT_ARGS} )

  # kdeinit5_wrapper
  add_executable(kdeinit5_wrapper ../wrapper.cpp)
  ecm_mark_nongui_executable(kdeinit5_wrapper)

  target_link_libraries(kdeinit5_wrapper  ${KINIT_SOCKET_LIBRARY} Qt5::Core)

  if (NOT X11_FOUND)
    target_compile_definitions(kdeinit5_wrapper -DNO_DISPLAY)
  endif ()

  install(TARGETS kdeinit5_wrapper ${INSTALL_TARGETS_DEFAULT_ARGS} )
endif ()
