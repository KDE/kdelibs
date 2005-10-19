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
#include <qlist.h>
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
    friend class RMB;
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

    /**
     * @since 3.2
     */
    bool isReadOnly() const;

    /**
     * @since 3.2
     */
    void setReadOnly(bool);

    /**
     * @since 3.2
     */
    QString parentAddress();

signals:
    /**
     * @since 3.2
     */
    void aboutToShowContextMenu( const KBookmark &, QMenu * );
    /**
     * @since 3.4
     */
    void openBookmark( const QString& url, Qt::ButtonState state );

public slots:
    void clear();

    void slotBookmarksChanged( const QString & );
    void slotBookmarkSelected();

    /**
     * @since 3.4
     */
    void slotBookmarkSelected( KAction::ActivationReason reason, Qt::ButtonState state );

    /// @since 3.2
    void slotRMBActionRemove( int );
    /// @since 3.2
    void slotRMBActionInsert( int );
    /// @since 3.2
    void slotRMBActionCopyLocation( int );
    /// @since 3.2
    void slotRMBActionEditAt( int );
    /// @since 3.2
    void slotRMBActionProperties( int );

protected:
    void fillBookmarkBar( KBookmarkGroup & parent );
    virtual bool eventFilter( QObject *o, QEvent *e );

private:
    KBookmarkGroup getToolbar();

    KBookmarkOwner *m_pOwner;
    QPointer<KToolBar> m_toolBar;
    KActionCollection *m_actionCollection;
    KBookmarkManager *m_pManager;
    QList<KBookmarkMenu *> m_lstSubMenus;

    KBookmarkBarPrivate * const d;
};

#endif // KBOOKMARKBAR_H
