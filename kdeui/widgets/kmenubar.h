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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KMENUBAR_H
#define KMENUBAR_H

#include <kdeui_export.h>

#include <QtGui/QMenuBar>

/**
 * %KDE Style-able menubar.
 *
 * This is required since QMenuBar is currently not handled by
 * QStyle.
 *
 * @author Daniel "Mosfet" Duley.
 */

class KDEUI_EXPORT KMenuBar : public QMenuBar
{
    Q_OBJECT
    Q_PROPERTY( bool topLevelMenu READ isTopLevelMenu WRITE setTopLevelMenu )

public:

    explicit KMenuBar (QWidget *parent=0);
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

    // TT are overloading virtuals :(
    virtual void setGeometry( const QRect &r );
    virtual void setGeometry( int x, int y, int w, int h );
    virtual void resize( int w, int h );
    void resize( const QSize& s );

    virtual void setFrameStyle( int );
    virtual void setLineWidth( int );
    virtual void setMargin( int );
    virtual QSize sizeHint() const;
protected Q_SLOTS:
    void slotReadConfig();
protected:
    virtual void resizeEvent( QResizeEvent* );
    virtual bool eventFilter(QObject *, QEvent *);
#ifdef Q_WS_X11
    virtual bool x11Event( XEvent* );
#endif
    virtual void closeEvent( QCloseEvent* );
    virtual void paintEvent( QPaintEvent* );
private Q_SLOTS:
    void updateFallbackSize();
    void selectionTimeout();
private:
    void setTopLevelMenuInternal(bool top_level);
    void updateMenuBarSize();
    void checkSize( int& w, int& h );
private:
    class KMenuBarPrivate;
    KMenuBarPrivate * const d;
};

#endif
