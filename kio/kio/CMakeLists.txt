
macro_bool_to_01(FAM_FOUND HAVE_FAM)

configure_file(config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdirwatch.h )

########### install files ###############
install_files( /include/kio FILES
  connection.h
  slaveinterface.h
  slave.h
  slaveconfig.h
  ../uiserver_stub.h
  sessiondata.h
  global.h
  passdlg.h
  netaccess.h
  job.h
  scheduler.h
  jobclasses.h
  paste.h
  slavebase.h
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
  skipdlg.h
)

install_files( /include FILES kservicetype.h
  kmimetype.h
  kmimemagic.h
  kservice.h
  kimageio.h
  krun.h
  kdirwatch.h
  kautomount.h
  kservicetypeprofile.h
  kar.h
  ktar.h
  kzip.h
  ktrader.h
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
  kdirnotify_stub.h
  kurlpixmapprovider.h
  kprotocolmanager.h
  kemailsettings.h
  kscan.h
  kdatatool.h
  karchive.h
  kfilefilter.h
  kfilemetainfo.h
  renamedlgplugin.h
  kmimetyperesolver.h
  kdcopservicestarter.h
  kremoteencoding.h
  kmimetypechooser.h
  kacl.h
  kdedesktopmimetype.h 
)
