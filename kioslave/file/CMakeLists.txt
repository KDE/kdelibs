
project(kioslave-file)

include_directories( ${KDE4_KIO_INCLUDES} )


########### next target ###############

if(WIN32)
  set(kio_file_PART_SRCS file.cpp file_win.cpp )
else(WIN32)
  set(kio_file_PART_SRCS file.cpp file_unix.cpp )
endif(WIN32)


kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file  ${KDE4_KDECORE_LIBS} kio )

if (HAVE_VOLMGT AND CMAKE_SYSTEM_NAME MATCHES SunOS)
    target_link_libraries(kio_file -lvolmgt)
endif (HAVE_VOLMGT AND CMAKE_SYSTEM_NAME MATCHES SunOS)

if(ACL_FOUND)
   target_link_libraries(kio_file ${ACL_LIBS})
endif(ACL_FOUND)

install(TARGETS kio_file  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES file.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




