
macro_bool_to_01(FAM_FOUND HAVE_FAM)
check_include_files(sys/inotify.h HAVE_SYS_INOTIFY_H)

configure_file(config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdirwatch.h )

########### install files ###############
install(  FILES
  connection.h
  slaveinterface.h
  slave.h
  slaveconfig.h
  sessiondata.h
  global.h
  passdlg.h
  netaccess.h
  job.h
  scheduler.h
  jobclasses.h
  copyjob.h
  deletejob.h
  jobuidelegate.h
  paste.h
  slavebase.h
  filejob.h
  progressbase.h
  defaultprogress.h
  statusbarprogress.h
  tcpslavebase.h
  forwardingslavebase.h
  observer.h
  chmodjob.h
  authinfo.h
  ioslave_defaults.h
  http_slave_defaults.h
  previewjob.h
  thumbcreator.h
  metainfojob.h
  davjob.h
  renamedlg.h
  renamedlgplugin.h
  skipdlg.h
  safesite.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kio)

install( FILES
  kmimetype.h
  kmimetypetrader.h
  kmimemagic.h
  kimageio.h
  krun.h
  kdirwatch.h
  kautomount.h
  kar.h
  ktar.h
  kzip.h
  kurifilter.h
  kurlcompletion.h
  kshellcompletion.h
  kfileitem.h
  kfileshare.h
  ksambashare.h
  knfsshare.h
  kdirlister.h
  kservicegroup.h
  kdirnotify.h
  kurlpixmapprovider.h
  kprotocolmanager.h
  kemailsettings.h
  kscan.h
  kdatatool.h
  karchive.h
  kfilefilter.h
  kfilemetainfo.h
  kmimetyperesolver.h
  kdbusservicestarter.h
  kremoteencoding.h
  kmimetypechooser.h
  kacl.h
  kdedesktopmimetype.h 
  kbuildsycocaprogressdialog.h
  DESTINATION ${INCLUDE_INSTALL_DIR}) 
