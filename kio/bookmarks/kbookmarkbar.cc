/* This file is part of the KDE project
   Copyright (C) 1999 Kurt Granroth <granroth@kde.org>
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qregexp.h>

#include <kbookmarkbar.h>

#include <kaction.h>
#include <kbookmarkmenu.h>
#include <kdebug.h>

#include <ktoolbar.h>

#include <kconfig.h>
#include <kpopupmenu.h>

#include "kbookmarkmenu_p.h"
#include <qptrdict.h>

template<class KBookmarkBar, class KBookmarkBarPrivate>
class Private {
public:
    static KBookmarkBarPrivate* d( const KBookmarkBar* instance ) 
    {
        if ( !d_ptr ) {
            d_ptr = new QPtrDict<KBookmarkBarPrivate>;
            qAddPostRoutine( cleanup_d_ptr );
        }
        KBookmarkBarPrivate* ret = d_ptr->find( (void*) instance );
        if ( ! ret ) {
            ret = new KBookmarkBarPrivate;
            d_ptr->replace( (void*) instance, ret );
        }
        return ret;
    }
    static void delete_d( const KBookmarkBar* instance ) 
    {
        if ( d_ptr )
            d_ptr->remove( (void*) instance );
    }
private:
    static void cleanup_d_ptr() 
    {
        delete d_ptr;
    }
    static QPtrDict<KBookmarkBarPrivate>* d_ptr;
};

class KBookmarkBarPrivate : public Private<KBookmarkBar, KBookmarkBarPrivate> {
public:
    QPtrList<KAction> m_actions;
};
QPtrDict<KBookmarkBarPrivate>* Private<KBookmarkBar, KBookmarkBarPrivate>::d_ptr = 0;

#define d() KBookmarkBarPrivate::d(this)

KBookmarkBar::KBookmarkBar( KBookmarkManager* mgr,
                            KBookmarkOwner *_owner, KToolBar *_toolBar,
                            KActionCollection *coll,
                            QObject *parent, const char *name )
    : QObject( parent, name ), m_pOwner(_owner), m_toolBar(_toolBar),
      m_actionCollection( coll ), m_pManager(mgr)
{
    m_lstSubMenus.setAutoDelete( true );

    d()->m_actions.setAutoDelete( true );

    connect( mgr, SIGNAL( changed(const QString &, const QString &) ),
             SLOT( slotBookmarksChanged(const QString &) ) );

    KBookmarkGroup toolbar = mgr->toolbar();
    fillBookmarkBar( toolbar );
}

KBookmarkBar::~KBookmarkBar()
{
    clear();
}

void KBookmarkBar::clear()
{
    m_lstSubMenus.clear();
    if ( m_toolBar )
        m_toolBar->clear();

    QPtrListIterator<KAction> it( d()->m_actions );
    for (; it.current(); ++it )
        it.current()->unplugAll();

    d()->m_actions.clear();
}

void KBookmarkBar::slotBookmarksChanged( const QString & group )
{
    KBookmarkGroup tb = m_pManager->toolbar();
    if ( tb.isNull() )
        return;
    if ( tb.address() == group )
    {
        clear();
        fillBookmarkBar( tb );
    } else
    {
        // Iterate recursively into child menus
        QPtrListIterator<KBookmarkMenu> it( m_lstSubMenus );
        for (; it.current(); ++it )
        {
            it.current()->slotBookmarksChanged( group );
        }
    }

}

void KBookmarkBar::fillBookmarkBar(KBookmarkGroup & parent)
{
    if (parent.isNull())
        return;

    for (KBookmark bm = parent.first(); !bm.isNull(); bm = parent.next(bm))
    {
        QString text = bm.text();
        text.replace( '&', "&&" );
        if (!bm.isGroup())
        {
            if ( bm.isSeparator() )
                m_toolBar->insertLineSeparator();
            else
            {
                // create a normal URL item, with ID as a name
                KAction *action = new KBookmarkAction( text, bm.icon(), 0,
                                                       this, SLOT(slotBookmarkSelected()),
                                                       m_actionCollection, 0 );

                action->setProperty( "url", bm.url().url() );
                action->setProperty( "address", bm.address() );

                // ummm.... this doesn't appear do anything...
                action->setToolTip( bm.url().prettyURL() );

                action->plug(m_toolBar);

                d()->m_actions.append( action );
            }
        }
        else
        {
            KActionMenu *action = new KBookmarkActionMenu( text, bm.icon(),
                                                           m_actionCollection, 
                                                           "bookmarkbar-actionmenu");
            action->setProperty( "address", bm.address() );
            action->setDelayed(false);

            // this flag doesn't have any UI yet
            KGlobal::config()->setGroup( "Settings" );
            bool addEntriesBookmarkBar = KGlobal::config()->readBoolEntry("AddEntriesBookmarkBar",true);

            KBookmarkMenu *menu = new KBookmarkMenu(m_pManager, m_pOwner, action->popupMenu(),
                                                    m_actionCollection, false, addEntriesBookmarkBar,
                                                    bm.address());
            menu->fillBookmarkMenu();
            action->plug(m_toolBar);
            m_lstSubMenus.append( menu );

            d()->m_actions.append( action );
        }
    }
}

void KBookmarkBar::slotBookmarkSelected()
{
    if (!m_pOwner) return; // this view doesn't handle bookmarks...
    m_pOwner->openBookmarkURL( sender()->property("url").toString() );
}

#include "kbookmarkbar.moc"
