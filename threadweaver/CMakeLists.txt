project(threadweaver)

#find_package(Qt4 REQUIRED)
include (FindQt4)

add_subdirectory(Weaver)
add_subdirectory(WeaverGui)
add_subdirectory(Action)
add_subdirectory(Experimental)
add_subdirectory(Tests)
add_subdirectory(Examples)
