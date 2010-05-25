
# The crosscompiling part is commented out on purpose. Alex

# if (CMAKE_CROSSCOMPILING)
#    set(IMPORT_KCONFIG_COMPILER_EXECUTABLE "${KDE_HOST_TOOLS_PATH}/ImportKConfigCompilerExecutable.cmake" CACHE FILEPATH "Point it to the export file of kconfig_compiler from a native build")
#    include(${IMPORT_KCONFIG_COMPILER_EXECUTABLE})
#    set(KDE4_KCFGC_EXECUTABLE kconfig_compiler)
# else (CMAKE_CROSSCOMPILING)


   set(kconfig_compiler_SRCS kconfig_compiler.cpp)


   kde4_add_executable(kconfig_compiler NOGUI ${kconfig_compiler_SRCS})

   target_link_libraries(kconfig_compiler  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

   # "export" this target too so we can use the LOCATION property of the imported target in
   # FindKDE4Internal.cmake to get the full path to the installed executable instead of using FIND_PROGRAM(), Alex
   install(TARGETS kconfig_compiler EXPORT kdelibsToolsTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


#    # export this binary for cross-compilation
#
#    export(TARGETS kconfig_compiler FILE ${CMAKE_BINARY_DIR}/ImportKConfigCompilerExecutable.cmake)
# endif (CMAKE_CROSSCOMPILING)





