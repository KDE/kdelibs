
project(kioslave-file)

include(ConfigureChecks.cmake)
configure_file(config-kioslave-file.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kioslave-file.h )


########### next target ###############

if(WIN32)
  set(kio_file_PART_SRCS file.cpp file_win.cpp )
else(WIN32)
  set(kio_file_PART_SRCS file.cpp file_unix.cpp )
endif(WIN32)

find_package(ACL)
macro_bool_to_01(ACL_FOUND HAVE_LIBACL HAVE_POSIX_ACL)
macro_log_feature(ACL_FOUND "LibACL" "Support for manipulating access control lists" "ftp://oss.sgi.com/projects/xfs/cmd_tars" FALSE "" "STRONGLY RECOMMENDED")

kde4_add_plugin(kio_file ${kio_file_PART_SRCS})

target_link_libraries(kio_file  ${KDE4_KDECORE_LIBS} kio ${KCoreAddons_LIBRARIES} ki18n)

if (HAVE_VOLMGT AND CMAKE_SYSTEM_NAME MATCHES SunOS)
    target_link_libraries(kio_file -lvolmgt)
endif (HAVE_VOLMGT AND CMAKE_SYSTEM_NAME MATCHES SunOS)

if(ACL_FOUND)
   target_link_libraries(kio_file ${ACL_LIBS})
endif(ACL_FOUND)

install(TARGETS kio_file  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES file.protocol  DESTINATION  ${SERVICES_INSTALL_DIR} )




