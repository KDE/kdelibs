# sonnet spell checking
# this consists of libsonnetcore & libsonnetui + plugins
project(sonnet)

add_definitions(-DQT_NO_CAST_FROM_ASCII)

add_subdirectory( core )
add_subdirectory( plugins )
