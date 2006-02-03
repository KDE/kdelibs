TEMPLATE	= lib
DEFINES += MAKE_KTEXTEDITOR_LIB

include( $(KDELIBS)/win/common.pro )

LIBS += $$KDELIBDESTDIR/kdecore$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR/kdeui$$KDELIB_SUFFIX $$KDELIBDESTDIR/kdefx$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\kparts$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\kio$$KDELIB_SUFFIX $$KDELIBDESTDIR\kabc$$KDELIB_SUFFIX \
    $$KDELIBDESTDIR\kdewin32$$KDELIB_SUFFIX

INCLUDEPATH += $(KDELIBS)/interfaces $(KDELIBS)/interfaces/ktexteditor \
 $(KDELIBS)/kabc

TARGET		= ktexteditor$$KDEBUG

system( bash kmoc )
system( bash kdcopidl )

SOURCES = \
    ktexteditor.cpp \
    editinterface.cpp editinterfaceext.cpp \
    clipboardinterface.cpp  selectioninterface.cpp searchinterface.cpp \
    codecompletioninterface.cpp wordwrapinterface.cpp blockselectioninterface.cpp \
    configinterface.cpp cursorinterface.cpp  dynwordwrapinterface.cpp \
    printinterface.cpp highlightinginterface.cpp markinterface.cpp \
    popupmenuinterface.cpp undointerface.cpp viewcursorinterface.cpp \
    editdcopinterface.cpp  clipboarddcopinterface.cpp \
    selectiondcopinterface.cpp  \
    searchdcopinterface.cpp  markinterfaceextension.cpp \
    configinterfaceextension.cpp encodinginterface.cpp sessionconfiginterface.cpp \
    viewstatusmsginterface.cpp  editorchooser.cpp \
    blockselectiondcopinterface.cpp documentinfo.cpp documentdcopinfo.cpp\
    encodingdcopinterface.cpp \
    printdcopinterface.cpp  \
    undodcopinterface.cpp viewcursordcopinterface.cpp \
    viewstatusmsgdcopinterface.cpp \
    selectioninterfaceext.cpp \
    texthintinterface.cpp variableinterface.cpp \
    templateinterface.cpp

# generated:
SOURCES += \
selectionextdcopinterface_skel.cpp \
viewstatusmsgdcopinterface_skel.cpp \
viewcursordcopinterface_skel.cpp \
undodcopinterface_skel.cpp \
printdcopinterface_skel.cpp \
encodingdcopinterface_skel.cpp \
documentdcopinfo_skel.cpp \
blockselectiondcopinterface_skel.cpp \
searchdcopinterface_skel.cpp \
selectiondcopinterface_skel.cpp \
clipboarddcopinterface_skel.cpp \
editdcopinterface_skel.cpp \
\
selectionextdcopinterface_stub.cpp \
viewstatusmsgdcopinterface_stub.cpp \
viewcursordcopinterface_stub.cpp \
undodcopinterface_stub.cpp \
printdcopinterface_stub.cpp \
encodingdcopinterface_stub.cpp \
documentdcopinfo_stub.cpp \
blockselectiondcopinterface_stub.cpp \
searchdcopinterface_stub.cpp \
selectiondcopinterface_stub.cpp \
clipboarddcopinterface_stub.cpp \
editdcopinterface_stub.cpp


HEADERS		= 

INTERFACES = \
editorchooser_ui.ui
