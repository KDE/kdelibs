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

#include <q3ptrlist.h>
#include <q3ptrstack.h>
#include <qobject.h>
#include <q3listview.h>
#include <QStack>

#include <kdialogbase.h>
#include <klocale.h>
#include <kaction.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QMenu;
class QPushButton;
class Q3ListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KMenu;

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
  Q_OBJECT
  friend class KBookmarkMenuNSImporter;
  friend class RMB;
public:
  /**
   * Fills a bookmark menu
   * (one instance of KBookmarkMenu is created for the toplevel menu,
   *  but also one per submenu).
   *
   * @param mgr The bookmark manager to use (i.e. for reading and writing)
   * @param owner implementation of the KBookmarkOwner callback interface.
   * @param parentMenu menu to be filled
   * @param collec parent collection for the KActions.
   *  Only used for other menus than the toplevel one.
   * @param root true for the toplevel menu
   * @param add true to show the "Add Bookmark" and "New Folder" entries
   * @param parentAddress the address of the group containing the items
   *  that we want to show.
   * @see KBookmark::address.
   * Be careful :
   * A _null_ parentAddress denotes a NS-bookmark menu.
   * An _empty_ parentAddress denotes the toplevel bookmark menu
   */
  KBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 KActionCollection * collec, bool root, bool add = true,
                 const QString & parentAddress = "" );

  ~KBookmarkMenu();

  /**
   * Even if you think you need to use this, you are probably wrong.
   * It fills a bookmark menu starting a given KBookmark.
   * This is public for KBookmarkBar.
   */
  void fillBookmarkMenu();

  /**
   * Call ensureUpToDate() if you need KBookmarkMenu to adjust to its
   * final size before it is executed.
   **/
  void ensureUpToDate();

  /**
   * Structure used for storing information about
   * the dynamic menu setting
   * @since 3.2
   */
  // TODO - transform into class
  struct DynMenuInfo {
     bool show;
     QString location;
     QString type;
     QString name;
     class DynMenuInfoPrivate *d;
  };

  /**
   * @return dynmenu info block for the given dynmenu name
   * @since 3.2
   */
  static DynMenuInfo showDynamicBookmarks( const QString &id );

  /**
   * Shows an extra menu for the given bookmarks file and type.
   * Upgrades from option inside XBEL to option in rc file
   * on first call of this function.
   * @param id the unique identification for the dynamic menu
   * @param info a DynMenuInfo struct containing the to be added/modified data
   * @since 3.2
   */
  static void setDynamicBookmarks( const QString &id, const DynMenuInfo &info );

  /**
   * @return list of dynamic menu ids
   * @since 3.2
   */
  static QStringList dynamicBookmarksList();

signals:
  void aboutToShowContextMenu( const KBookmark &, QMenu * );
  /**
   * @since 3.4
   */
  void openBookmark( const QString& url, Qt::ButtonState state );

public slots: // public for bookmark bar
  void slotBookmarksChanged( const QString & );

protected slots:
  void slotAboutToShow();
  void slotAboutToShowContextMenu( KMenu *, QAction*, QMenu * );
  void slotActionHighlighted( KAction * );

  void slotRMBActionRemove( int );
  void slotRMBActionInsert( int );
  void slotRMBActionCopyLocation( int );
  void slotRMBActionEditAt( int );
  void slotRMBActionProperties( int );

  void slotBookmarkSelected();
  /**
   * @ since 3.4
   */
  void slotBookmarkSelected( KAction::ActivationReason reason, Qt::ButtonState state );
  void slotAddBookmarksList();
  void slotAddBookmark();
  void slotNewFolder();

  /**
   * load Netscape's bookmarks
   */
  void slotNSLoad();

protected:
  KExtendedBookmarkOwner* extOwner();
  void refill();
  void addAddBookmark();
  void addAddBookmarksList();
  void addEditBookmarks();
  void addNewFolder();

  void fillContextMenu( QMenu *, const QString &, int );

  bool m_bIsRoot:1;
  bool m_bAddBookmark:1;
  bool m_bDirty:1;
  bool m_bNSBookmark:1;
  bool m_bAddShortcuts:1;

  KBookmarkManager * m_pManager;
  KBookmarkOwner *m_pOwner;
  /**
   * The menu in which we plug our actions.
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

  class KBookmarkMenuPrivate* d;

  // TODO make non static!
  static QString s_highlightedAddress;
  static QString s_highlightedImportLocation;
  static QString s_highlightedImportType;
};

/**
 * A class connected to KNSBookmarkImporter, to fill KActionMenus.
 */
class KIO_EXPORT KBookmarkMenuNSImporter : public QObject
{
  Q_OBJECT
public:
  KBookmarkMenuNSImporter( KBookmarkManager* mgr, KBookmarkMenu * menu, KActionCollection * act ) :
     m_menu(menu), m_actionCollection(act), m_pManager(mgr) {}

  void openNSBookmarks();
  void openBookmarks( const QString &location, const QString &type );
  void connectToImporter( const QObject &importer );

protected slots:
  void newBookmark( const QString & text, const QString & url, const QString & );
  void newFolder( const QString & text, bool, const QString & );
  void newSeparator();
  void endFolder();

protected:
  QStack<KBookmarkMenu*> mstack;
  KBookmarkMenu * m_menu;
  KActionCollection * m_actionCollection;
  KBookmarkManager* m_pManager;
};

#endif
