
add_subdirectory( example )



########### next target ###############

set(kconfig_compiler_SRCS kconfig_compiler.cpp)


kde4_add_executable(kconfig_compiler NOGUI RUN_UNINSTALLED ${kconfig_compiler_SRCS})

target_link_libraries(kconfig_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

install(TARGETS kconfig_compiler ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############



