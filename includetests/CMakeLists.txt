
if(KDE4_BUILD_TESTS)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${KDE4_KDECORE_INCLUDES} ${CMAKE_SOURCE_DIR}/solid )
set(CMAKE_EXE_LINKER_FLAGS ; true )
add_definitions (-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_ASCII)
set(includetest_SRCS main.cpp
)
kde4_add_executable(includetest NOGUI ${includetest_SRCS})
endif(KDE4_BUILD_TESTS)
