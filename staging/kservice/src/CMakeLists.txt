include(CheckSymbolExists)
include(CheckFunctionExists)
check_function_exists(mmap HAVE_MMAP)
check_symbol_exists(posix_madvise "sys/mman.h" HAVE_MADVISE)
configure_file(config-ksycoca.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksycoca.h )

set(kservice_SRCS
   services/kmimetypefactory.cpp
   services/kmimetypetrader.cpp
   services/kservice.cpp
   services/kserviceaction.cpp
   services/kservicefactory.cpp
   services/kservicegroup.cpp
   services/kservicegroupfactory.cpp
   services/kserviceoffer.cpp
   services/kservicetype.cpp
   services/kservicetypefactory.cpp
   services/kservicetypeprofile.cpp
   services/kservicetypetrader.cpp
   services/ktraderparse.cpp
   services/ktraderparsetree.cpp
   services/yacc.c
   services/lex.c
   services/kplugininfo.cpp
   sycoca/ksycoca.cpp
   sycoca/ksycocadict.cpp
   sycoca/ksycocaentry.cpp
   sycoca/ksycocafactory.cpp
   sycoca/kmemfile.cpp
   plugin/klibrary.cpp
   plugin/kpluginfactory.cpp
   plugin/kpluginloader.cpp
   #plugin/kqpluginfactory.cpp
)

add_library(kservice ${kservice_SRCS})
generate_export_header(kservice)
set(kservice_includes
   ${CMAKE_CURRENT_BINARY_DIR}/.. # Since we publicly include kservice_version.h
   ${CMAKE_CURRENT_SOURCE_DIR}/services
   ${CMAKE_CURRENT_SOURCE_DIR}/sycoca
   ${CMAKE_CURRENT_SOURCE_DIR}/plugin)
target_include_directories(kservice PUBLIC "$<BUILD_INTERFACE:${kservice_includes}>")

target_link_libraries(kservice
   Qt5::DBus)
target_link_libraries(kservice
  LINK_PUBLIC
    KConfigCore
    KCoreAddons
  LINK_PRIVATE
   ki18n
)

set_target_properties(kservice PROPERTIES VERSION ${KSERVICE_VERSION_STRING}
                                        SOVERSION ${KSERVICE_SOVERSION})

install(TARGETS kservice EXPORT kserviceTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
   "${CMAKE_CURRENT_BINARY_DIR}/kservice_export.h"
   services/kmimetypetrader.h
   services/kservice.h
   services/kserviceaction.h
   services/kservicegroup.h
   #services/kservicefactory.h: do not install, internal API
   services/kservicetype.h
   #services/kservicetypefactory.h: do not install, internal API
   services/kservicetypeprofile.h
   services/kservicetypetrader.h
   #services/kserviceoffer.h: do not install, internal API
   services/kplugininfo.h
   sycoca/ksycoca.h
   sycoca/ksycocaentry.h
   sycoca/ksycocatype.h
   plugin/klibrary.h
   plugin/kpluginfactory.h
   #plugin/kqpluginfactory.h
   plugin/kpluginloader.h
   plugin/kexportplugin.h
   DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(FILES
   services/kplugininfo.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR}
)


set(kqpluginfactory_SRCS
    plugin/kqpluginfactory.cpp
    )

add_library(kqpluginfactory SHARED ${kqpluginfactory_SRCS})

target_link_libraries(kqpluginfactory
    kservice
    KConfigCore
    KCoreAddons
    ki18n)

install(TARGETS kqpluginfactory DESTINATION ${LIB_INSTALL_DIR}/kplugins)



