if (WIN32)
  set(kwrapper_SRCS kwrapper_win.cpp  )
else (WIN32)
  set(kwrapper_SRCS kwrapper.cpp  )
endif (WIN32)

add_executable(kwrapper5 ${kwrapper_SRCS})
ecm_mark_nongui_executable(kwrapper5)

target_link_libraries(kwrapper5 ${KINIT_SOCKET_LIBRARY} Qt5::Core)
install(TARGETS kwrapper5 ${INSTALL_TARGETS_DEFAULT_ARGS} )
