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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <kfileitem.h>
#include <kicontheme.h>

#include "k3filetreeviewitem.h"

/* --- K3FileTreeViewItem --- */
/*
 */
K3FileTreeViewItem::K3FileTreeViewItem( K3FileTreeViewItem *parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   : K3ListViewItem( parent ),
     m_kfileitem( item ),
     m_branch( brnch ),
     m_wasListed(false)
{
   setPixmap(0, item->pixmap( K3Icon::SizeSmall ));
   setText( 0, item->text());

}

K3FileTreeViewItem::K3FileTreeViewItem( K3FileTreeView* parent,
				      KFileItem* item,
				      KFileTreeBranch *brnch )
   :K3ListViewItem( (Q3ListView*)parent ),
    m_kfileitem(item ),
    m_branch( brnch ),
    m_wasListed(false)
{
   setPixmap(0, item->pixmap( K3Icon::SizeSmall ));
   setText( 0, item->text());
}

K3FileTreeViewItem::~K3FileTreeViewItem()
{
    if ( m_kfileitem )
        m_kfileitem->removeExtraData( m_branch );
}

bool K3FileTreeViewItem::alreadyListed() const
{
   return m_wasListed;
}

void K3FileTreeViewItem::setListed( bool wasListed )
{
   m_wasListed = wasListed;
}

KUrl K3FileTreeViewItem::url() const
{
    return m_kfileitem ? m_kfileitem->url() : KUrl();
}

QString K3FileTreeViewItem::path()  const
{
    return m_kfileitem ? m_kfileitem->url().path() : QString();
}

bool K3FileTreeViewItem::isDir() const
{
    return m_kfileitem ? m_kfileitem->isDir() : false;
}
