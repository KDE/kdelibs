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

class KIO_EXPORT KonqBookmarkOwner : public KBookmarkOwner
{
public:
  virtual ~KonqBookmarkOwner();
  virtual void openInNewTab(KBookmark bm) = 0;
  virtual void openInNewWindow(KBookmark bm) = 0;
  virtual void openFolderinTabs(KBookmark bm) = 0;
};

class KIO_EXPORT KonqBookmarkActionMenu : public KBookmarkActionMenu
{
  Q_OBJECT
public:
  KonqBookmarkActionMenu(KBookmark bm, QObject *parent);
  KonqBookmarkActionMenu(KBookmark bm, const QString & text, QObject *parent);
  virtual ~KonqBookmarkActionMenu();
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner);
};

class KonqBookmarkAction : public KBookmarkAction
{
  Q_OBJECT
public:
  KonqBookmarkAction(KBookmark bm, KonqBookmarkOwner * owner, QObject *parent);
  virtual void contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner);
  virtual ~KonqBookmarkAction();
};

class KIO_EXPORT KonqBookmarkMenu : public KBookmarkMenu
{
  friend class KBookmarkBar;
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
  KonqBookmarkMenu( KBookmarkManager* mgr, KonqBookmarkOwner * owner, KonqBookmarkActionMenu * parentMenu, KActionCollection *collec)
    : KBookmarkMenu( mgr, owner, parentMenu->menu(), collec)
  {
  }
  ~KonqBookmarkMenu()
  {}

  /**
   * Structure used for storing information about
   * the dynamic menu setting
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

protected:
  KonqBookmarkMenu( KBookmarkManager* mgr, KonqBookmarkOwner * owner, KonqBookmarkActionMenu * parentMenu, QString parentAddress)
    : KBookmarkMenu( mgr, owner, parentMenu->menu(), parentAddress)
  {
  }
  virtual void refill();
  virtual QAction* actionForBookmark(KBookmark bm);
  void fillDynamicBookmarks();
private:
  KonqBookmarkOwner * owner()
    { return static_cast<KonqBookmarkOwner *>(m_pOwner);}
};

class KonqBookmarkContextMenu : private KBookmarkActionContextMenu
{
  Q_OBJECT
public:
  ~KonqBookmarkContextMenu();
  static KonqBookmarkContextMenu & self();
  void contextMenu(QPoint pos, QString highlightedAddress, KBookmarkManager *pManager, KBookmarkOwner *pOwner);
public Q_SLOTS:
  void openInNewTab();
  void openInNewWindow();
  void openFolderinTabs();
private:
  KonqBookmarkOwner * owner()
    { return static_cast<KonqBookmarkOwner *>(m_pOwner);}
  KonqBookmarkContextMenu();
};
#endif

