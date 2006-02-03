TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )
include( $(KDELIBS)/win/zlib.pro )

# needed to export library classes:
DEFINES += MAKE_KIO_LIB

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdewin32$$KDELIB_SUFFIX

system( bash kmoc kio kfile misc bookmarks kssl )

TARGET = kio$$KDEBUG

INCLUDEPATH += $(KDELIBS)/kdecore/network $(KDELIBS)/kio/kio $(KDELIBS)/kio/misc $(KDELIBS)/kio/bookmarks \
  $(KDELIBS)/kio/kssl \
  $(KDELIBS)/libltdl $(KDELIBS)/interfaces \
  $(KDELIBS)/kio/kio/moc $(KDELIBS)/kio/misc/moc $(KDELIBS)/kio/kfile/moc \
  $(KDELIBS)/kio/misc/moc \
  $(KDELIBS)/kio/bookmarks/moc \
  $(KDELIBS)/kio/kssl/moc $(KDELIBS)/kwallet/client 

system( cd kio && dcopidl kdirnotify.h > kdirnotify.kidl && dcopidl2cpp --no-stub kdirnotify.kidl )
system( cd kio && dcopidl observer.h > observer.kidl && dcopidl2cpp observer.kidl )
system( cd bookmarks && dcopidl kbookmarknotifier.h > kbookmarknotifier.kidl && dcopidl2cpp kbookmarknotifier.kidl )
system( cd bookmarks && dcopidl kbookmarkmanager.h > kbookmarkmanager.kidl && dcopidl2cpp kbookmarkmanager.kidl )
system( cd misc && dcopidl uiserver.h > uiserver.kidl && dcopidl2cpp uiserver.kidl )


SOURCES = \
kio/authinfo.cpp \
kio/chmodjob.cpp \
kio/connection.cpp \
kio/dataprotocol.cpp \
kio/dataslave.cpp \
kio/davjob.cpp \
kio/defaultprogress.cpp \
kio/global.cpp \
kio/job.cpp \
kio/kar.cpp \
kio/karchive.cpp \
kio/kdatatool.cpp \
kio/kdcopservicestarter.cpp \
kio/kdirlister.cpp \
kio/kdirnotify.cpp \
kio/kdirwatch.cpp \
kio/kemailsettings.cpp \
kio/kfilefilter.cpp \
kio/kfileitem.cpp \
kio/kfilemetainfo.cpp \
kio/kfileshare.cpp \
kio/kfilterbase.cpp \
kio/kfilterdev.cpp \
kio/kimageio.cpp \
kio/kmimemagic.cpp \
kio/kmimetype.cpp \
kio/kmimetypechooser.cpp \
kio/knfsshare.cpp \
kio/kprotocolinfo.cpp \
kio/kprotocolmanager.cpp \
kio/kremoteencoding.cpp \
kio/krun.cpp \
kio/ksambashare.cpp \
kio/kscan.cpp \
kio/kservice.cpp \
kio/kservicefactory.cpp \
kio/kservicegroup.cpp \
kio/kservicegroupfactory.cpp \
kio/kservicetype.cpp \
kio/kservicetypefactory.cpp \
kio/kshellcompletion.cpp \
kio/kshred.cpp \
kio/ktar.cpp \
kio/ktrader.cpp \
kio/ktraderparse.cpp \
kio/ktraderparsetree.cpp \
kio/kurifilter.cpp \
kio/kurlcompletion.cpp \
kio/kurlpixmapprovider.cpp \
kio/kuserprofile.cpp \
kio/kzip.cpp \
kio/lex.c \
kio/metainfojob.cpp \
kio/netaccess.cpp \
kio/observer.cpp \
kio/passdlg.cpp \
kio/paste.cpp \
kio/pastedialog.cpp \
kio/previewjob.cpp \
kio/progressbase.cpp \
kio/renamedlg.cpp \
kio/scheduler.cpp \
kio/sessiondata.cpp \
kio/skipdlg.cpp \
kio/slave.cpp \
kio/slavebase.cpp \
kio/slaveconfig.cpp \
kio/slaveinterface.cpp \
kio/statusbarprogress.cpp \
kio/tcpslavebase.cpp \
kio/yacc.c \
\
bookmarks/kbookmark.cc \
bookmarks/kbookmarkbar.cc \
bookmarks/kbookmarkdombuilder.cc \
bookmarks/kbookmarkdrag.cc \
bookmarks/kbookmarkexporter.cc \
bookmarks/kbookmarkimporter.cc \
bookmarks/kbookmarkimporter_crash.cc \
bookmarks/kbookmarkimporter_ie.cc \
bookmarks/kbookmarkimporter_kde1.cc \
bookmarks/kbookmarkimporter_ns.cc \
bookmarks/kbookmarkimporter_opera.cc \
bookmarks/kbookmarkmanager.cc \
bookmarks/kbookmarkmenu.cc \
\
kfile/kcombiview.cpp \
kfile/kcustommenueditor.cpp \
kfile/kdiroperator.cpp \
kfile/kdirselectdialog.cpp \
kfile/kdirsize.cpp \
kfile/kdiskfreesp.cpp \
kfile/kencodingfiledialog.cpp \
kfile/kfilebookmarkhandler.cpp \
kfile/kfiledetailview.cpp \
kfile/kfiledialog.cpp \
kfile/kfilefiltercombo.cpp \
kfile/kfileiconview.cpp \
kfile/kfilemetainfowidget.cpp \
kfile/kfilemetapreview.cpp \
kfile/kfilepreview.cpp \
kfile/kfilesharedlg.cpp \
kfile/kfilespeedbar.cpp \
kfile/kfiletreebranch.cpp \
kfile/kfiletreeview.cpp \
kfile/kfiletreeviewitem.cpp \
kfile/kfileview.cpp \
kfile/kicondialog.cpp \
kfile/kimagefilepreview.cpp \
kfile/kmetaprops.cpp \
kfile/knotifydialog.cpp \
kfile/kopenwith.cpp \
kfile/kpreviewprops.cpp \
kfile/kpreviewwidgetbase.cpp \
kfile/kpropertiesdialog.cpp \
kfile/krecentdirs.cpp \
kfile/krecentdocument.cpp \
kfile/kurlbar.cpp \
kfile/kurlcombobox.cpp \
kfile/kurlrequester.cpp \
kfile/kurlrequesterdlg.cpp \
\
misc/uiserver.cpp \
\
kssl/ksslcertdlg.cc \
kssl/ksslinfodlg.cc \
kssl/ksslcsessioncache.cc \
kssl/ksslsession.cc \
kssl/ksslsettings.cc \
kssl/ksslcertchain.cc \
kssl/ksslcertificate.cc \
kssl/ksslcertificatecache.cc \
kssl/ksslcertificatehome.cc \
kssl/ksslcertificatefactory.cc \
kssl/kssl.cc \
kssl/ksslconnectioninfo.cc \
kssl/ksslkeygen.cc \
kssl/ksslpkcs7.cc \
kssl/ksslpkcs12.cc \
kssl/ksslx509v3.cc \
kssl/ksslx509map.cc \
kssl/ksslsigners.cc \
kssl/ksslpeerinfo.cc \
kssl/kopenssl.cc \
kssl/ksmimecrypto.cc

#removed kio/kautomount.cpp \

SOURCES += \
../kdecore/kprotocolinfo_kdecore.cpp

# js 2004-08-05 ^^^^^ a hack because msvc cannot split a class between two libraries!

#kprotocolinfofactory.cpp \

#slavebase.cpp \

# generated:
SOURCES += \
kio/kdirnotify_stub.cpp \
kio/kdirnotify_skel.cpp \
kio/observer_stub.cpp \
kio/observer_skel.cpp \
\
bookmarks/kbookmarknotifier_stub.cpp \
bookmarks/kbookmarknotifier_skel.cpp \
bookmarks/kbookmarkmanager_stub.cpp \
bookmarks/kbookmarkmanager_skel.cpp \
\
misc/uiserver_stub.cpp \
misc/uiserver_skel.cpp


FORMS = \
kssl/keygenwizard.ui \
kssl/keygenwizard2.ui \
kfile/knotifywidgetbase.ui \
kfile/kpropertiesdesktopadvbase.ui \
kfile/kpropertiesdesktopbase.ui \
kfile/kpropertiesmimetypebase.ui
