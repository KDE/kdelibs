project(threadweaver)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

add_subdirectory(Weaver)
add_subdirectory(WeaverGui)
add_subdirectory(Action)
add_subdirectory(Experimental)
add_subdirectory(Tests)
add_subdirectory(Examples)
