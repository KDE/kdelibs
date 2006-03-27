kde4_header()



########### next target ###############

#set(example_SRCS
#example.cpp
#exampleprefs_base.cpp
#)
#
#kde4_automoc(${example_SRCS})

if(KDE4_BUILD_TESTS)

message(STATUS "Skipped kdecore/kconfig_compiler example")

#kde4_add_executable(example ${example_SRCS})
#
#target_link_libraries(example  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

#set(autoexample_SRCS
#exampleprefs_base.cpp
#autoexample.cpp
#)
#
#kde4_automoc(${autoexample_SRCS})
#
#set( autoexample_UI3
#general_base.ui3
#myoptions_base.ui3
#)
#
#kde4_add_ui3_files(autoexample_SRCS ${autoexample_UI3} )
#
if(KDE4_BUILD_TESTS)
message(STATUS "Skipped kdecore/kconfig_compiler autotest")
#kde4_add_executable(autoexample ${autoexample_SRCS})
#
#target_link_libraries(autoexample  ${KDE4_KDECORE_LIBS} kdeui )
#
endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()

