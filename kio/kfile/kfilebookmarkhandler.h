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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFILEBOOKMARKHANDLER_H
#define KFILEBOOKMARKHANDLER_H

#include <kbookmarkmanager.h>
#include <kbookmarkmenu.h>

class QTextStream;
class KPopupMenu;

class KFileBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KFileBookmarkHandler( KFileDialog *dialog );
    ~KFileBookmarkHandler();

    QPopupMenu * popupMenu();

    // KBookmarkOwner interface:
    virtual void openBookmarkURL( const QString& url ) { emit openURL( url ); }
    virtual QString currentURL() const;

    KPopupMenu *menu() const { return m_menu; }

signals:
    void openURL( const QString& url );

private slots:
    // for importing
    void slotNewBookmark( const QString& text, const QCString& url,
                          const QString& additionalInfo );
    void slotNewFolder( const QString& text, bool open,
                        const QString& additionalInfo );
    void newSeparator();
    void endFolder();

private:
    void importOldBookmarks( const QString& path, const QString& destinationPath );

    KFileDialog *m_dialog;
    KPopupMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;

    QTextStream *m_importStream;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KFileBookmarkHandlerPrivate;
    KFileBookmarkHandlerPrivate *d;
};


#endif // KFILEBOOKMARKHANDLER_H
