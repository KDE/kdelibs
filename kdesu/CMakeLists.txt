
project(kdesu)

include_directories( ${KDE4_KPTY_INCLUDES} )

########### next target ###############

set(kdesu_LIB_SRCS
   client.cpp
   process.cpp
   kcookie.cpp
   su.cpp
   ssh.cpp
   stub.cpp
   )


kde4_add_library(kdesu SHARED ${kdesu_LIB_SRCS})

target_link_libraries(kdesu ${KDE4_KPTY_LIBS} )
target_link_libraries(kdesu LINK_INTERFACE_LIBRARIES ${QT_QTCORE_LIBRARY} )

if(KDESU_USE_SUDO_DEFAULT)
   set_source_files_properties(su.cpp PROPERTIES 
                               COMPILE_FLAGS -DKDESU_USE_SUDO_DEFAULT)
endif(KDESU_USE_SUDO_DEFAULT)

set_target_properties(kdesu PROPERTIES
   VERSION ${KDE_NON_GENERIC_LIB_VERSION}
   SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION}
)

install(TARGETS kdesu EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kdesu_stub_SRCS kdesu_stub.c )

kde4_add_executable(kdesu_stub NOGUI ${kdesu_stub_SRCS})

install(TARGETS kdesu_stub DESTINATION ${LIBEXEC_INSTALL_DIR} )


########### install files ###############
install( FILES kdesu_export.h client.h process.h su.h ssh.h stub.h defaults.h 
         DESTINATION  ${INCLUDE_INSTALL_DIR}/kdesu COMPONENT Devel)




