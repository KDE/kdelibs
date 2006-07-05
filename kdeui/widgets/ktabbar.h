/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>

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

#ifndef KTABBAR_H
#define KTABBAR_H

#include <QTabBar>

#include <kdelibs_export.h>

/**
 * A QTabBar with extended features.
 */
class KDEUI_EXPORT KTabBar: public QTabBar
{
    Q_OBJECT

  public:
    /**
     * Creates a new tab bar.
     *
     * @param parent The parent widget.
     */
    explicit KTabBar( QWidget* parent = 0 );

    /**
     * Destroys the tab bar.
     */
    virtual ~KTabBar();

    /**
     * Sets the tab reordering enabled or disabled. If enabled,
     * the user can reorder the tabs by drag and drop the tab
     * headers.
     */
    void setTabReorderingEnabled( bool enable );

    /**
     * Returns whether tab reordering is enabled.
     */
    bool isTabReorderingEnabled() const;

    // KDE4 porting: disabled
    void setHoverCloseButton( bool );
    bool hoverCloseButton() const;

    // KDE4 porting: disabled
    void setHoverCloseButtonDelayed( bool );
    bool hoverCloseButtonDelayed() const;

    /**
     * Sets the 'activate previous tab on close' feature enabled
     * or disabled. If enabled, as soon as you close a tab, the
     * previously selected tab is activated again.
     */
    void setTabCloseActivatePrevious( bool );

    /**
     * Returns whether the 'activate previous tab on close' feature
     * is enabled.
     */
    bool tabCloseActivatePrevious() const;

    /**
     * Selects the tab which has a tab header at
     * given @param position.
     */
    int selectTab( const QPoint &position ) const;

  Q_SIGNALS:
    void contextMenu( int, const QPoint& );
    void mouseDoubleClick( int );
    void mouseMiddleClick( int );
    void initiateDrag( int );
    void testCanDecode( const QDragMoveEvent*, bool& );
    void receivedDropEvent( int, QDropEvent* );
    void moveTab( int, int );
    void closeRequest( int );
#ifndef QT_NO_WHEELEVENT
    void wheelDelta( int );
#endif

  protected:
    virtual void mouseDoubleClickEvent( QMouseEvent *event );
    virtual void mousePressEvent( QMouseEvent *event );
    virtual void mouseMoveEvent( QMouseEvent *event );
    virtual void mouseReleaseEvent( QMouseEvent *event );
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent( QWheelEvent *event );
#endif

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragMoveEvent( QDragMoveEvent *event );
    virtual void dropEvent( QDropEvent *event );

  protected Q_SLOTS:
    //virtual void closeButtonClicked();
    //virtual void enableCloseButton();
    virtual void activateDragSwitchTab();

  protected:
    virtual void tabLayoutChange();

  private:
    class Private;
    Private* const d;
};

#endif
