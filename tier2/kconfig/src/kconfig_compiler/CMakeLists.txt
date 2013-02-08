
remove_definitions(-DQT_NO_KEYWORDS)

set(kconfig_compiler_SRCS kconfig_compiler.cpp)


add_executable(kconfig_compiler ${kconfig_compiler_SRCS})

target_link_libraries(kconfig_compiler  ${QT_QTXML_LIBRARY} ${QT_QTMAIN_LIBRARY} )

# "export" this target too so we can use the LOCATION property of the imported target in
# FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
install(TARGETS kconfig_compiler EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
