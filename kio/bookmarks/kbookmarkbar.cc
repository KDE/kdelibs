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
#include <ktoolbarbutton.h>

#include <kconfig.h>
#include <kpopupmenu.h>

#include "kbookmarkdrag.h"
#include "kbookmarkmenu_p.h"

#include "dptrtemplate.h"

#ifndef enable_final_users_suck
#define enable_final_users_suck

static bool *s_advanced = 0;

static bool isAdvanced()
{
  if (!s_advanced)
  {
    s_advanced = new bool;
    KConfig config("kbookmarkrc", false, false);
    config.setGroup("Bookmarks");
    (*s_advanced) = config.readBoolEntry("AdvancedAddBookmark", false);
  }
  return (*s_advanced);
}

#endif

class KBookmarkBarPrivate : public dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate> {
public:
    QPtrList<KAction> m_actions;
};
QPtrDict<KBookmarkBarPrivate>* dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate>::d_ptr = 0;

#define dptr() KBookmarkBarPrivate::d(this)

KBookmarkBar::KBookmarkBar( KBookmarkManager* mgr,
                            KBookmarkOwner *_owner, KToolBar *_toolBar,
                            KActionCollection *coll,
                            QObject *parent, const char *name )
    : QObject( parent, name ), m_pOwner(_owner), m_toolBar(_toolBar),
      m_actionCollection( coll ), m_pManager(mgr)
{
    m_lstSubMenus.setAutoDelete( true );

    if ( isAdvanced() )
    {
        m_toolBar->setAcceptDrops( true );
        m_toolBar->installEventFilter( this ); // for drops
    }

    dptr()->m_actions.setAutoDelete( true );

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

    QPtrListIterator<KAction> it( dptr()->m_actions );
    for (; it.current(); ++it )
        it.current()->unplugAll();

    dptr()->m_actions.clear();
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

                dptr()->m_actions.append( action );
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
            connect(menu, SIGNAL( aboutToShowContextMenu(const KBookmark &, QPopupMenu * ) ), 
                    this, SIGNAL( aboutToShowContextMenu(const KBookmark &, QPopupMenu * ) ));
            menu->fillBookmarkMenu();
            action->plug(m_toolBar);
            m_lstSubMenus.append( menu );

            dptr()->m_actions.append( action );
        }
    }
}

void KBookmarkBar::slotBookmarkSelected()
{
    if (!m_pOwner) return; // this view doesn't handle bookmarks...
    m_pOwner->openBookmarkURL( sender()->property("url").toString() );
}

static KToolBar* sepToolBar = 0;
static const int sepId = -9999; // fixme with define for num?

static void removeTempSep()
{
    if (sepToolBar) {
        sepToolBar->removeItem(sepId);
        sepToolBar = 0; // needed?
    }
}

static KAction* findPluggedAction(QPtrList<KAction> actions, KToolBar *tb, int id)
{
    QPtrListIterator<KAction> it( actions );
    for (; (*it); ++it )
        if ((*it)->isPlugged(tb, id))
            return (*it);
    return 0;
}

/** 
 * Handle a QDragMoveEvent event on a toolbar drop
 * @return the action to be dropped after (@see @param atFirst)
 *         else a NULL if event should be ignored, this occurs
 *         frequently and should be handled by ignoring the return
 *         value and @param atFirst.
 * @param pos the current QDragMoveEvent position
 * @param actions the list of actions plugged into the bar
 * @param atFirst bool reference, when true the position before the 
 *        returned action was dropped on
 */
static KAction* handleToolbarDragMoveEvent(QPoint pos, QPtrList<KAction> actions, bool &atFirst)
{
    static int sepIndex;
    KToolBar *tb = dynamic_cast<KToolBar*>(actions.first()->container(0));
    Q_ASSERT(tb);

    sepToolBar = tb;
    sepToolBar->removeItemDelayed(sepId);

    int index;
    KToolBarButton* b;
    b = dynamic_cast<KToolBarButton*>(tb->childAt(pos)); 
    KAction *a = 0;
    atFirst = false;

    if (b)
    {
        index = tb->itemIndex(b->id());
        QRect r = b->geometry();
        if (index == 0)
            atFirst = true;
        else if (pos.x() < ((r.left() + r.right())/2))
        {
            // if in first half of button then 
            // we jump to previous index
            index--;
            b = tb->getButton(tb->idAt(index));
        }
    }
    else // (!b)
    {
        index = actions.count() - 1;
        b = tb->getButton(tb->idAt(index));
        // if !b and not past last button, we didn't find button
        if (pos.x() <= b->geometry().left())
            goto failure_exit;
    }

    a = findPluggedAction(actions, tb, b->id());
    Q_ASSERT(a);
    sepIndex = index + (atFirst ? 0 : 1);

failure_exit:
    tb->insertLineSeparator(sepIndex, sepId);
    return a;
}

bool KBookmarkBar::eventFilter( QObject *, QEvent *e ){
    static bool atFirst = false;
    static KAction* a = 0;
    if ( e->type() == QEvent::DragLeave )
    {
        removeTempSep();
        a = 0;
    }
    else if ( e->type() == QEvent::Drop )
    {
        removeTempSep();
        QDropEvent *dev = (QDropEvent*)e;
        if ( KBookmarkDrag::canDecode( dev ) )
           return false;
        QValueList<KBookmark> list = KBookmarkDrag::decode( dev );
        if (list.count() > 1)
           kdWarning(7043) << "Sorry, currently you can only drop one address "
                              "onto the bookmark bar!" << endl;
        KBookmark toInsert = list.first();
        QString address = a->property("address").toString();
        KBookmark bookmark = m_pManager->findByAddress( address );
        Q_ASSERT(!bookmark.isNull());
        kdDebug(7043) << "inserting " 
                      << QString(atFirst ? "before" : "after")
                      << " address == " << address << endl;
        KBookmarkGroup parentBookmark = bookmark.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark( 
                                      m_pManager, toInsert.fullText(),
                                      toInsert.url() );
        parentBookmark.moveItem( newBookmark, atFirst ? KBookmark() : bookmark );
        m_pManager->emitChanged( parentBookmark );
        return true;
    }
    else if ( e->type() == QEvent::DragMove )
    {
        QDragMoveEvent *dme = (QDragMoveEvent*)e;
        if (!KBookmarkDrag::canDecode( dme ))
            return false;
        KAction *_a; 
        bool _atFirst;
        _a = handleToolbarDragMoveEvent(dme->pos(), dptr()->m_actions, _atFirst);
        if (_a)
        {
            a = _a;
            atFirst = _atFirst;
            dme->accept();
        }
    }
    return false;
}

#undef dptr
#include "kbookmarkbar.moc"
