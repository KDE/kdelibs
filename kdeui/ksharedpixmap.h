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
 */

#ifndef __KSharedPixmap_h_Included__
#define __KSharedPixmap_h_Included__

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>

#ifndef Q_WS_QWS // FIXME: Implement for Qt Embedded
#include <kpixmap.h>

class KSharedPixmapPrivate;

/**
 * Shared pixmap client.
 *
 * A shared pixmap is a pixmap that resides on the X server, is referenced
 * by a global id string, and can be accessed by all X clients.
 *
 * This class is a client class to shared pixmaps in KDE. You can use it
 * to copy (a part of) a shared pixmap into. KSharedPixmap inherits KPixmap
 * for that purpose.
 *
 * The server part of shared pixmaps is not implemented here. 
 * That part is provided by KPixmapServer, in the source file:
 * kdebase/kdesktop/pixmapserver.cc.
 *
 * An example: copy from a shared pixmap:
 * <pre>
 *   KSharedPixmap *pm = new KSharedPixmap;
 *   connect(pm, SIGNAL(done(bool)), SLOT(slotDone(bool)));
 *   pm->loadFromShared("My Pixmap");
 * </pre>
 *
 * @author Geert Jansen <jansen@kde.org>
 * @version $Id$
 *
 */
class KSharedPixmap: 
    public QWidget,
    public KPixmap
{
    Q_OBJECT

public:

    /**
     * Construct an empty pixmap.
     */
    KSharedPixmap();

    /**
     * Destroys the pixmap.
     */
    ~KSharedPixmap();

    /**
     * Load from a shared pixmap reference. The signal done() is emitted
     * when the operation has finished.
     *
     * @param name The shared pixmap name.
     * @param rect If you pass a nonzero rectangle, a tile is generated which 
     * is able to fill up the specified rectangle completely. This is solely 
     * for optimisation: in some cases the tile will be much smaller than the 
     * original pixmap. It reflects KSharedPixmap's original use: sharing
     * of the desktop background to achieve pseudo transparency.
     * @return True if the shared pixmap exists and loading has started
     * successfully, false otherwise.
     */
    bool loadFromShared(QString name, QRect rect=QRect());

    /**
     * Check whether a shared pixmap is available.
     *
     * @param name The shared pixmap name.
     * @return True if the shared pixmap is available, false otherwise.
     */
    bool isAvailable(QString name);

signals:
    /** 
     * This signal is raised when a pixmap load operation has finished.
     *
     * @param success True if successfull, false otherwise.
     */
    void done(bool success);

protected:
    bool x11Event(XEvent *);
    
private:
    bool copy(QString id, QRect rect);
    void init();

    KSharedPixmapPrivate *d;
};
#else
// Qt Embedded... Let's simply assume KPixmap will do for now. Yes, I know that's broken.
#define KSharedPixmap KPixmap
#endif

#endif
