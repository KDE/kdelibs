/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFILEBOOKMARKHANDLER_H
#define KFILEBOOKMARKHANDLER_H

#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>

class QTextStream;
class KMenu;


class KIO_EXPORT KFileBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KFileBookmarkHandler( KFileDialog *dialog );
    ~KFileBookmarkHandler();

    QMenu * popupMenu();

    // KBookmarkOwner interface:
    virtual void openBookmarkURL( const QString& url ) { emit openURL( url ); }
    virtual QString currentURL() const;

    KMenu *menu() const { return m_menu; }

signals:
    void openURL( const QString& url );

private:
    void importOldBookmarks( const QString& path, KBookmarkManager *manager );

    KFileDialog *m_dialog;
    KMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileBookmarkHandlerPrivate;
    KFileBookmarkHandlerPrivate *d;
};


#endif // KFILEBOOKMARKHANDLER_H
