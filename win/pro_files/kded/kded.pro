TEMPLATE	= app

include( $(QKW)/kwcommon.pro )

win32:LIBS += $$QKWLIB\dcop$$KDELIB_SUFFIX $$QKWLIB\kio$$KDELIB_SUFFIX

# icon
win32:LIBS+=$(QKW)/files/resources/kbuildsycoca.res

QMAKE_CXXFLAGS += /I$(QKW)/kdelibs/kded

#system( moc kbuildsycoca.h -o moc/kbuildsycoca.moc )
#system( moc vfolder_menu.h -o moc/vfolder_menu.moc )

system( bash kmoc )

#DEFINES += i18n=KLocale::translateQt

TARGET		= kbuildsycoca

SOURCES = \
kbuildsycoca.cpp \
kbuildimageiofactory.cpp \
kbuildprotocolinfofactory.cpp \
kbuildservicefactory.cpp \
kbuildservicegroupfactory.cpp \
kbuildservicetypefactory.cpp \
kctimefactory.cpp \
vfolder_menu.cpp

HEADERS		=

TRANSLATIONS    = kbuildsycoca_pl.ts

