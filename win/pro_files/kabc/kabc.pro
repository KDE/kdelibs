TEMPLATE	= lib

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KABC_LIB
win32:LIBS += $$QKWLIB\kdecore$$KDELIB_SUFFIX $$QKWLIB\kdeui$$KDELIB_SUFFIX \
	$$QKWLIB\kresources$$KDELIB_SUFFIX $$QKWLIB\dcop$$KDELIB_SUFFIX $$QKWLIB\kio$$KDELIB_SUFFIX \
	$$QKWLIB\kvcard$$KDELIB_SUFFIX

INCLUDEPATH += $(QKW)/kdelibs/kabc/vcard/include $(QKW)/kdelibs/kabc/vcard/include/generated \
	$(QKW)/kdelibs/kabc/vcardparser

system( bash kmoc )
system( bash kdcopidl )
system( cd scripts && perl.exe makeaddressee )

TARGET		= kabc$$KDEBUG

SOURCES = \
  address.cpp addressbook.cpp addressee.cpp addresseedialog.cpp agent.cpp \
  distributionlist.cpp distributionlistdialog.cpp distributionlisteditor.cpp \
  errorhandler.cpp field.cpp formatfactory.cpp geo.cpp key.cpp \
  phonenumber.cpp picture.cpp plugin.cpp resource.cpp \
  resourceselectdialog.cpp secrecy.cpp sound.cpp stdaddressbook.cpp \
  timezone.cpp vcard21parser.cpp vcardconverter.cpp vcardformat.cpp \
  vcardformatimpl.cpp vcardformatplugin.cpp ldifconverter.cpp addresslineedit.cpp \
  ldapclient.cpp addresseelist.cpp vcardtool.cpp addresseehelper.cpp \
  lock.cpp locknull.cpp ldif.cpp ldapurl.cpp ldapconfigwidget.cpp \
\
vcardparser/vcard.cpp vcardparser/vcardline.cpp vcardparser/vcardparser.cpp

HEADERS		=

generated:
SOURCES += \
addresseehelper_skel.cpp \
addresseehelper_stub.cpp

