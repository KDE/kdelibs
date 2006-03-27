kde4_header()

add_subdirectory( example tests )



########### next target ###############

set(kconfig_compiler_SRCS kconfig_compiler.cpp)

kde4_automoc(${kconfig_compiler_SRCS})

kde4_add_executable(kconfig_compiler ${kconfig_compiler_SRCS})

target_link_libraries(kconfig_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

install_targets(/bin kconfig_compiler )


########### install files ###############


kde4_footer()

