TEMPLATE	= lib
CONFIG += kde3lib #this is a kde module library

include( $(QKW)/kwcommon.pro )

# needed to export library classes:
DEFINES += MAKE_KATEPART_LIB

TARGET		= katepart$$KDELIBDEBUG

win32:LIBS += $$QKWLIB/katepartinterfaces$$KDELIB_SUFFIX \
$$QKWLIB/ktexteditor$$KDELIB_SUFFIX $$QKWLIB/kdecore$$KDELIB_SUFFIX $$QKWLIB/kdeui$$KDELIB_SUFFIX \
$$QKWLIB/kutils$$KDELIB_SUFFIX $$QKWLIB/kparts$$KDELIB_SUFFIX \
$$QKWLIB/dcop$$KDELIB_SUFFIX $$QKWLIB/kio$$KDELIB_SUFFIX

#$$QKWLIB/kdeprint$$KDELIB_SUFFIX

INCLUDEPATH += $(QKW)/kdelibs/interfaces $(QKW)/kdelibs/interfaces/kregexpeditor \
	$(QKW)/kdelibs/kutils $(QKW)/kdelibs/kdeprint

system( bash kmoc )
system( bash kdcopidl )

SOURCES = \
  katesearch.cpp katebuffer.cpp katecmds.cpp \
  kateundo.cpp katecursor.cpp katedialogs.cpp katedocument.cpp \
  katefactory.cpp katehighlight.cpp katesyntaxdocument.cpp \
  katetextline.cpp kateview.cpp kateconfig.cpp kateviewhelpers.cpp \
  katecodecompletion.cpp katedocumenthelpers.cpp \
  katecodefoldinghelpers.cpp kateviewinternal.cpp katebookmarks.cpp \
  katefont.cpp katelinerange.cpp katesupercursor.cpp \
  katearbitraryhighlight.cpp katerenderer.cpp kateattribute.cpp \
  kateautoindent.cpp katefiletype.cpp kateschema.cpp \
  katetemplatehandler.cpp \
  kateprinter.cpp

HEADERS =

# generated:
SOURCES += \
katedocument_skel.cpp
