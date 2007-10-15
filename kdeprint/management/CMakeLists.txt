########### next target ###############

set(kdeprint_management_LIB_SRCS
kvisiblebuttongroup.cpp
kmdbcreator.cpp
kmdriverdb.cpp
kmdriverdbwidget.cpp
kmiconview.cpp
kminfopage.cpp
kmmainview.cpp
kmpages.cpp
kmprinterview.cpp
kmjobviewer.cpp
jobitem.cpp
kmpropwidget.cpp
kmpropcontainer.cpp
kmpropertypage.cpp
kmpropgeneral.cpp
cjanuswidget.cpp
kmpropmembers.cpp
kmpropbackend.cpp
kmpropdriver.cpp
kmlistview.cpp
kminstancepage.cpp
kmtimer.cpp
kmwizard.cpp
kmwizardpage.cpp
kmwinfopage.cpp
kmwpassword.cpp
kmwsocketutil.cpp
kmwbackend.cpp
kmwsocket.cpp
kmwdriver.cpp
kmwdriverselect.cpp
kmwdrivertest.cpp
kmwinfobase.cpp
kmwname.cpp
kmwend.cpp
kmwclass.cpp
kmwlpd.cpp
kmwfile.cpp
kmdriverdialog.cpp
kmwsmb.cpp
smbview.cpp
kmconfigdialog.cpp
kmconfigpage.cpp
kmconfiggeneral.cpp
kmspecialprinterdlg.cpp
kmwlocal.cpp
kiconselectaction.cpp
kmconfigpreview.cpp
sidepixmap.cpp
pluginaction.cpp
kxmlcommanddlg.cpp
kxmlcommandselector.cpp
kmconfigcommand.cpp
kmconfigfilter.cpp
kmconfigfonts.cpp
kmconfigjobs.cpp
networkscanner.cpp
)


kde4_add_library(kdeprint_management SHARED ${kdeprint_management_LIB_SRCS})

target_link_libraries(kdeprint_management  ${KDE4_KDECORE_LIBS} kdeprint ${KDE4_KDE3SUPPORT_LIBS} )

set_target_properties(kdeprint_management PROPERTIES VERSION ${KDE_NON_GENERIC_LIB_VERSION} SOVERSION ${KDE_NON_GENERIC_LIB_SOVERSION} )
install(TARGETS kdeprint_management
             RUNTIME DESTINATION ${BIN_INSTALL_DIR}
             LIBRARY DESTINATION ${LIB_INSTALL_DIR}
             ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

########### next target ###############

set(kdeprint_management_module_PART_SRCS kdeprint_management_module.cpp )


kde4_add_plugin(kdeprint_management_module WITH_PREFIX ${kdeprint_management_module_PART_SRCS})

target_link_libraries(kdeprint_management_module  ${KDE4_KDECORE_LIBS} kdeprint_management)

install(TARGETS kdeprint_management_module  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### kaddprinterwizard ###############

set(kaddprinterwizard_KDEINIT_SRCS kaddprinterwizard.cpp )


kde4_add_kdeinit_executable( kaddprinterwizard ${kaddprinterwizard_KDEINIT_SRCS})

target_link_libraries(kdeinit_kaddprinterwizard  ${KDE4_KDECORE_LIBS} kdeprint_management)

install(TARGETS kdeinit_kaddprinterwizard 
             RUNTIME DESTINATION ${BIN_INSTALL_DIR}
             LIBRARY DESTINATION ${LIB_INSTALL_DIR}
             ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

target_link_libraries( kaddprinterwizard kdeinit_kaddprinterwizard )
install(TARGETS kaddprinterwizard DESTINATION ${LIBEXEC_INSTALL_DIR} )

########### install files ###############

install( FILES tileup.png tiledown.png side.png  DESTINATION  ${DATA_INSTALL_DIR}/kdeprint )
install( FILES  kmmainview.h kmjobviewer.h kmprinterpage.h kiconselectaction.h kmtimer.h  DESTINATION  ${INCLUDE_INSTALL_DIR}/kdeprint )


