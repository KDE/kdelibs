include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckPrototypeDefinition)

check_function_exists(strlcpy HAVE_STRLCPY)
check_prototype_definition(strlcpy "unsigned long strlcpy(char*, const char*, unsigned long)" "0" string.h HAVE_STRLCPY_PROTO)
check_include_files(string.h HAVE_STRING_H)

set(kcrash_SRCS
  kcrash.cpp
  strlcpy-fake.c
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-kcrash.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kcrash.h
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-strlcpy.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-strlcpy.h
)

add_library(KF5Crash ${kcrash_SRCS})
generate_export_header(KF5Crash BASE_NAME KCrash)
add_library(KF5::Crash ALIAS KF5Crash)

target_link_libraries(KF5Crash PUBLIC Qt5::Core)
target_link_libraries(KF5Crash PRIVATE KF5::CoreAddons KF5::WindowSystem Qt5::Gui)


if(${X11_FOUND})
  find_package(Qt5X11Extras 5.2.0 REQUIRED NO_MODULE)
  target_link_libraries(KF5Crash PRIVATE Qt5::X11Extras ${X11_LIBRARIES})
endif()

set_target_properties(KF5Crash PROPERTIES VERSION   ${KCrash_VERSION_STRING}
                                          SOVERSION ${KCrash_SOVERSION}
                                          EXPORT_NAME Crash
)

install(TARGETS KF5Crash EXPORT KF5CrashTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kcrash.h
  ${CMAKE_CURRENT_BINARY_DIR}/kcrash_export.h
  DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
