add_subdirectory( core )
add_subdirectory( ui )
add_subdirectory( modules )
add_subdirectory( console )

# KDE Javascript using kjs and kjsembed
# test kjsembed I think
if (NOT WINCE)
add_subdirectory( kjs )
endif(NOT WINCE)

# QtScript backend
add_subdirectory( qts )
