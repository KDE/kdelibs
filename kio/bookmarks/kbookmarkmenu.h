//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 Daniel Teske <teske@squorn.de>

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

#ifndef __kbookmarkmenu_h__
#define __kbookmarkmenu_h__

#include <sys/types.h>

#include <qobject.h>
#include <QtCore/QStack>

#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kaction.h>
#include <kicon.h>
#include <krun.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class QPushButton;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class QAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KMenu;
class KBookmarkActionInterface;

class KBookmarkMenuPrivate; // Not implemented

namespace KIO { class Job; }

/**
 * This class provides a bookmark menu.  It is typically used in
 * cooperation with KActionMenu but doesn't have to be.
 *
 * If you use this class by itself, then it will use KDE defaults for
 * everything -- the bookmark path, bookmark editor, bookmark launcher..
 * everything.  These defaults reside in the classes
 * KBookmarkOwner (editing bookmarks) and KBookmarkManager
 * (almost everything else).  If you wish to change the defaults in
 * any way, you must reimplement and instantiate those classes
 * <em>before</em> this class is ever called.
 *
 * Using this class is very simple:
 *
 * 1) Create a popup menu (either KActionMenu or KMenu will do)
 * 2) Instantiate a new KBookmarkMenu object using the above popup
 *    menu as a parameter
 * 3) Insert your (now full) popup menu wherever you wish
 *
 * Again, if you wish to modify any defaults, the procedure is:
 *
 * 1a) Reimplement your own KBookmarkOwner
 * 1b) Reimplement and instantiate your own KBookmarkManager
 */
class KIO_EXPORT KBookmarkMenu : public QObject
{
  friend class KBookmarkBar;
  Q_OBJECT
public:
  /**
   * Fills a bookmark menu
   * (one instance of KBookmarkMenu is created for the toplevel menu,
   *  but also one per submenu).
   *
   * @param mgr The bookmark manager to use (i.e. for reading and writing)
   * @param owner implementation of the KBookmarkOwner callback interface.
   * Note: If you pass a null KBookmarkOwner to the constructor, the
   * openBookmark signal is not emitted, instead KRun is used to open the bookmark.
   * @param parentMenu menu to be filled
   * @param collec parent collection for the KActions.
   */
  KBookmarkMenu( KBookmarkManager* mgr, KBookmarkOwner * owner, KMenu * parentMenu, KActionCollection *collec);

  ~KBookmarkMenu();

  /**
   * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its
   * final size before it is executed.
   **/
  void ensureUpToDate();

protected Q_SLOTS:
  void slotBookmarksChanged( const QString & );
  void slotAboutToShow();
  void contextMenu( const QPoint & );

  void slotAddBookmarksList();
  void slotAddBookmark();
  void slotNewFolder();

protected:
  /**
   * Creates a bookmark submenu
   */
  KBookmarkMenu( KBookmarkManager* mgr, KBookmarkOwner * owner,
                 KMenu * parentMenu, const QString & parentAddress);
  virtual void clear();
  virtual void refill();
  virtual QAction* actionForBookmark(KBookmark bm);
  void addActions();
  void fillBookmarks();
  void addAddBookmark();
  void addAddBookmarksList();
  void addEditBookmarks();
  void addNewFolder();

  bool m_bIsRoot:1;
  bool m_bDirty:1;

  KBookmarkManager * m_pManager;
  KBookmarkOwner *m_pOwner;
  /**
   * The menu in which we insert our actions
   * Supplied in the constructor.
   */
  KMenu * m_parentMenu;
  /**
   * List of our sub menus
   */
  QList<KBookmarkMenu *> m_lstSubMenus;
  KActionCollection * m_actionCollection;
  /**
   * List of our actions.
   */
  QList<QAction *> m_actions;
  /**
   * Parent bookmark for this menu.
   */
  QString m_parentAddress;
private:
  KBookmarkMenuPrivate* d;
};

class KIO_EXPORT KBookmarkActionContextMenu : public QObject
{
  Q_OBJECT
public:
  void contextMenu(QPoint pos, QString highlightedAddress, KBookmarkManager *pManager, KBookmarkOwner *pOwner);
  ~KBookmarkActionContextMenu();
  KBookmark atAddress(const QString & address);
  static KBookmarkActionContextMenu & self();
public Q_SLOTS:
  void slotEditAt();
  void slotProperties();
  void slotInsert();
  void slotRemove();
  void slotCopyLocation();

protected:
  void addBookmark();
  void addFolderActions();
  void addProperties();
  void addBookmarkActions();

  QString m_parentAddress;
  QString m_highlightedAddress;
  KBookmarkManager *m_pManager;
  KBookmarkOwner *m_pOwner;
  QWidget *m_parentMenu;
  QMenu * m_contextMenu;
};


class KIO_EXPORT KBookmarkActionInterface
{
public:
  KBookmarkActionInterface(KBookmark bk);
  virtual ~KBookmarkActionInterface();
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner) = 0;
protected:
  const KBookmark bookmark() const;
private:
  KBookmark bm;
};

class KIO_EXPORT KBookmarkActionMenu : public KActionMenu, public KBookmarkActionInterface
{
public:
  KBookmarkActionMenu(KBookmark bm, QObject *parent);
  KBookmarkActionMenu(KBookmark bm, const QString & text, QObject *parent);
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner);
  virtual ~KBookmarkActionMenu();
};

class KIO_EXPORT KBookmarkAction : public KAction, public KBookmarkActionInterface
{
  Q_OBJECT
public:
  KBookmarkAction(KBookmark bk, KBookmarkOwner* owner, QObject *parent);
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner);
  virtual ~KBookmarkAction();

public Q_SLOTS:
  void slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km);

private:
  KBookmarkOwner* m_pOwner;
};

class KImportedBookmarkMenu : public KBookmarkMenu
{
  friend class KBookmarkMenuNSImporter;
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

#endif
