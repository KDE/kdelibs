TEMPLATE	= lib
CONFIG += kde3lib #this is a kde module library

include( $(KDELIBS)/win/common.pro )

# needed to export library classes:
DEFINES += MAKE_KATEPART_LIB

TARGET		= katepart$$KDELIBDEBUG

LIBS += $$KDELIBDESTDIR/katepartinterfaces$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/ktexteditor$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/kutils$$KDELIB_SUFFIX $$KDELIBDESTDIR/kparts$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR/dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR/kio$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/interfaces $(KDELIBS)/interfaces/kregexpeditor \
	$(KDELIBS)/kutils $(KDELIBS)/kdeprint

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
  kateindentscriptabstracts.cpp \
  kateautoindent.cpp katefiletype.cpp kateschema.cpp \
  katetemplatehandler.cpp \
  kateprinter.cpp katespell.cpp

#todo:	katejscript.cpp

HEADERS =

# generated:
SOURCES += \
katedocument_skel.cpp
