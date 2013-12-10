set(kbuildsycoca_SRCS
   kbuildsycoca.cpp
   kbuildmimetypefactory.cpp
   kmimeassociations.cpp
   kbuildservicetypefactory.cpp
   kbuildservicefactory.cpp
   kbuildservicegroupfactory.cpp
   kctimefactory.cpp
   vfolder_menu.cpp
   )

# We need to add a '5' so that kde3/kde4 apps running kbuildsycoca don't run the kde4 one.
add_executable( kbuildsycoca5 ${kbuildsycoca_SRCS})

target_link_libraries(kbuildsycoca5
   KF5::KService #KBuildMimeTypeFactory subclasses KMimeTypeFactory, KServiceGroupFactory and KService used as well
   KF5::Crash  #set a crash handler for ksycoca binary
   KF5::CoreAddons # KAboutData
   Qt5::Xml #vfolder menui QDomDocument and family
   Qt5::DBus #kbuildsycoca uses QDBus to talk with interested applications
)

install(TARGETS kbuildsycoca5 ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############
if (WIN32)
install( FILES applications.menu  DESTINATION  ${SHARE_INSTALL_PREFIX}/xdg/menus )
else ()
install( FILES applications.menu  DESTINATION  ${SYSCONF_INSTALL_DIR}/xdg/menus )
endif ()
