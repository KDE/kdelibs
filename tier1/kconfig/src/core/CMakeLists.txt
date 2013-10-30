
find_package(Qt5Core 5.2.0 REQUIRED NO_MODULE)

set(libkconfigcore_SRCS
   kconfig.cpp
   kconfigbase.cpp
   kconfigdata.cpp
   kconfiggroup.cpp
   kconfigbackend.cpp
   kconfigini.cpp
   kdesktopfile.cpp
   ksharedconfig.cpp
   kcoreconfigskeleton.cpp
   kauthorized.cpp
)

add_library(KConfigCore ${libkconfigcore_SRCS})
generate_export_header(KConfigCore)
add_library(KF5::KConfigCore ALIAS KConfigCore)

target_link_libraries(KConfigCore PUBLIC Qt5::Core)
if(WIN32)
    target_link_libraries(KConfigCore PRIVATE ${KDEWIN_LIBRARIES})
endif()

if(IS_ABSOLUTE "${INCLUDE_INSTALL_DIR}")
  target_include_directories(KConfigCore INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>" )
else()
  target_include_directories(KConfigCore INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${INCLUDE_INSTALL_DIR}>" )
endif()

set_target_properties(KConfigCore PROPERTIES VERSION   ${KCONFIG_VERSION_STRING}
                                             SOVERSION ${KCONFIG_SOVERSION} )

install(TARGETS KConfigCore EXPORT KConfigTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfigcore_export.h
   conversion_check.h
   kconfig.h
   #kconfigbackend.h re-enable post-API review and implementation (4.2?)
   kconfigbase.h
   kconfiggroup.h
   kdesktopfile.h
   ksharedconfig.h
   kcoreconfigskeleton.h
   kauthorized.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
