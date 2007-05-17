
add_subdirectory( example )



########### next target ###############

set(kconfig_compiler_SRCS kconfig_compiler.cpp)

kde4_automoc(${kconfig_compiler_SRCS})

kde4_add_executable(kconfig_compiler NOGUI RUN_UNINSTALLED ${kconfig_compiler_SRCS})

target_link_libraries(kconfig_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

if(WIN32)
install(TARGETS kconfig_compiler DESTINATION ${BIN_INSTALL_DIR} )
else(WIN32)
install(TARGETS kconfig_compiler DESTINATION ${LIBEXEC_INSTALL_DIR} )
endif(WIN32)

########### install files ###############



