# explicitely install the "public" cmake files

#do not install FindLibKNotificationItem-1.cmake at this time
#set(cmakeFiles FindLibKNotificationItem-1.cmake)
install(FILES ${cmakeFiles} DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

