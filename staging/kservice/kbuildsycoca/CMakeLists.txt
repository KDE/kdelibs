set(kbuildsycoca_KDEINIT_SRCS
   kbuildsycoca.cpp
   kbuildmimetypefactory.cpp
   kmimeassociations.cpp
   kbuildservicetypefactory.cpp
   kbuildservicefactory.cpp
   kbuildservicegroupfactory.cpp
   kctimefactory.cpp
   vfolder_menu.cpp
   )



# We need to add a '5' so that kde4 apps running kbuildsycoca don't run the kde4 one.
kf5_add_kdeinit_executable( kbuildsycoca5 NOGUI ${kbuildsycoca_KDEINIT_SRCS})

target_link_libraries(kdeinit_kbuildsycoca5
   KF5::KCoreAddons
   KF5::KService
   KF5::KI18n
   KF5::KCrash
   Qt5::Xml
   Qt5::DBus
)
if(NOT KBUILDSYCOCA_NO_KCRASH)
   target_link_libraries(kdeinit_kbuildsycoca5 KCrash)
endif()

install(TARGETS kdeinit_kbuildsycoca5 ${INSTALL_TARGETS_DEFAULT_ARGS})

target_link_libraries( kbuildsycoca5 kdeinit_kbuildsycoca5 )
install(TARGETS kbuildsycoca5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############
if (WIN32)
install( FILES applications.menu  DESTINATION  ${SHARE_INSTALL_PREFIX}/xdg/menus )
else (WIN32)
install( FILES applications.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
endif (WIN32)