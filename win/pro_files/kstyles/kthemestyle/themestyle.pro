include( ../common.pro )

LIBS += $$KDELIBDESTDIR\kdecore$$KDELIB_SUFFIX

TARGET		= kthemestyle$$KDEBUG

SOURCES = \
kthemestyle.cpp \
kthemebase.cpp \
kstyledirs.cpp

system( moc kthemestyle.h -o moc/kthemestyle.moc )
system( moc kthemebase.h -o moc/kthemebase.moc )


