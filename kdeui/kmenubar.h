/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998, 1999, 2000  Sven Radej (radej@kde.org)
    Copyright (C) 1997, 1998, 1999, 2000 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999, 2000 Daniel "Mosfet" Duley (mosfet@kde.org)


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

//$Id$

#ifndef _KMENUBAR_H
#define _KMENUBAR_H

#include <qmenubar.h>

/**
 * KDE Style-able menubar.
 *
 * This is required since @ref QMenuBar is currently not handled by
 * @ref QStyle.
 *
 * @author Daniel "Mosfet" Duley.
 * @version $Id$
*/

class KMenuBar : public QMenuBar
{
    Q_OBJECT

public:

    KMenuBar (QWidget *parent=0, const char *name=0);
    ~KMenuBar ();

    /**
     * This controls whether or not this menubar will be a top-level
     * bar similar to the way Macintosh handles menubars.  This
     * overrides any global config settings.
     *
     * Keep in mind that it is probably a really bad idea to use this
     * unless you really know what you're doing.  A feature like a
     * top-level menubar is one that should really be shared by all
     * applications.  If your app is the only one with a top-level
     * bar, then things might look very... odd.
     *
     * This is included only for those people that @p do know that
     * they need to use it.
     *
     * @param top_level If set to true, then this menubar will be a
     *                  top-level menu
     */
    void setTopLevelMenu(bool top_level = true);

    /**
     * Is our menubar a top-level (Macintosh style) menubar?
     *
     * @return True if it is top-level.
     */
    bool isTopLevelMenu() const;

    virtual void setGeometry( const QRect &r ) { QMenuBar::setGeometry( r ); }
    virtual void        setGeometry( int x, int y, int w, int h );

    virtual void show();
protected slots:
    void slotReadConfig();

protected:
    void showEvent( QShowEvent* );
    void resizeEvent( QResizeEvent* );
    bool eventFilter(QObject *, QEvent *);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    void setTopLevelMenuInternal(bool top_level = true);
    void updateKMenubarSize();
    class KMenuBarPrivate;
    KMenuBarPrivate *d;
};

#endif
