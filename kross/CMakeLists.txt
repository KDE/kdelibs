# the core library
add_subdirectory( core )
# the ui library
add_subdirectory( ui )
# optional modules
add_subdirectory( modules )
# commandline-prog to run scripts
add_subdirectory( console )

# unittests and such stuff
add_subdirectory( test )

# KDE Javascript using kjs and kjsembed
# test kjsembed I think
if (NOT WINCE)
add_subdirectory( kjs )
endif(NOT WINCE)

# QtScript backend
add_subdirectory( qts )
