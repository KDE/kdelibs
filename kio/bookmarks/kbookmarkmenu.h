//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include "kbookmark.h"
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
class KMenu;
class RMB;

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

  /**
   * Structure used for storing information about
   * the dynamic menu setting
   */
  // TODO - transform into class, move to KonqBookmarkMenu
  struct DynMenuInfo {
     bool show;
     QString location;
     QString type;
     QString name;
     class DynMenuInfoPrivate *d;
  };

  /**
   * @return dynmenu info block for the given dynmenu name
   */
  static DynMenuInfo showDynamicBookmarks( const QString &id );
  //TODO move to KonqBookmarkMenu

  /**
   * Shows an extra menu for the given bookmarks file and type.
   * Upgrades from option inside XBEL to option in rc file
   * on first call of this function.
   * @param id the unique identification for the dynamic menu
   * @param info a DynMenuInfo struct containing the to be added/modified data
   */
  static void setDynamicBookmarks( const QString &id, const DynMenuInfo &info );
  //TODO move to KonqBookmarkMenu

  /**
   * @return list of dynamic menu ids
   */
  static QStringList dynamicBookmarksList();
  //TODO move to KonqBookmarkMenu

Q_SIGNALS:
  void aboutToShowContextMenu( const KBookmark &, QMenu * );
  /**
   * Emitted if a bookmark was selected
   * Note: If you passed a null KBookmarkOwner to the constructor, this signal
   * is not emitted, instead KRun is used to open the bookmark.
  **/
  void openBookmark( KBookmark, Qt::MouseButtons, Qt::KeyboardModifiers );

protected Q_SLOTS:
  void slotBookmarksChanged( const QString & );
  void slotAboutToShow();
  void contextMenu( const QPoint & );

  void slotBookmarkSelected(Qt::MouseButtons, Qt::KeyboardModifiers);
  void slotAddBookmarksList();
  void slotAddBookmark();
  void slotNewFolder();

protected:
  /**
   * Creates a bookmark submenu
   */
  KBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 KActionCollection * collec, const QString & parentAddress);
  void fillBookmarkMenu();
  void refill();
  void addAddBookmark();
  void addAddBookmarksList();
  void addEditBookmarks();
  void addNewFolder();

  void showContextMenu( const QString &, const QPoint & pos);

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
  QList<KAction *> m_actions;
  /**
   * Parent bookmark for this menu.
   */
  QString m_parentAddress;


  RMB *m_rmb;
private:
  KBookmarkMenuPrivate* d;
};

class KImportedBookmarkMenu : public KBookmarkMenu
{
  friend class KBookmarkMenuNSImporter;
  Q_OBJECT
public:
  //TODO simplfy
  KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 KActionCollection * collec, const QString & type, const QString & location );
  KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 KActionCollection * collec);
  ~KImportedBookmarkMenu();
protected Q_SLOTS:
  void slotNSLoad();
private:
   QString m_type;
   QString m_location;
};

/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KIO_EXPORT KBookmarkMenuNSImporter : public QObject
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

#endif
