/* This file is part of the KDEproject
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

#include <kdebug.h>
#include <kfileitem.h>
#include <kicontheme.h>

#include "kfiletreeviewitem.h"

/* --- KFileTreeViewItem --- */
/*
 * hmm. Dont know, if it is good just to remember the UDS entry in order to save space.
 * the KFileItem is a relatively large thing and can be reproduced having the UDSEntry.
 */
KFileTreeViewItem::KFileTreeViewItem( KFileTreeViewItem *parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   : QListViewItem( parent ),
     m_kfileitem( item ),
     m_branch( brnch )
{
   setPixmap(0, item->pixmap( KIcon::SizeSmall ));
   setText( 0, item->text());

}

KFileTreeViewItem::KFileTreeViewItem( KFileTreeView* parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   :QListViewItem( (QListView*)parent ),
    m_kfileitem(item ),
    m_branch( brnch )
{
}

KURL KFileTreeViewItem::url() const
{
    return m_kfileitem ? m_kfileitem->url() : KURL();
}

QString KFileTreeViewItem::path()  const
{
    return m_kfileitem ? m_kfileitem->url().path() : QString::null;
}

bool KFileTreeViewItem::isDir() const
{
    return m_kfileitem ? m_kfileitem->isDir() : false;
}
