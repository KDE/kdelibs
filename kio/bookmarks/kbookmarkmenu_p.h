//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <sys/types.h>

#include <QtCore/QObject>

#include <kdialog.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <krun.h>
#include <kmenu.h>
#include <QtGui/QBoxLayout>
#include <QtCore/QModelIndex>
#include <QtGui/QTreeWidget>

#include "kbookmark.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class QPushButton;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KBookmarkBar;
class KMenu;

class KBookmarkEditFields {
public:
  typedef enum { FolderFieldsSet, BookmarkFieldsSet } FieldsSet;
  KLineEdit * m_url;
  KLineEdit * m_title;
  KBookmarkEditFields(QWidget *main, QBoxLayout *vbox, FieldsSet isFolder);
  void setName(const QString &str);
  void setLocation(const QString &str);
};

class KBookmarkEditDialog : public KDialog
{
  Q_OBJECT

public:
  typedef enum { ModifyMode, InsertionMode } BookmarkEditType;

  KBookmarkEditDialog( const QString& title, const QString& url, KBookmarkManager *, BookmarkEditType editType, const QString& address = QString(),
                       QWidget * = 0, const QString& caption = i18n( "Add Bookmark" ) );

  QString finalUrl() const;
  QString finalTitle() const;
  QString finalAddress() const;

protected Q_SLOTS:
  void slotOk();
  void slotCancel();
  void slotUser1();
  void slotDoubleClicked(QTreeWidgetItem* item);

private:
  void fillGroup( QTreeWidget* listview, QTreeWidgetItem * parentItem, KBookmarkGroup group, const QString& address);
  QWidget * m_main;
  KBookmarkEditFields * m_fields;
  QTreeWidget * m_folderTree;
  QPushButton * m_button;
  KBookmarkManager * m_mgr;
  BookmarkEditType m_editType;
};

class KBookmarkTreeItem : public QTreeWidgetItem
{
public:
    KBookmarkTreeItem(QTreeWidget * tree);
    KBookmarkTreeItem(QTreeWidgetItem * parent, QTreeWidget * tree, KBookmarkGroup bk);
    ~KBookmarkTreeItem();
    QString address();
private:
    QString m_address;
};

class KBookmarkSettings
{
public:
  bool m_advancedaddbookmark;
  bool m_contextmenu;
  bool m_filteredtoolbar;
  static KBookmarkSettings *s_self;
  static void readSettings();
  static KBookmarkSettings *self();
};

/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KBookmarkMenuNSImporter : public QObject
{
  Q_OBJECT
public:
  KBookmarkMenuNSImporter( KBookmarkManager* mgr, KImportedBookmarkMenu * menu, KActionCollection * act ) :
     m_menu(menu), m_actionCollection(act), m_pManager(mgr) {}

  void openNSBookmarks();
  void openBookmarks( const QString &location, const QString &type );
  void connectToImporter( const QObject &importer );

protected Q_SLOTS:
  void newBookmark( const QString & text, const QString & url, const QString & );
  void newFolder( const QString & text, bool, const QString & );
  void newSeparator();
  void endFolder();

protected:
  QStack<KImportedBookmarkMenu*> mstack;
  KImportedBookmarkMenu * m_menu;
  KActionCollection * m_actionCollection;
  KBookmarkManager* m_pManager;
};

class KImportedBookmarkActionMenu : public KActionMenu, public KBookmarkActionInterface
{
public:
  KImportedBookmarkActionMenu(const KIcon &icon, const QString &text, QObject *parent)
    : KActionMenu(icon, text, parent),
      KBookmarkActionInterface(KBookmark())
  {
  }
  ~KImportedBookmarkActionMenu()
  {}
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
  {
    Q_UNUSED(pos)
    Q_UNUSED(m_pManager)
    Q_UNUSED(m_pOwner)
  }
};

class KImportedBookmarkAction : public KAction, public KBookmarkActionInterface
{
  Q_OBJECT
public:
  KImportedBookmarkAction(KBookmark bk, KBookmarkOwner* owner, QObject *parent );
  ~KImportedBookmarkAction();
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner);

public Q_SLOTS:
  void slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km);
private:
  KBookmarkOwner * m_pOwner;
};

#endif
