//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 1999 Kurt Granroth <granroth@kde.org>

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
#ifndef KBOOKMARKBAR_H
#define KBOOKMARKBAR_H

#include <qobject.h>
#include <qpointer.h>
#include <QList>
#include <kbookmark.h>
#include <kaction.h>

class KToolBar;
class KBookmarkMenu;
class KBookmarkOwner;
class KActionCollection;
class KAction;
class QMenu;
class KBookmarkBarPrivate;

/**
 * This class provides a bookmark toolbar.  Using this class is nearly
 * identical to using KBookmarkMenu so follow the directions
 * there.
 */
class KIO_EXPORT KBookmarkBar : public QObject
{
    Q_OBJECT
public:
    /**
     * Fills a bookmark toolbar
     *
     * @param manager the bookmark manager
     * @param owner implementation of the KBookmarkOwner interface (callbacks)
     * @param toolBar toolbar to fill
     *
     * The KActionCollection pointer argument is now obsolete.
     *
     * @param parent the parent widget for the bookmark toolbar
     */
    KBookmarkBar( KBookmarkManager* manager,
                  KBookmarkOwner *owner, KToolBar *toolBar,
                  KActionCollection *,
                  QObject *parent = 0);

    virtual ~KBookmarkBar();

    bool isReadOnly() const;

    void setReadOnly(bool);

    QString parentAddress();

Q_SIGNALS:
    void aboutToShowContextMenu( const KBookmark &, QMenu * );
    void openBookmark( const QString& url, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers );

public Q_SLOTS:
    void clear();
    void contextMenu( const QPoint & );

    void slotBookmarksChanged( const QString & );
    void slotBookmarkSelected();

protected:
    void fillBookmarkBar( KBookmarkGroup & parent );
    virtual bool eventFilter( QObject *o, QEvent *e );

private:
    KBookmarkGroup getToolbar();
    void removeTempSep();
    bool handleToolbarDragMoveEvent(const QPoint& pos, const QList<KAction *>& actions, QString text);

    KBookmarkOwner *m_pOwner;
    QPointer<KToolBar> m_toolBar;
    KActionCollection *m_actionCollection;
    KBookmarkManager *m_pManager;
    QList<KBookmarkMenu *> m_lstSubMenus;
    QAction* m_toolBarSeparator;

    /**
     * If you want to extend KBookmarkBar without breaking binary compatibility, 
     * put additional members into this class. When you can break compatibility, move 
     * members into KBookmarBar and empty this private class.
     */
    KBookmarkBarPrivate * const d;
};

#endif // KBOOKMARKBAR_H
