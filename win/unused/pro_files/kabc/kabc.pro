TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KABC_LIB
LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\kresources$$KDELIB_SUFFIX $$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\kio$$KDELIB_SUFFIX $$KDELIBDESTDIR\kvcard$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/kabc/vcard/include $(KDELIBS)/kabc/vcard/include/generated \
	$(KDELIBS)/kabc/vcardparser

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
  sortmode.cpp \
\
vcardparser/vcard.cpp vcardparser/vcardline.cpp vcardparser/vcardparser.cpp

HEADERS		=

generated:
SOURCES += \
addresseehelper_skel.cpp \
addresseehelper_stub.cpp

