// -*- mode: c++; c-basic-offset: 2 -*-
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "kbookmarktree.h"
#include "kbookmarkimporter.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <qlayout.h>

#include "klineedit.h"

#include <qfile.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>

#include <qlistview.h>

#include <kiconloader.h>

KBookmarkFolderTree::KBookmarkFolderTree( QWidget* parent, const char* name )
{
#if 0
  m_pListView->clear();
  KBookmarkGroup root = s_pManager->root();
  KBookmarkFolderTreeItem * rootItem = new KBookmarkFolderTreeItem( m_pListView, root );
  fillGroup( rootItem, root );
  rootItem->QListViewItem::setOpen(true);
#endif
}

// toplevel item
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( QListView *parent, const KBookmark & gp )
   : QListViewItem(parent, i18n("Bookmarks")), m_bookmark(gp)
{
  setPixmap(0, SmallIcon("bookmark"));
  setExpandable(true); // Didn't know this was necessary :)
}

// group
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *parent, QListViewItem *after, const KBookmarkGroup & gp )
   : QListViewItem(parent, after, gp.fullText()), m_bookmark(gp)
{
  setPixmap(0, SmallIcon( gp.icon() ) );
  setExpandable(true);
}

static void fillGroup( KBookmarkFolderTreeItem * parentItem, KBookmarkGroup group )
{
  KBookmarkFolderTreeItem * lastItem = 0L;
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    kdDebug() << "KEBTopLevel::fillGroup group=" << group.text() << " bk=" << bk.text() << endl;
    if ( bk.isGroup() )
    {
      KBookmarkGroup grp = bk.toGroup();
      KBookmarkFolderTreeItem * item = new KBookmarkFolderTreeItem( parentItem, lastItem, grp );
      fillGroup( item, grp );
      if (grp.isOpen())
        item->QListViewItem::setOpen(true); // no need to save it again :)
      lastItem = item;
    }
  }
}
