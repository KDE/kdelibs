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

#ifndef __kbookmarktree_h__
#define __kbookmarktree_h__

#include <sys/types.h>

#include <qptrlist.h>
#include <qptrstack.h>
#include <qobject.h>
#include <qlistview.h>

#include <kdialogbase.h>

#include "kbookmark.h"
#include "kbookmarkmanager.h"

class QString;
class QGridLayout;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KPopupMenu;
class QPopupMenu;
class QListView;

class KBookmarkFolderTreeItem : public QListViewItem
{
  friend class KBookmarkFolderTree;
public:
  KBookmarkFolderTreeItem( QListView *, const KBookmark & );
  KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *, QListViewItem *, const KBookmarkGroup & );
private:
  KBookmark m_bookmark;
};

class KBookmarkFolderTree
{
public:
  static QListView* createTree( KBookmarkManager *, QWidget * = 0, const char * = 0 );
  static QString selectedAddress( QListView* );
};

#endif
