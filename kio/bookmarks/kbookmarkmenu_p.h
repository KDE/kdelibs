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

#include <QtCore/QObject>

#include <kdialog.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <QtGui/QBoxLayout>
#include <QtGui/QTreeWidget>
#include <QtGui/QLabel>

#include "kbookmark.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmanager.h"

class QString;
class QPushButton;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;

class KImportedBookmarkMenu : public KBookmarkMenu
{
    friend class KBookmarkMenuImporter;
  Q_OBJECT
public:
  //TODO simplfy
  KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 const QString & type, const QString & location );
  KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu);
  ~KImportedBookmarkMenu();
  virtual void clear();
  virtual void refill();
protected Q_SLOTS:
  void slotNSLoad();
private:
   QString m_type;
   QString m_location;
};


class KBookmarkTreeItem : public QTreeWidgetItem
{
public:
    KBookmarkTreeItem(QTreeWidget * tree);
    KBookmarkTreeItem(QTreeWidgetItem * parent, QTreeWidget * tree, const KBookmarkGroup &bk);
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
  static KBookmarkSettings *s_self;
  static void readSettings();
  static KBookmarkSettings *self();
};

/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KBookmarkMenuImporter : public QObject
{
  Q_OBJECT
public:
  KBookmarkMenuImporter( KBookmarkManager* mgr, KImportedBookmarkMenu * menu ) :
     m_menu(menu), m_pManager(mgr) {}

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
};

#endif
