if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${KDE4_KDECORE_INCLUDES} )

########### targets ###############


endif(KDE4_BUILD_TESTS)

########### install files ###############





