/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE libraries.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
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

#include <kpixmap.h>

/**
 * Implements shared X pixmaps.
 *
 * A shared pixmap is a pixmap that resides only once in the X server.
 * Every shared pixmap is referenced by an ID string, which has to be
 * globally unique (unique among all X clients).
 *
 * You make a pixmap accessible to other X client by publishing it, using
 * publish().
 *
 * Normally, if an X client exits, all its resources (including pixmaps) 
 * are deleted. This might give undesireable effects when other clients are 
 * using that pixmap. If you want to keep the  pixmaps around, use 
 * setKeepResources(). This will keep _all_ this client's X recources in 
 * the server until it resets so don't use without thought.
 *
 * Even when you unpublish() a shared pixmap first, you should take special 
 * caution when deleting it. Clients might still have the handle around and 
 * try to access it. You're on you own here, KSharedPixmap doesn't provide
 * you with some kind of synchronisation mechanism.
 *
 *  A simple example: share a pixmap.
 * <pre>
 *   KSharedPixmap pm;
 *   pm.load("file.xpm");
 *   pm.publish("pixmap_id");
 * </pre>
 *
 * Get a shared pixmap:
 * <pre>
 *   KSharedPixmap pm("pixmap_id");
 *   if (!pm.isNull())
 *	use_pixmap();
 * </pre>
 *
 * @author Geert Jansen <g.t.jansen@stud.tue.nl>
 * @version $Id$
 *
 */
class KSharedPixmap: public KPixmap
{
public:

    /**
     * Construct an empty pixmap.
     */
    KSharedPixmap();

    /**
     * Constructs a pixmap from a shared pixmap reference. This pixmap will
     * be a copy of the shared pixmap. The copy is done using an efficient
     * bit block transfer.
     *
     * @param id The shared pixmap id string.
     * @param rect The region of the pixmap you want.
     */
    KSharedPixmap(QString id, QRect rect=QRect());
    
    /**
     * Construct from QPixmap.
     */
    KSharedPixmap(const QPixmap &pm);

    /**
     * Destroys and unpublishes the pixmap.
     */
    ~KSharedPixmap();

    /**
     * Set the X property to use for storing the pixmap info. You
     * can use a non default value if you want a "privately shared" 
     * pixmap. The default is "KDE_SHARED_PIXMAPS".
     *
     * @param prop The name of the X property to use.
     */
    void setProp(QString prop);

    /**
     * Publish the pixmap.
     *
     * Publishing a pixmap makes it available to other X clients.
     * You can publish it under multiple id's.
     *
     * @param id An identification string for this pixmap. 
     * @param overwrite If set to true, overwrite pixmaps with the same
     * name.
     * @return True if successfull, False otherwise.
     *
     */
    bool publish(QString id, bool overwrite=true);

    /**
     * Unpublish the pixmap.
     *
     * @param id A published id string. Use the default value to
     * unpublish all references.
     * @return True if successfull, False otherwise.
     */
    bool unpublish(QString id=QString::null);

    /**
     * Load from a shared pixmap. 
     *
     * @param id The pixmap ID string.
     * @param rect The rectangle to copy. Use the default argument
     * to copy the entire pixmap.
     * @return True if successfull, False otherwise.
     */
    bool loadFromShared(QString id, QRect rect=QRect());

    /**
     * Set the X11 closedown mode.
     *
     * This call uses XSetCloseDownMode() to set the closedown mode of the X
     * server. If set to true, all resources of the this client are kept in 
     * the X server until it resets (normally they are deleted). Note that
     * this is a global option!
     *
     * IMPORTANT: If you want to use this, make sure the QPixmap destructor
     * doesn't get called, i.e. use "new" and don't "delete". Otherwise,
     * said destructor will free the pixmap.
     *
     * @param keep Set to true to keep resources, false to delete them when
     * the display is closed.
     */
    void setKeepResources(bool keep=true);

    /**
     * Query wether a shared pixmap exists.
     *
     * @param id The pixmap id string.
     * @param prop The X property to use.
     * @return The size of the pixmap if it's available, a zero size
     * otherwise.
     */
    static QSize query(QString id, QString prop=QString::null);

    /**
     * List all available shared pixmaps.
     *
     * @param prop The X property to use.
     * @return A QStringList containing all the available pixmaps id's.
     */
    static QStringList list(QString prop=QString::null);

private:
    bool copy(QString id, QRect rect);
    void init();

    QString mProp;
    QStringList mRefs;
};


#endif
