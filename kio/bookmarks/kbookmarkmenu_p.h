/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <sys/types.h>

#include <qptrlist.h>
#include <qptrstack.h>
#include <qobject.h>
#include <qlistview.h>

#include <kdialogbase.h>
#include <klocale.h>
#include <kaction.h>

#include "kbookmark.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmanager.h"

class QString;
class QPopupMenu;
class QPushButton;
class QListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KPopupMenu;

class KImportedBookmarksActionMenu : public KActionMenu {
  Q_OBJECT
  Q_PROPERTY( QString type READ type WRITE setType )
  Q_PROPERTY( QString location READ location WRITE setLocation )
public:
  const QString type() const { return m_type; }
  void setType(const QString &type) { m_type = type; }
  const QString location() const { return m_location; }
  void setLocation(const QString &location) { m_location = location; }
private:
  QString m_type;
  QString m_location;
public:
  KImportedBookmarksActionMenu( 
    const QString &text, const QString& sIconName,
    KActionCollection* parent, const char* name)
  : KActionMenu(text, sIconName, parent, name) {
     ;
  }
};

class KBookmarkActionMenu : public KActionMenu {
  Q_OBJECT
  Q_PROPERTY( QString url READ url WRITE setUrl )
  Q_PROPERTY( QString address READ address WRITE setAddress )
  Q_PROPERTY( bool readOnly READ readOnly WRITE setReadOnly )
public:
  const QString url() const { return m_url; }
  void setUrl(const QString &url) { m_url = url; }
  const QString address() const { return m_address; }
  void setAddress(const QString &address) { m_address = address; }
  bool readOnly() const { return m_readOnly; }
  void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
private:
  QString m_url;
  QString m_address;
  bool m_readOnly;
public:
  KBookmarkActionMenu( 
    const QString &text, const QString& sIconName,
    KActionCollection* parent, const char* name)
  : KActionMenu(text, sIconName, parent, name) {
     ;
  }
};

class KBookmarkAction : public KAction {
  Q_OBJECT
  Q_PROPERTY( QString url READ url WRITE setUrl )
  Q_PROPERTY( QString address READ address WRITE setAddress )
public:
  const QString url() const { return m_url; }
  void setUrl(const QString &url) { m_url = url; }
  const QString address() const { return m_address; }
  void setAddress(const QString &address) { m_address = address; }
private:
  QString m_url;
  QString m_address;
public:
  KBookmarkAction(
    const QString& text, const QString& sIconName, const KShortcut& cut,
    const QObject* receiver, const char* slot,
    KActionCollection* parent, const char* name)
  : KAction(text, sIconName, cut, receiver, slot, parent, name) {
    ;
  }
};

class KBookmarkEditDialog : public KDialogBase
{
  Q_OBJECT

public:
  KBookmarkEditDialog( const QString& title, const QString& url, KBookmarkManager *, QWidget * = 0, const char * = 0, const QString& caption = i18n( "Add Bookmark" ) );
  QString finalUrl() const;
  QString finalTitle() const;
  QString finalAddress() const;

protected slots:
  void slotOk();
  void slotCancel();
  void slotInsertFolder();
  void slotDoubleClicked(QListViewItem* item);

private:
  QWidget * m_main;
  QListView * m_folderTree;
  KLineEdit * m_url;
  KLineEdit * m_title;
  QPushButton * m_button;
  KBookmarkManager * m_mgr;
};

class KBookmarkFolderTreeItem : public QListViewItem
{
  // make this an accessor
  friend class KBookmarkFolderTree;
public:
  KBookmarkFolderTreeItem( QListView *, const KBookmark & );
  KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *, QListViewItem *, const KBookmarkGroup & );
private:
  KBookmark m_bookmark;
};

class KBookmarkFolderTree
{
public:
  static QListView* createTree( KBookmarkManager *, QWidget * = 0, const char * = 0 );
  static void fillTree( QListView*, KBookmarkManager * );
  static QString selectedAddress( QListView* );
};

#endif
