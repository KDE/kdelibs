/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * Shared pixmap client for KDE.
 */
#include "config.h"

#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qwindowdefs.h>
#include <qwidget.h>

#include <kapp.h>
#include <krootprop.h>
#include <ksharedpixmap.h>
#include <kdebug.h>

#include <X11/Xlib.h>

// Make sure to include all this X-based shit before we clean up the mess.
// Needed for --enable-final. Not needed by this file itself!
#include <X11/Xutil.h>
#ifdef HAVE_MITSHM
#include <X11/extensions/XShm.h>
#endif

#include <netwm.h>

// Clean up the mess

#undef Bool
#undef Above
#undef Below
#undef KeyPress
#undef FocusOut

/**
 * KSharedPixmap
 */

KSharedPixmap::KSharedPixmap()
    : QWidget(0L, "shpixmap comm window")
{
    init();
}


KSharedPixmap::~KSharedPixmap()
{
}


void KSharedPixmap::init()
{
    pixmap = XInternAtom(qt_xdisplay(), "PIXMAP", false);
    QCString atom;
    atom.sprintf("target prop for window %x", winId());
    target = XInternAtom(qt_xdisplay(), atom.data(), false);
    m_Selection = None;
}


bool KSharedPixmap::isAvailable(QString name)
{
    QString str = QString("KDESHPIXMAP:%1").arg(name);
    Atom sel = XInternAtom(qt_xdisplay(), str.latin1(), true);
    if (sel == None)
	return false;
    return XGetSelectionOwner(qt_xdisplay(), sel) != None;
}


bool KSharedPixmap::loadFromShared(QString name, QRect rect)
{
    if (m_Selection != None)
	// already active
	return false;

    m_Rect = rect;
    QPixmap::resize(0, 0); // invalidate

    QString str = QString("KDESHPIXMAP:%1").arg(name);
    m_Selection = XInternAtom(qt_xdisplay(), str.latin1(), true);
    if (m_Selection == None)
	return false;
    if (XGetSelectionOwner(qt_xdisplay(), m_Selection) == None) 
    {
	m_Selection = None;
	return false;
    }

    XConvertSelection(qt_xdisplay(), m_Selection, pixmap, target,
	    winId(), CurrentTime);
    return true;
}


bool KSharedPixmap::x11Event(XEvent *event)
{
    if (event->type != SelectionNotify)
	return false;
	
    XSelectionEvent *ev = &event->xselection;
    if (ev->selection != m_Selection)
	return false;

    if ((ev->target != pixmap) || (ev->property == None)) 
    {
	kdWarning(270) << k_funcinfo << "illegal selection notify event.\n";
	m_Selection = None;
	emit done(false);
	return true;
    }

    // Read pixmap handle from ev->property

    int dummy, format;
    unsigned long nitems, ldummy;
    Drawable *pixmap_id;
    Atom type;

    XGetWindowProperty(qt_xdisplay(), winId(), ev->property, 0, 1, false,
	    pixmap, &type, &format, &nitems, &ldummy,
	    (unsigned char **) &pixmap_id);

    if (nitems != 1) 
    {
	kdWarning(270) << k_funcinfo << "could not read property, nitems = " << nitems << "\n";
	emit done(false);
	return true;
    }

    Window root;
    unsigned int width, height, udummy;
    XGetGeometry(qt_xdisplay(), *pixmap_id, &root, &dummy, &dummy, &width,
	    &height, &udummy, &udummy);

    if (m_Rect.isEmpty()) 
    {
	QPixmap::resize(width, height);
	XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
		0, 0, width, height, 0, 0);
	XDeleteProperty(qt_xdisplay(), winId(), ev->property);
	m_Selection = None;
	emit done(true);
	return true;
    }

    // Do some more processing here: Generate a tile that can be used as a
    // background tile for the rectangle "rect".
	
    unsigned w = m_Rect.width(), h = m_Rect.height();
    unsigned tw = QMIN(width, w), th = QMIN(height, h);
    unsigned xa = m_Rect.x() % width, ya = m_Rect.y() % height;
    unsigned t1w = QMIN(width-xa,tw), t1h = QMIN(height-ya,th);

    QPixmap::resize(tw, th);

    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    xa, ya, t1w, t1h, 0, 0);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    0, ya, tw-t1w, t1h, t1w, 0);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    xa, 0, t1w, th-t1h, 0, t1h);
    XCopyArea(qt_xdisplay(), *pixmap_id, ((KPixmap*)this)->handle(), qt_xget_temp_gc(),
	    0, 0, tw-t1w, th-t1h, t1w, t1h);

    m_Selection = None;
    XDeleteProperty(qt_xdisplay(), winId(), ev->property);
    emit done(true);
    return true;
}


#include "ksharedpixmap.moc"
