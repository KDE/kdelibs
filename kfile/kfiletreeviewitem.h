/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
                 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kfile_tree_view_item_h
#define kfile_tree_view_item_h

#include <qlistview.h>
#include <kfileitem.h>
#include <kio/global.h>
#include <kdirlister.h>
#include <kio/job.h>
#include <qdict.h>

class KURL;
class KFileTreeView;
class KFileTreeBranch;
class KFileTreeItem;

class KFileTreeViewItem : public QListViewItem
{
public:
   KFileTreeViewItem( KFileTreeViewItem*, KFileItem*, KFileTreeBranch * );
   KFileTreeViewItem( KFileTreeView*, KFileItem*, KFileTreeBranch * );
   KFileTreeBranch* branch() const     { return m_branch; }
   KFileItem *fileItem() const         { return m_kfileitem; }
   QString path() const;
   KURL    url() const;
   bool    isDir() const;

   virtual void middleButtonPressed() { }
   virtual void rightButtonPressed() { }

protected:

private:

   KFileItem *m_kfileitem;
   KFileTreeBranch *m_branch;

   class KFileTreeViewItemPrivate;
   KFileTreeViewItemPrivate *d;
};

#endif

