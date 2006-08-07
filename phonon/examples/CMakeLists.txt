
########### next target ###############

set(crossfade_SRCS crossfade.cpp )

kde4_automoc(${crossfade_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(crossfade ${crossfade_SRCS})

target_link_libraries(crossfade  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############





