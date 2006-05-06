kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kde3support )

########### next target ###############

set(kunittest_LIB_SRCS runner.cpp tester.cpp )

kde4_automoc(${kunittest_LIB_SRCS})

kde4_add_library(kunittest SHARED ${kunittest_LIB_SRCS})

target_link_libraries(kunittest ${KDE4_KDECORE_LIBS} )

set_target_properties(kunittest PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install_targets(${LIB_INSTALL_DIR} kunittest )


########### next target ###############

set(kunittestmodrunner_SRCS modrunner.cpp )

kde4_automoc(${kunittestmodrunner_SRCS})

kde4_add_executable(kunittestmodrunner NOGUI ${kunittestmodrunner_SRCS})

target_link_libraries(kunittestmodrunner ${KDE4_KDECORE_LIBS} kunittest)

install(TARGETS kunittestmodrunner DESTINATION bin)


########### install files ###############
install_files( /include/kunittest FILES runner.h tester.h module.h )


kde4_footer()
