/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 *
 * Shared pixmaps for KDE.
 *
 * $Id$
 *
 * 1 Oct 99 Geert Jansen:      Initial implementation.
 */

#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qwindowdefs.h>

#include <krootprop.h>
#include <ksharedpixmap.h>

#include <X11/Xlib.h>

/**
 * A utility class to read/store pixmap descriptions.
 * Shoud not be exported.
 */
class KSharedPixmapData
{
public:
    KSharedPixmapData(QString desc);
    KSharedPixmapData(QPixmap *pm);

    int handle() { return mHandle; }
    int height() { return mHeight; }
    int width() { return mWidth; }
    bool ok() { return mbOK; }

    QString desc() { return mDesc; }

private:
    bool mbOK;
    int mHandle, mHeight, mWidth;
    QString mDesc;
};

KSharedPixmapData::KSharedPixmapData(QPixmap *pm)
{
    mHandle = pm->handle();
    mWidth = pm->width();
    mHeight = pm->height();
    mDesc.sprintf("%d,%d,%d", mHandle, mWidth, mHeight);
    mbOK = true;
}

KSharedPixmapData::KSharedPixmapData(QString desc)
{    
    mbOK = false;

    int c1, c2;
    c1 = desc.find(',');
    if (c1 == -1)
	return;
    c2 = desc.find(',', c1+1);
    if (c2 == -1)
	return;

    bool ok;
    mHandle = desc.left(c1).toInt(&ok);
    if (!ok)
	return;
    mWidth = desc.mid(c1+1, c2-c1-1).toInt(&ok);
    if (!ok)
	return;
    mHeight = desc.mid(c2+1).toInt(&ok);
    if (!ok)
	return;

    mbOK = true;
}


/**
 * KSharedPixmap
 */

KSharedPixmap::KSharedPixmap()
{
    init();
}


KSharedPixmap::KSharedPixmap(QString id, QRect rect, QString prop)
{
    mProp = prop;
    init();
    if (!id.isEmpty())
	copy(id, rect);
}
    

KSharedPixmap::KSharedPixmap(const QPixmap &pm)
{
    init();
    *((QPixmap *) this) = pm;
}


KSharedPixmap::~KSharedPixmap()
{
    if (mRefs.isEmpty())
	return;
    unpublish();
}


/* private */
void KSharedPixmap::init()
{
    if (mProp.isEmpty())
	mProp = "KDE_SHARED_PIXMAPS";
}


void KSharedPixmap::setProp(QString prop)
{
    mProp = prop;
}


bool KSharedPixmap::publish(QString id, bool overwrite)
{
    if (id.isEmpty() || isNull())
	return false;

    // Keep a list of published id's
    if (mRefs.contains(id))
	return true;
    mRefs.append(id);

    KRootProp prop(mProp);
    QString val = prop.readEntry(id);
    if (!val.isNull() && !overwrite)
	return false;
    KSharedPixmapData pmd(this);
    prop.writeEntry(id, pmd.desc());

    return true;
}


bool KSharedPixmap::unpublish(QString id)
{
    KRootProp prop(mProp);
    if (id.isEmpty()) {
	QStringList::Iterator it;
	bool ok = true;
	for (it=mRefs.begin(); it!=mRefs.end(); it++)
	    if (prop.removeEntry(*it).isNull())
		ok = false;
	return ok;
    } 
    if (mRefs.contains(id))
	return !prop.removeEntry(id).isNull();
    return false;
}


bool KSharedPixmap::loadFromShared(QString id, QRect rect)
{
    if (!id.isEmpty())
	return copy(id, rect);
    return false;
}


/* private */
bool KSharedPixmap::copy(QString id, QRect rect)
{
    KRootProp prop(mProp);
    QString desc = prop.readEntry(id);
    if (desc.isEmpty())
	return false;
    KSharedPixmapData pmd(desc);
    if (!pmd.ok())
	return false;

    if (rect.isEmpty()) {
	XCopyArea(qt_xdisplay(), pmd.handle(), handle(), qt_xget_temp_gc(),
		0, 0, pmd.width(), pmd.height(), 0, 0);
	return true;
    }

    // Do some more processing here: Generate a tile that can be used as a
    // background tile for the rectangle "rect".
	
    int sw = pmd.width(), sh = pmd.height();
    int w = rect.width(), h = rect.height();
    int tw = QMIN(sw, w), th = QMIN(sh, h);
    int xa = rect.x() % sw, ya = rect.y() % sh;
    int t1w = QMIN(sw-xa,tw), t1h = QMIN(sh-ya,th);

    detach(); 
    resize(tw, th);

    XCopyArea(qt_xdisplay(), pmd.handle(), handle(), qt_xget_temp_gc(),
	    xa, ya, t1w, t1h, 0, 0);
    XCopyArea(qt_xdisplay(), pmd.handle(), handle(), qt_xget_temp_gc(),
	    0, ya, tw-t1w, t1h, t1w, 0);
    XCopyArea(qt_xdisplay(), pmd.handle(), handle(), qt_xget_temp_gc(),
	    xa, 0, t1w, th-t1h, 0, t1h);
    XCopyArea(qt_xdisplay(), pmd.handle(), handle(), qt_xget_temp_gc(),
	    0, 0, tw-t1w, th-t1h, t1w, t1h);
    
    return true;
}


void KSharedPixmap::setKeepResources(bool keep)
{
    XSetCloseDownMode(qt_xdisplay(), keep ? RetainTemporary : DestroyAll);
}


/* static */
QSize KSharedPixmap::query(QString id, QString property)
{
    if (id.isEmpty())
	return QSize();
    if (property.isEmpty())
	property = "KDE_SHARED_PIXMAPS";

    KRootProp prop(property);
    QString data = prop.readEntry(id, QString());
    if (data.isEmpty())
	return QSize();

    KSharedPixmapData pmd(data);
    if (!pmd.ok())
	return QSize();

    return QSize(pmd.width(), pmd.height());
}


/* static */
QStringList KSharedPixmap::list(QString property)
{
    if (property.isEmpty())
	property = "KDE_SHARED_PIXMAPS";

    KRootProp prop(property);
    return prop.listEntries();
}
