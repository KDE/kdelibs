include_directories(${kcoreaddons_INCLUDE_DIRS}  ${kdeqt5staging_INCLUDE_DIRS})

set(libkconfigcore_SRCS
   kconfig.cpp
   kconfigbase.cpp
   kconfiggroup.cpp
   kconfigbackend.cpp
   kconfigini.cpp
   kdesktopfile.cpp
   ksharedconfig.cpp
   kcoreconfigskeleton.cpp
   kcoreauthorized.cpp
)

add_library(kconfigcore ${libkconfigcore_SRCS})
generate_export_header(kconfigcore)

target_link_libraries(kconfigcore ${kcoreaddons_LIBRARIES})
if(WIN32)
    target_link_libraries(kconfigcore ${KDEWIN_LIBRARIES})
endif(WIN32)
set_target_properties(kconfigcore PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                             SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kconfigcore EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

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
   kcoreauthorized.h
   ksharedptr.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
