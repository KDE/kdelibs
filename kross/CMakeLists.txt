# the core library
add_subdirectory( core )

# optional modules
add_subdirectory( modules )

# commandline-prog to run scripts
add_subdirectory( console )

# unittests and such stuff
if (KDE4_BUILD_TESTS)
        add_subdirectory( test )
endif (KDE4_BUILD_TESTS)

# KDE Javascript
# test kjsembed I think
add_subdirectory( kjs )
