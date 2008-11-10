
include_directories( ${CMAKE_SOURCE_DIR}/kde3support )

########### next target ###############

set(kunittest_LIB_SRCS runner.cpp tester.cpp )


kde4_add_library(kunittest SHARED ${kunittest_LIB_SRCS})

target_link_libraries(kunittest ${KDE4_KDECORE_LIBS} )
target_link_libraries(kunittest LINK_INTERFACE_LIBRARIES kdecore ${QT_QTCORE_LIBRARY} )

set_target_properties(kunittest PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS kunittest EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kunittestmodrunner_SRCS modrunner.cpp )


kde4_add_executable(kunittestmodrunner NOGUI ${kunittestmodrunner_SRCS})

target_link_libraries(kunittestmodrunner ${KDE4_KDECORE_LIBS} kunittest)

install(TARGETS kunittestmodrunner DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############
install( FILES kunittest_export.h runner.h tester.h module.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kunittest COMPONENT Devel)


