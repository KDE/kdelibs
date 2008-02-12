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

#include "konqbookmarkmenu.h"

#include <QMenu>
#include <QFile>

#include <kmenu.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kactioncollection.h>

#include "kbookmarkimporter.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"
#include "kbookmarkmenu_p.h"

KonqBookmarkContextMenu::KonqBookmarkContextMenu(const KBookmark & bm, KBookmarkManager * mgr, KonqBookmarkOwner * owner)
    : KBookmarkContextMenu(bm, mgr, owner)
{
}

KonqBookmarkContextMenu::~KonqBookmarkContextMenu()
{
}


void KonqBookmarkContextMenu::addActions()
{
  KConfigGroup config = KSharedConfig::openConfig("kbookmarkrc", KConfig::NoGlobals)->group("Bookmarks");
  bool filteredToolbar = config.readEntry( "FilteredToolbar", false );

  if (bookmark().isGroup())
  {
    addOpenFolderInTabs();
    addBookmark();

    if(filteredToolbar)
    {
        QString text = bookmark().showInToolbar() ? i18n("Hide in toolbar") : i18n("Show in toolbar");
        addAction( SmallIcon(""), text, this, SLOT( toggleShowInToolbar()));
    }

    addFolderActions();
  }
  else
  {
    if(owner())
    {
      addAction( SmallIcon("window-new"), i18n( "Open in New Window" ), this, SLOT( openInNewWindow() ) );
      addAction( SmallIcon("tab-new"), i18n( "Open in New Tab" ), this, SLOT( openInNewTab() ) );
    }
    addBookmark();

    if(filteredToolbar)
    {
        QString text = bookmark().showInToolbar() ? i18n("Hide in toolbar") : i18n("Show in toolbar");
        addAction( SmallIcon(""), text, this, SLOT( toggleShowInToolbar()));
    }

    addBookmarkActions();
  }
}

void KonqBookmarkContextMenu::toggleShowInToolbar()
{
    bookmark().setShowInToolbar(!bookmark().showInToolbar());
    manager()->emitChanged(bookmark().parentGroup());
}

void KonqBookmarkContextMenu::openInNewTab()
{
  owner()->openInNewTab(bookmark());
}

void KonqBookmarkContextMenu::openInNewWindow()
{
  owner()->openInNewWindow(bookmark());
}

/******************************/
/******************************/
/******************************/

void KonqBookmarkMenu::fillDynamicBookmarks()
{
  if ( isDirty()
       && KBookmarkManager::userBookmarksManager()->path() == manager()->path() )
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
        parentMenu()->addSeparator();
        haveSep = true;
      }

      KActionMenu * actionMenu;
      actionMenu = new KActionMenu( KIcon(info.type), info.name, this );
      m_actionCollection->addAction( "kbookmarkmenu", actionMenu );

      parentMenu()->addAction(actionMenu);
      m_actions.append( actionMenu );

      KImportedBookmarkMenu *subMenu =
        new KImportedBookmarkMenu( manager(), owner(), actionMenu->menu(),
                                   info.type, info.location);
      m_lstSubMenus.append( subMenu );
    }
  }
}

void KonqBookmarkMenu::refill()
{
  if(isRoot())
    addActions();
  fillDynamicBookmarks();
  fillBookmarks();
  if(!isRoot())
    addActions();
}

QAction* KonqBookmarkMenu::actionForBookmark(const KBookmark &bm)
{
  if ( bm.isGroup() )
  {
    kDebug(7043) << "Creating Konq bookmark submenu named " << bm.text();
    KBookmarkActionMenu * actionMenu = new KBookmarkActionMenu( bm, this );
    m_actionCollection->addAction( "kbookmarkmenu", actionMenu );
    m_actions.append( actionMenu );

    KBookmarkMenu *subMenu = new KonqBookmarkMenu( manager(), owner(), actionMenu, bm.address() );

    m_lstSubMenus.append( subMenu );
    return actionMenu;
  }
  else if( bm.isSeparator() )
  {
    return KBookmarkMenu::actionForBookmark(bm);
  }
  else
  {
    kDebug(7043) << "Creating Konq bookmark action named " << bm.text();
    KBookmarkAction * action = new KBookmarkAction( bm, owner(), this );
    m_actionCollection->addAction(action->objectName(), action);
    m_actions.append( action );
    return action;
  }
}

KonqBookmarkMenu::DynMenuInfo KonqBookmarkMenu::showDynamicBookmarks( const QString &id )
{
    KConfig bookmarkrc("kbookmarkrc", KConfig::NoGlobals);
    KConfigGroup config(&bookmarkrc, "Bookmarks");

    DynMenuInfo info;
    info.show = false;

    if (!config.hasKey("DynamicMenus")) {
        if (bookmarkrc.hasGroup("DynamicMenu-" + id)) {
            KConfigGroup dynGroup(&bookmarkrc, "DynamicMenu-" + id);
            info.show = dynGroup.readEntry("Show", false);
            info.location = dynGroup.readPathEntry("Location", QString());
            info.type = dynGroup.readEntry("Type");
            info.name = dynGroup.readEntry("Name");
        }
    }
    return info;
}

QStringList KonqBookmarkMenu::dynamicBookmarksList()
{
    KConfigGroup config = KSharedConfig::openConfig("kbookmarkrc", KConfig::NoGlobals)->group("Bookmarks");

    QStringList mlist;
    if (config.hasKey("DynamicMenus"))
        mlist = config.readEntry("DynamicMenus", QStringList());

    return mlist;
}

void KonqBookmarkMenu::setDynamicBookmarks(const QString &id, const DynMenuInfo &newMenu)
{
    KSharedConfig::Ptr kbookmarkrc = KSharedConfig::openConfig("kbookmarkrc", KConfig::NoGlobals);
    KConfigGroup dynConfig = kbookmarkrc->group(QString("DynamicMenu-" + id));

    // add group unconditionally
    dynConfig.writeEntry("Show", newMenu.show);
    dynConfig.writePathEntry("Location", newMenu.location);
    dynConfig.writeEntry("Type", newMenu.type);
    dynConfig.writeEntry("Name", newMenu.name);

    QStringList elist;
    KConfigGroup config = kbookmarkrc->group("Bookmarks");
    if (config.hasKey("DynamicMenus")) {
        elist = config.readEntry("DynamicMenus", QStringList());
    }

    // make sure list includes type
    if (!elist.contains(id)) {
        elist << id;
        config.writeEntry("DynamicMenus", elist);
    }

    config.sync();
}

KonqBookmarkOwner::~KonqBookmarkOwner()
{
}

KMenu * KonqBookmarkMenu::contextMenu(QAction * action)
{
    KBookmarkActionInterface* act = dynamic_cast<KBookmarkActionInterface *>(action);
    if (!act)
        return 0;
    return new KonqBookmarkContextMenu(act->bookmark(), manager(), owner());
}

#include "konqbookmarkmenu.moc"
