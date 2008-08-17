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

#include <QtCore/QObject>
#include <QtCore/QStack>

#include <klocale.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kicon.h>
#include <krun.h>
#include <kmenu.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class KBookmark;
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
 * any way, you must reimplement either this class or KBookmarkOwner.
 *
 * Using this class is very simple:
 *
 * 1) Create a popup menu (either KActionMenu or KMenu will do)
 * 2) Instantiate a new KBookmarkMenu object using the above popup
 *    menu as a parameter
 * 3) Insert your (now full) popup menu wherever you wish
 *
 */
class KIO_EXPORT KBookmarkMenu : public QObject
{
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

  /**
   * Creates a bookmark submenu
   */
  KBookmarkMenu( KBookmarkManager* mgr, KBookmarkOwner * owner,
                 KMenu * parentMenu, const QString & parentAddress);

  ~KBookmarkMenu();

  /**
   * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its
   * final size before it is executed.
   **/
  void ensureUpToDate();

public Q_SLOTS:
    // public for KonqBookmarkBar
  void slotBookmarksChanged( const QString & ); 

protected Q_SLOTS:
  void slotAboutToShow();
  void slotAddBookmarksList();
  void slotAddBookmark();
  void slotNewFolder();
  void slotOpenFolderInTabs();

protected:
  virtual void clear();
  virtual void refill();
  virtual QAction* actionForBookmark(const KBookmark &bm);
  virtual KMenu * contextMenu(QAction * action );

  void addActions();
  void fillBookmarks();
  void addAddBookmark();
  void addAddBookmarksList();
  void addEditBookmarks();
  void addNewFolder();
  void addOpenInTabs();


  bool isRoot() const;
  bool isDirty() const;

  /**
   * Parent bookmark for this menu.
   */
  QString parentAddress() const;

  KBookmarkManager * manager() const;
  KBookmarkOwner * owner() const;
  /**
   * The menu in which we insert our actions
   * Supplied in the constructor.
   */
  KMenu * parentMenu() const;

  /**
   * List of our sub menus
   */
  QList<KBookmarkMenu *> m_lstSubMenus;

  // This is used to "export" our actions into an actionlist
  // we got in the constructor. So that the program can show our 
  // actions in their shortcut dialog
  KActionCollection * m_actionCollection;
  /**
   * List of our actions.   
   */
  QList<QAction *> m_actions;


private Q_SLOTS:
  void slotCustomContextMenu( const QPoint & );

private:
  KBookmarkMenuPrivate* d;

  bool m_bIsRoot;
  bool m_bDirty;
  KBookmarkManager * m_pManager;
  KBookmarkOwner * m_pOwner;

  KMenu * m_parentMenu;

private:
  QString m_parentAddress;
};

class KIO_EXPORT KBookmarkContextMenu : public KMenu
{
    Q_OBJECT
    
public:
    KBookmarkContextMenu(const KBookmark & bm, KBookmarkManager * manager, KBookmarkOwner *owner, QWidget * parent = 0);
    virtual ~KBookmarkContextMenu();
    virtual void addActions();

public Q_SLOTS:
    void slotEditAt();
    void slotProperties();
    void slotInsert();
    void slotRemove();
    void slotCopyLocation();
    void slotOpenFolderInTabs();

protected:
    void addBookmark();
    void addFolderActions();
    void addProperties();
    void addBookmarkActions();
    void addOpenFolderInTabs();

    KBookmarkManager * manager() const;
    KBookmarkOwner * owner() const;
    KBookmark bookmark() const;

private Q_SLOTS:
    void slotAboutToShow();

private:
    KBookmark bm;
    KBookmarkManager * m_pManager;
    KBookmarkOwner * m_pOwner;
};

class KIO_EXPORT KBookmarkActionInterface
{
public:
  KBookmarkActionInterface(const KBookmark &bk);
  virtual ~KBookmarkActionInterface();
  const KBookmark bookmark() const;
private:
  KBookmark bm;
};

/***
 * A wrapper around KActionMenu to provide a nice constructor for bookmark groups.
 *
 */

class KIO_EXPORT KBookmarkActionMenu : public KActionMenu, public KBookmarkActionInterface
{
public:
  KBookmarkActionMenu(const KBookmark &bm, QObject *parent);
  KBookmarkActionMenu(const KBookmark &bm, const QString & text, QObject *parent);
  virtual ~KBookmarkActionMenu();
};

/***
 * This class is a KAction for bookmarks.
 * It provides a nice constructor.
 * And on triggered uses the owner to open the bookmark.
 *
 */

class KIO_EXPORT KBookmarkAction : public KAction, public KBookmarkActionInterface
{
  Q_OBJECT
public:
  KBookmarkAction(const KBookmark &bk, KBookmarkOwner* owner, QObject *parent);
  virtual ~KBookmarkAction();

public Q_SLOTS:
  void slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km);

private:
  KBookmarkOwner* m_pOwner;
};

#endif
