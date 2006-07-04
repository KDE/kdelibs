
project(kdesu)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

set(kdesu_LIB_SRCS
   client.cpp
   process.cpp
   kcookie.cpp
   su.cpp
   ssh.cpp
   stub.cpp
   kdesu_pty.cpp
   )

kde4_automoc(${kdesu_LIB_SRCS})

kde4_add_library(kdesu SHARED ${kdesu_LIB_SRCS})

target_link_libraries(kdesu ${KDE4_KDECORE_LIBS} )

set_target_properties(kdesu PROPERTIES VERSION 5.0.0 SOVERSION 5 )
install(TARGETS kdesu  DESTINATION ${LIB_INSTALL_DIR} )


########### next target ###############

set(kdesu_stub_SRCS kdesu_stub.c )

kde4_add_executable(kdesu_stub NOGUI ${kdesu_stub_SRCS})

target_link_libraries(kdesu_stub  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdesu_stub DESTINATION bin)


########### install files ###############
install_files( /include/kdesu FILES  defaults.h client.h process.h kdesu_pty.h kcookie.h su.h ssh.h stub.h)




