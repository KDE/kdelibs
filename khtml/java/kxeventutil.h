#ifndef KXEVENTUTIL_H
#define KXEVENTUTIL_H

#include <X11/X.h>
#include <X11/Xlib.h>
#include <qstring.h>

class KXEventUtil {
    public:
        static QString getXEventName(XEvent *e);
        static QString getXAnyEventInfo(XEvent *xevent);
        static QString getXButtonEventInfo(XEvent *xevent);
        static QString getXKeyEventInfo(XEvent *xevent);
        static QString getXMotionEventInfo(XEvent *xevent);
        static QString getXCrossingEventInfo(XEvent *xevent);
        static QString getXFocusChangeEventInfo(XEvent *xevent);
        static QString getXExposeEventInfo(XEvent *xevent);
        static QString getXGraphicsExposeEventInfo(XEvent *xevent);
        static QString getXNoExposeEventInfo(XEvent *xevent);
        static QString getXCreateWindowEventInfo(XEvent *xevent);
        static QString getXDestroyWindowEventInfo(XEvent *xevent);
        static QString getXMapEventInfo(XEvent *xevent);
        static QString getXMappingEventInfo(XEvent *xevent);
        static QString getXReparentEventInfo(XEvent *xevent);
        static QString getXUnmapEventInfo(XEvent *xevent);
        static QString getXConfigureEventInfo(XEvent *xevent);
        static QString getXConfigureRequestEventInfo(XEvent *xevent);
        static QString getX11EventInfo( XEvent* e );
};

#endif
