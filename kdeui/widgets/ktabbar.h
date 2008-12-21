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

#include <kdeui_export.h>

#include <QtGui/QTabBar>

/**
 * A QTabBar with extended features.
 */
class KDEUI_EXPORT KTabBar: public QTabBar //krazy:exclude=qclasses
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

    /**
     * If enabled, a close button is shown above the tab icon.
     * The signal KTabBar::closeRequest() is emitted, if the
     * close button has been clicked. Note that the tab must have
     * an icon to use this feature.
     *
     * @deprecated Use KTabBar::setCloseButtonEnabled() instead.
     */
    KDE_DEPRECATED void setHoverCloseButton( bool );

    /** @deprecated Use KTabBar::isCloseButtonEnabled() instead. */
    KDE_DEPRECATED bool hoverCloseButton() const;

    /**
     * If enabled, the close button cannot get clicked until a
     * minor delay has been passed. This prevents that user
     * accidentally closes a tab.
     *
     * @deprecated Use KTabBar::setCloseButtonEnabled() instead.
     */
    KDE_DEPRECATED void setHoverCloseButtonDelayed( bool );

    /** @deprecated Use KTabBar::isCloseButtonEnabled() instead. */
    KDE_DEPRECATED bool hoverCloseButtonDelayed() const;

    /**
     * If enabled, a close button is available for each tab. The
     * signal KTabBar::closeRequest() is emitted, if the close button
     * has been clicked.
     *
     * @since 4.1
     */
    void setCloseButtonEnabled( bool );

    /**
     * Returns true if the close button is shown on tabs.
     *
     * @since 4.1
     */
    bool isCloseButtonEnabled() const;

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
     *
     * @param position the coordinates of the tab
     */
    int selectTab( const QPoint &position ) const;

  Q_SIGNALS:
    /** Emitted when the user right-clicks a tab. */
    void contextMenu( int, const QPoint& );
    /** Emitted when the user right-clicks the empty area on the tab bar. */
    void emptyAreaContextMenu( const QPoint& );
    /** @deprecated use tabDoubleClicked(int) and newTabRequest() instead. */
    QT_MOC_COMPAT void mouseDoubleClick( int );
    /** Emitted when a tab has been double-clicked. */
    void tabDoubleClicked( int );
    /** Emitted when the user double-clicks the empty area on the tab bar. */
    void newTabRequest();
    /** Emitted when a tab has been middle-clicked. */
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

    virtual void paintEvent( QPaintEvent *event );
    virtual void leaveEvent( QEvent *event );
    virtual QSize tabSizeHint( int index ) const;

  protected Q_SLOTS:
    /** @deprecated */
    QT_MOC_COMPAT void closeButtonClicked();
    /** @deprecated */
    QT_MOC_COMPAT void enableCloseButton();
    virtual void activateDragSwitchTab();

  protected:
    virtual void tabLayoutChange();

  private:
    QPoint closeButtonPos( int tabIndex ) const;
    QRect closeButtonRect( int tabIndex ) const;

  private:
    class Private;
    Private* const d;
};

#endif
