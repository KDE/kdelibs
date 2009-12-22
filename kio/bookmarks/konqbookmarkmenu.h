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

#ifndef __konqbookmarkmenu_h__
#define __konqbookmarkmenu_h__
#include "kbookmarkmenu.h"

class KIO_EXPORT KonqBookmarkOwner : public KBookmarkOwner // KDE5 TODO: merge with KBookmarkOwner
{
public:
  virtual ~KonqBookmarkOwner();
  virtual void openInNewTab(const KBookmark &bm) = 0;
  virtual void openInNewWindow(const KBookmark &bm) = 0;
};

class KIO_EXPORT KonqBookmarkMenu : public KBookmarkMenu
{
    //friend class KBookmarkBar;
  Q_OBJECT
public:
  /**
   * Fills a bookmark menu with konquerors bookmarks
   * (one instance of KonqBookmarkMenu is created for the toplevel menu,
   *  but also one per submenu).
   *
   * @param mgr The bookmark manager to use (i.e. for reading and writing)
   * @param owner implementation of the KonqBookmarkOwner callback interface.
   * Note: If you pass a null KonqBookmarkOwner to the constructor, the
   * URLs are openend by KRun and "Add Bookmark" is disabled.
   * @param parentMenu menu to be filled
   * @param collec parent collection for the KActions.
   */
  KonqBookmarkMenu( KBookmarkManager* mgr, KonqBookmarkOwner * owner, KBookmarkActionMenu * parentMenu, KActionCollection *collec)
    : KBookmarkMenu( mgr, owner, parentMenu->menu(), collec)
  {
  }
  ~KonqBookmarkMenu()
  {}

  /**
   * Creates a bookmark submenu.
   * Only used internally and for bookmark toolbar.
   */
  KonqBookmarkMenu( KBookmarkManager* mgr, KonqBookmarkOwner * owner, KBookmarkActionMenu * parentMenu, QString parentAddress)
    : KBookmarkMenu( mgr, owner, parentMenu->menu(), parentAddress)
  {
  }

protected:
  /**
   * Structure used for storing information about
   * the dynamic menu setting
   */
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

  /**
   * Shows an extra menu for the given bookmarks file and type.
   * Upgrades from option inside XBEL to option in rc file
   * on first call of this function.
   * @param id the unique identification for the dynamic menu
   * @param info a DynMenuInfo struct containing the to be added/modified data
   */
  static void setDynamicBookmarks( const QString &id, const DynMenuInfo &info );

  /**
   * @return list of dynamic menu ids
   */
  static QStringList dynamicBookmarksList();

  virtual void refill();
  virtual QAction* actionForBookmark(const KBookmark &bm);
  virtual KMenu * contextMenu(QAction * act);
  void fillDynamicBookmarks();
private:
  KonqBookmarkOwner * owner()
    { return static_cast<KonqBookmarkOwner *>(KBookmarkMenu::owner());}
};

class KIO_EXPORT KonqBookmarkContextMenu : public KBookmarkContextMenu
{
  Q_OBJECT
public:
  KonqBookmarkContextMenu(const KBookmark & bm, KBookmarkManager * mgr, KonqBookmarkOwner * owner );
  virtual ~KonqBookmarkContextMenu();
  virtual void addActions();

public Q_SLOTS:
  void openInNewTab();
  void openInNewWindow();
  void toggleShowInToolbar();
private:
  KonqBookmarkOwner * owner()
    { return static_cast<KonqBookmarkOwner *>(KBookmarkContextMenu::owner());}
};
#endif

