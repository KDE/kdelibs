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
#include <QMenu>
#include <QFile>

#include <kmenu.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kicon.h>
#include <kiconloader.h>

#include "kbookmarkimporter.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"

#include "konqbookmarkmenu.h"

KonqBookmarkContextMenu::KonqBookmarkContextMenu()
{
}

KonqBookmarkContextMenu::~KonqBookmarkContextMenu()
{
}

KonqBookmarkContextMenu & KonqBookmarkContextMenu::self()
{
  static KonqBookmarkContextMenu s;
  return s;
}

void KonqBookmarkContextMenu::contextMenu(QPoint pos, QString highlightedAddress,
                                          KBookmarkManager *pManager, KBookmarkOwner *pOwner)
{
  m_parentAddress = KBookmark::parentAddress(highlightedAddress) ;
  m_highlightedAddress = highlightedAddress;
  m_pManager = pManager;
  m_pOwner = pOwner;
  delete m_contextMenu;
  m_contextMenu = new QMenu();

  KBookmark bookmark = atAddress(m_highlightedAddress);
  addBookmark();

  if(pOwner)
  {
    if(bookmark.isGroup())
    {
      m_contextMenu->addAction(SmallIcon("tab_new"), i18n( "Open Folder in Tabs" ), this, SLOT( openFolderinTabs() ) );
    }
    else
    {
      m_contextMenu->addAction( SmallIcon("window_new"), i18n( "Open in New Window" ), this, SLOT( openInNewWindow() ) );
      m_contextMenu->addAction( SmallIcon("tab_new"), i18n( "Open in New Tab" ), this, SLOT( openInNewTab() ) );
    }
  }

  if (bookmark.isGroup())
    addFolderActions();
  else
    addBookmarkActions();

  addProperties();

  if(!m_highlightedAddress.isNull())
    m_contextMenu->popup(pos);
}

void KonqBookmarkContextMenu::openInNewTab()
{
  owner()->openInNewTab(atAddress(m_highlightedAddress));
}

void KonqBookmarkContextMenu::openInNewWindow()
{
  owner()->openInNewWindow(atAddress(m_highlightedAddress));
}

void KonqBookmarkContextMenu::openFolderinTabs()
{
  owner()->openFolderinTabs(atAddress(m_highlightedAddress));
}

void KonqBookmarkMenu::fillDynamicBookmarks()
{
  if ( m_bIsRoot
       && KBookmarkManager::userBookmarksManager()->path() == m_pManager->path() )
  {
    bool haveSep = false;

    const QStringList keys = KonqBookmarkMenu::dynamicBookmarksList();
    for ( QStringList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
      DynMenuInfo info;
      info = showDynamicBookmarks((*it));

      if ( !info.show || !QFile::exists( info.location ) )
        continue;

      if (!haveSep)
      {
        m_parentMenu->addSeparator();
        haveSep = true;
      }

      KActionMenu * actionMenu;
      actionMenu = new KActionMenu(
                                    KIcon(info.type), info.name,
                                    m_actionCollection, "kbookmarkmenu" );

      m_parentMenu->addAction(actionMenu);
      m_actions.append( actionMenu );

      KImportedBookmarkMenu *subMenu =
        new KImportedBookmarkMenu( m_pManager, m_pOwner, actionMenu->menu(),
                                   info.type, info.location);
      m_lstSubMenus.append( subMenu );
    }
  }
}

void KonqBookmarkMenu::refill()
{
  if(m_bIsRoot)
    addActions();
  fillDynamicBookmarks();
  fillBookmarks();
  if(!m_bIsRoot)
    addActions();
}

KAction* KonqBookmarkMenu::actionForBookmark(KBookmark bm)
{
  if ( bm.isGroup() )
  {
    kDebug(7043) << "Creating Konq bookmark submenu named " << bm.text() << endl;
    KonqBookmarkActionMenu * actionMenu = new KonqBookmarkActionMenu( bm, m_actionCollection, "kbookmarkmenu" );
    m_actions.append( actionMenu );

    KBookmarkMenu *subMenu = new KonqBookmarkMenu( m_pManager, owner(), actionMenu, bm.address() );
    m_lstSubMenus.append( subMenu );
    return actionMenu;
  }
  else if( bm.isSeparator() )
  {
    return KBookmarkMenu::actionForBookmark(bm);
  }
  else
  {
    kDebug(7043) << "Creating Konq bookmark action named " << bm.text() << endl;
    KonqBookmarkAction * action = new KonqBookmarkAction( bm, m_actionCollection, owner() );
    m_actions.append( action );
    return action;
  }
}

KonqBookmarkMenu::DynMenuInfo KonqBookmarkMenu::showDynamicBookmarks( const QString &id )
{
  KConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  DynMenuInfo info;
  info.show = false;

  if (!config.hasKey("DynamicMenus")) {
    // upgrade path
    if (id == "netscape") {
      KBookmarkManager *manager = KBookmarkManager::userBookmarksManager();
      info.show = manager->root().internalElement().attribute("hide_nsbk") != "yes";
      info.location = KNSBookmarkImporter::netscapeBookmarksFile();
      info.type = "netscape";
      info.name = i18n("Netscape Bookmarks");
    } // else, no show

  } else {
    // have new version config
    if (config.hasGroup("DynamicMenu-" + id)) {
      config.setGroup("DynamicMenu-" + id);
      info.show = config.readEntry("Show", false);
      info.location = config.readPathEntry("Location");
      info.type = config.readEntry("Type");
      info.name = config.readEntry("Name");
    } // else, no show
  }

  return info;
}

QStringList KonqBookmarkMenu::dynamicBookmarksList()
{
  KConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  QStringList mlist;
  if (config.hasKey("DynamicMenus"))
    mlist = config.readEntry("DynamicMenus", QStringList());
  else
    mlist << "netscape";

  return mlist;
}

void KonqBookmarkMenu::setDynamicBookmarks(const QString &id, const DynMenuInfo &newMenu)
{
  KConfig config("kbookmarkrc", false, false);

  // add group unconditionally
  config.setGroup("DynamicMenu-" + id);
  config.writeEntry("Show", newMenu.show);
  config.writePathEntry("Location", newMenu.location);
  config.writeEntry("Type", newMenu.type);
  config.writeEntry("Name", newMenu.name);

  QStringList elist;

  config.setGroup("Bookmarks");
  if (!config.hasKey("DynamicMenus")) {
    if (newMenu.type != "netscape") {
      // update from old xbel method to new rc method
      // though only if not writing the netscape setting
      config.setGroup("DynamicMenu-" "netscape");
      DynMenuInfo xbelSetting;
      xbelSetting = showDynamicBookmarks("netscape");
      config.writeEntry("Show", xbelSetting.show);
      config.writePathEntry("Location", xbelSetting.location);
      config.writeEntry("Type", xbelSetting.type);
      config.writeEntry("Name", xbelSetting.name);
    }
  } else {
    elist = config.readEntry("DynamicMenus", QStringList());
  }

  // make sure list includes type
  config.setGroup("Bookmarks");
  if (!elist.contains(id)) {
    elist << id;
    config.writeEntry("DynamicMenus", elist);
  }

  config.sync();
}

KonqBookmarkAction::KonqBookmarkAction(KBookmark bm, KActionCollection * collec, KonqBookmarkOwner * owner)
  : KBookmarkAction(bm, collec, owner)
{
}

KonqBookmarkAction::~KonqBookmarkAction()
{
}

void KonqBookmarkAction::contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
{
  KonqBookmarkContextMenu::self().contextMenu( pos, bookmark().address(), m_pManager, m_pOwner );
}

KonqBookmarkActionMenu::KonqBookmarkActionMenu(KBookmark bm, KActionCollection* parent, const char* name)
  : KBookmarkActionMenu(bm, parent, name)
{
}

KonqBookmarkActionMenu::KonqBookmarkActionMenu(KBookmark bm, const QString & text, KActionCollection* parent, const char* name)
: KBookmarkActionMenu(bm, text, parent, name)
{
}

KonqBookmarkActionMenu::~KonqBookmarkActionMenu()
{
}

void KonqBookmarkActionMenu::contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
{
  KonqBookmarkContextMenu::self().contextMenu( pos, bookmark().address(), m_pManager, m_pOwner );
}

void KonqBookmarkOwner::openInNewTab(KBookmark bm)
{}

void KonqBookmarkOwner::openInNewWindow(KBookmark bm)
{}

void KonqBookmarkOwner::openFolderinTabs(KBookmark bm)
{}

KonqBookmarkOwner::~KonqBookmarkOwner()
{
}

#include "konqbookmarkmenu.moc"
