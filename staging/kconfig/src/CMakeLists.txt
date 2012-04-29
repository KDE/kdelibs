include_directories(
  ${CMAKE_SOURCE_DIR}/tier1/kcoreaddons/src/io   # klockfile.h
  ${CMAKE_SOURCE_DIR}/tier1/kcodecs/src # kstringhandler.h
  ${CMAKE_BINARY_DIR}/tier1/kcoreaddons/src
  ${CMAKE_BINARY_DIR}/tier1/kcodecs/src
)

set(libkconfig_SRCS
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

add_library(kconfig ${LIBRARY_TYPE} ${libkconfig_SRCS})
generate_export_header(kconfig)

target_link_libraries(kconfig ${QT_QTCORE_LIBRARY} kcoreaddons kcodecs ${INQT5_LIBRARY})
set_target_properties(kconfig PROPERTIES VERSION   ${ECM_VERSION_STRING}
                                       SOVERSION ${ECM_SOVERSION}
)

install(TARGETS kconfig EXPORT kdelibsLibraryTargets ${ECM_TARGET_DEFAULT_ARGS})

install( FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kconfig_export.h
   conversion_check.h
   kconfig.h
   #kconfigbackend.h re-enable post-API review and implementation (4.2?)
   kconfigbase.h
   kconfiggroup.h
   kdesktopfile.h
   ksharedconfig.h
   kcoreconfigskeleton.h
   kconfiggroup_kurl.h
   kcoreauthorized.h
   ksharedptr.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
