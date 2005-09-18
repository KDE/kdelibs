TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )
# for kqiodevicegzip_p.cpp
include( $(KDELIBS)/win/zlib.pro )

# needed to export library classes:
DEFINES += MAKE_KDECORE_LIB

LIBS += $$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX \
	-lqassistantclient

TARGET		= kdecore$$KDEBUG

INCLUDEPATH += $(KDELIBS)/libltdl $(KDELIBS)/kdecore/network

system( bash kmoc )
system( bash kdcopidl )

SOURCES += \
fakes.c \
kpixmapprovider.cpp \
kpalette.cpp \
kprocess.cpp \
kprocio.cpp \
kcrash.cpp \
kallocator.cpp \
knotifyclient.cpp \
kcompletionbase.cpp \
kcompletion.cpp \
kmimesourcefactory.cpp \
ksimpleconfig.cpp \
libintl.cpp \
kcatalogue.cpp \
kcalendarsystem.cpp \
kcalendarsystemfactory.cpp \
kcalendarsystemgregorian.cpp \
kcalendarsystemhebrew.cpp \
kcalendarsystemhijri.cpp \
kcalendarsystemjalali.cpp \
kaboutdata.cpp \
kstringhandler.cpp \
kcmdlineargs.cpp \
kurldrag.cpp \
kurl.cpp \
kidna.cpp \
kstaticdeleter.cpp \
kstandarddirs.cpp \
kconfig.cpp \
kconfigdialogmanager.cpp \
kcharsets.cpp \
kglobal.cpp \
kdebug.cpp \
ktempfile.cpp \
ktempdir.cpp \
ksavefile.cpp \
kconfigbackend.cpp \
kconfigbase.cpp \
kconfigskeleton.cpp \
klockfile.cpp \
kstdaccel.cpp \
kcheckaccelerators.cpp \
kkeyserver_x11.cpp \
kkeynative_x11.cpp \
kaccelbase.cpp \
kaccel.cpp \
kaccelmanager.cpp \
kshortcut.cpp \
kshortcutmenu.cpp \
kshortcutlist.cpp \
kinstance.cpp \
kdeversion.cpp \
klocale.cpp \
kicontheme.cpp \
kiconloader.cpp \
kiconeffect.cpp \
kglobalsettings.cpp \
kckey.cpp \
kglobalaccel.cpp \
kglobalaccel_win.cpp \
kaccelaction.cpp \
kuniqueapplication.cpp \
ksycoca.cpp \
ksycocadict.cpp \
ksycocafactory.cpp \
kapplication.cpp \
kapplication_win.cpp \
kappdcopiface.cpp \
kprocctrl.cpp \
kdesktopfile.cpp \
kbufferedio.cpp \
netsupp_win32.cpp \
kasyncio.cpp \
ksockaddr.cpp \
kmdcodec.cpp \
kdcoppropertyproxy.cpp \
klibloader.cpp \
kprotocolinfo_kdecore.cpp \
../kio/kio/kprotocolinfo.cpp \
kprotocolinfofactory.cpp \
kmountpoint.cpp \
kmacroexpander.cpp \
kshell.cpp \
kclipboard.cpp \
kdebugdcopiface.cpp \
krandomsequence.cpp \
krfcdate.cpp \
kmultipledrag.cpp \
kipc.cpp \
kuser.cpp \
kaudioplayer.cpp \
kvmallocator.cpp \
kqiodevicegzip_p.cpp

#network/kresolver.cpp \
#network/kresolvermanager.cpp \
#network/kreverseresolver.cpp \
#network/ksocketaddress.cpp \
#network/ksocketbase.cpp \
#network/ksocketdevice.cpp \
#network/ksockssocketdevice.cpp

#kstartupinfo.cpp \
#todo: kextsock.cpp \
#todo: ksock.cpp \
#todo: ksocks.cpp \
#kpath.cpp \

# generated:
SOURCES += \
kappdcopiface_stub.cpp \
kappdcopiface_skel.cpp \
ksycoca_stub.cpp \
ksycoca_skel.cpp \
kdebugdcopiface_stub.cpp \
kdebugdcopiface_skel.cpp


exists( custom_kdecore.pro ) {
  include( custom_kdecore.pro )
}

HEADERS		=
