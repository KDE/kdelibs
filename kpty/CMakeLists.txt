project(kpty)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION 5.2)
find_package(KF5 5.0.0 REQUIRED MODULE COMPONENTS CMake Compiler InstallDirs)
find_package(Qt5 "${REQUIRED_QT_VERSION}" REQUIRED NO_MODULE COMPONENTS Test)

set(kpty_LIB_SRCS
   kpty.cpp
   kptydevice.cpp
   kptyprocess.cpp
)

add_library(kpty ${kpty_LIB_SRCS})
generate_export_header(kpty)

include(ConfigureChecks.cmake)
configure_file(config-pty.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-pty.h )

target_link_libraries(kpty Qt5::Core ${UTIL_LIBRARY} ${UTEMPTER_LIBRARY} KCoreAddons KI18n)
target_link_libraries(kpty LINK_INTERFACE_LIBRARIES KI18n )

target_include_directories(kpty PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/..)

set_target_properties(kpty PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kpty EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kpty_export.h
   kpty.h
   kptydevice.h
   kptyprocess.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

########### next target ###############

if (NOT HAVE_OPENPTY)
   add_executable(kgrantpty kgrantpty.c)
   ecm_mark_nongui_executable(kgrantpty)
   install(TARGETS kgrantpty DESTINATION ${LIBEXEC_INSTALL_DIR})
endif (NOT HAVE_OPENPTY)

add_subdirectory( tests )
