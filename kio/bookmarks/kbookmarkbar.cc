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
#include <qfile.h>

#include <kbookmarkbar.h>
#include <kbookmarkdrag.h>

#include <kaction.h>
#include <kbookmarkmenu.h>
#include <kdebug.h>

#include <ktoolbar.h>
#include <ktoolbarbutton.h>

#include <kconfig.h>
#include <kpopupmenu.h>

#include "kbookmarkdrag.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkdombuilder.h"

#include "dptrtemplate.h"

#include <qapplication.h>

class KBookmarkBarPrivate : public dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate> {
public:
    QPtrList<KAction> m_actions;
    bool m_readOnly;
    KBookmarkManager* m_filteredMgr;
};
template<> QPtrDict<KBookmarkBarPrivate>* dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate>::d_ptr = 0;


KBookmarkBarPrivate* KBookmarkBar::dptr() const {
    return KBookmarkBarPrivate::d( this );
}

// usage of KXBELBookmarkImporterImpl is just plain evil, but it reduces code dup. so...
class ToolbarFilter : public KXBELBookmarkImporterImpl {
public:
    ToolbarFilter() : m_visible(false) { ; }
    void filter( const KBookmarkGroup &grp ) { traverse(grp); }
private:
    virtual void visit( const KBookmark & );
    virtual void visitEnter( const KBookmarkGroup & );
    virtual void visitLeave( const KBookmarkGroup & );
private:
    bool m_visible;
    KBookmarkGroup m_visibleStart;
};

KBookmarkBar::KBookmarkBar( KBookmarkManager* mgr,
                            KBookmarkOwner *_owner, KToolBar *_toolBar,
                            KActionCollection *coll,
                            QObject *parent, const char *name )
    : QObject( parent, name ), m_pOwner(_owner), m_toolBar(_toolBar),
      m_actionCollection( coll ), m_pManager(mgr)
{
    m_lstSubMenus.setAutoDelete( true );

    if ( KBookmarkSettings::self()->m_advanced )
    {
        m_toolBar->setAcceptDrops( true );
        m_toolBar->installEventFilter( this ); // for drops
    }

    dptr()->m_actions.setAutoDelete( true );

    connect( mgr, SIGNAL( changed(const QString &, const QString &) ),
             SLOT( slotBookmarksChanged(const QString &) ) );

    dptr()->m_filteredMgr = 0;
    KBookmarkGroup toolbar = getToolbar();
    dptr()->m_readOnly = !!dptr()->m_filteredMgr;

    fillBookmarkBar( toolbar );
}

KBookmarkGroup KBookmarkBar::getToolbar() {

    if ( KBookmarkSettings::self()->m_filteredtoolbar )
    {
        QString fname = m_pManager->path() + ".ftbcache"; // never actually written to
        if ( !dptr()->m_filteredMgr ) {
            QFile::remove( fname );
            dptr()->m_filteredMgr = KBookmarkManager::managerForFile( fname, false );
        } else {
            KBookmarkGroup bkRoot = dptr()->m_filteredMgr->root();
            QValueList<KBookmark> bks;
            for (KBookmark bm = bkRoot.first(); !bm.isNull(); bm = bkRoot.next(bm)) 
                bks << bm;
            for ( QValueListConstIterator<KBookmark> it = bks.begin(); it != bks.end(); ++it ) 
                bkRoot.deleteBookmark( (*it) );
        }
        ToolbarFilter filter;
        KBookmarkDomBuilder builder( dptr()->m_filteredMgr->root(), 
                                     dptr()->m_filteredMgr );
        builder.connectImporter( &filter );
        filter.filter( m_pManager->root() );
    }

    return dptr()->m_filteredMgr 
         ? dptr()->m_filteredMgr->root() 
         : m_pManager->toolbar();
}

KBookmarkBar::~KBookmarkBar()
{
    clear();
    KBookmarkBarPrivate::delete_d(this);
}

void KBookmarkBar::clear()
{
    QPtrListIterator<KAction> it( dptr()->m_actions );
    for (; it.current(); ++it ) {
        (*it)->unplugAll();
    }
    dptr()->m_actions.clear();
    m_lstSubMenus.clear();
}

void KBookmarkBar::slotBookmarksChanged( const QString & group )
{
    KBookmarkGroup tb = getToolbar(); // heavy for non cached toolbar version
    kdDebug(7043) << "slotBookmarksChanged( " << group << " )" << endl;

    if ( tb.isNull() )
        return;
    if ( tb.address() == group || KBookmarkSettings::self()->m_filteredtoolbar )
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
            action->setProperty( "readOnly", dptr()->m_readOnly );
            action->setDelayed( false );

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

void KBookmarkBar::setReadOnly(bool readOnly)
{
    dptr()->m_readOnly = readOnly;
}

bool KBookmarkBar::isReadOnly() const
{
    return dptr()->m_readOnly;
}

void KBookmarkBar::slotBookmarkSelected()
{
    if (!m_pOwner) return; // this view doesn't handle bookmarks...
    m_pOwner->openBookmarkURL( sender()->property("url").toString() );
}

static KToolBar* s_sepToolBar = 0;
static const int sepId = -9999;

static void removeTempSep()
{
    if (s_sepToolBar) {
        s_sepToolBar->removeItem(sepId);
        s_sepToolBar = 0; // needed?
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
static KAction* handleToolbarDragMoveEvent(QPoint pos, QPtrList<KAction> actions, bool &atFirst, KBookmarkManager *mgr)
{
    static int sepIndex;
    KToolBar *tb = dynamic_cast<KToolBar*>(actions.first()->container(0));
    Q_ASSERT(tb);
    s_sepToolBar = tb;
    s_sepToolBar->removeItemDelayed(sepId);

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
            goto hell;
    }

    a = findPluggedAction(actions, tb, b->id());
    Q_ASSERT(a);
    sepIndex = index + (atFirst ? 0 : 1);

    {//

    QString address = a->property("address").toString();
    KBookmark bk = mgr->findByAddress( address );
    kdDebug() << "popping up " << bk.text() << endl;
    if (bk.isGroup())
    {
        KBookmarkActionMenu *menu = dynamic_cast<KBookmarkActionMenu*>(a);
        Q_ASSERT(menu);
        menu->popup(tb->mapToGlobal(b->geometry().center()));
    } 

    }
    //

hell:
    tb->insertLineSeparator(sepIndex, sepId);
    return a;
}

static KAction* handleToolbarMouseButton(QPoint pos, QPtrList<KAction> actions, 
	                                 KBookmarkManager * /*mgr*/, QPoint & pt)
{
    KToolBar *tb = dynamic_cast<KToolBar*>(actions.first()->container(0));
    Q_ASSERT(tb);

    KToolBarButton *b;
    b = dynamic_cast<KToolBarButton*>(tb->childAt(pos)); 
    if (!b)
        return 0;

    KAction *a = 0;
    a = findPluggedAction(actions, tb, b->id());
    Q_ASSERT(a);
    pt = tb->mapToGlobal(pos);

    return a;
}

// TODO    *** drop improvements ***
// open submenus on drop interactions

// TODO    *** generic rmb improvements ***
// don't *ever* show the rmb on press, always relase, possible???

class KBookmarkBarRMBAssoc : public dPtrTemplate<KBookmarkBar, RMB> { };
template<> QPtrDict<RMB>* dPtrTemplate<KBookmarkBar, RMB>::d_ptr = 0;

static RMB* rmbSelf(KBookmarkBar *m) { return KBookmarkBarRMBAssoc::d(m); }

static QString s_highlightedAddress;

void RMB::begin_rmb_action(KBookmarkBar *self)
{
    RMB *s = rmbSelf(self);
    s->recv = self;
    s->m_parentAddress = QString::null;
    s->s_highlightedAddress = ::s_highlightedAddress;
    s->m_pManager = self->m_pManager;
    s->m_pOwner = self->m_pOwner;
    s->m_parentMenu = 0;
}

void KBookmarkBar::slotRMBActionEditAt( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionEditAt( val ); }

void KBookmarkBar::slotRMBActionProperties( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionProperties( val ); }

void KBookmarkBar::slotRMBActionInsert( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionInsert( val ); }

void KBookmarkBar::slotRMBActionRemove( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionRemove( val ); }

void KBookmarkBar::slotRMBActionCopyLocation( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionCopyLocation( val ); }

void KBookmarkBar::slotRMBActionOpen( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionOpen( val ); }

bool KBookmarkBar::eventFilter( QObject *, QEvent *e )
{
    static bool atFirst = false;
    static KAction* a = 0;

    if (dptr()->m_readOnly)
        return false; // todo: make this limit the actions

    if ( (e->type() == QEvent::MouseButtonRelease) || (e->type() == QEvent::MouseButtonPress) )
    {
        QMouseEvent *mev = (QMouseEvent*)e;

        QPoint pt;
        KAction *_a; 

        _a = handleToolbarMouseButton( mev->pos(), dptr()->m_actions, m_pManager, pt );

        if (_a)
        {
            if (mev->button() == Qt::RightButton)
            {
                s_highlightedAddress = _a->property("address").toString();
                KBookmark bookmark = m_pManager->findByAddress( s_highlightedAddress );
                RMB::begin_rmb_action(this); 
                KPopupMenu *pm = new KPopupMenu;
                rmbSelf(this)->fillContextMenu( pm, s_highlightedAddress, 0 );
                emit aboutToShowContextMenu( rmbSelf(this)->atAddress( s_highlightedAddress ), pm );
                pm->popup( pt );
                mev->accept();
            }

            return !!_a;
        }
    }
    else if ( e->type() == QEvent::DragLeave )
    {
        removeTempSep();
        a = 0;
    }
    else if ( e->type() == QEvent::Drop )
    {
        removeTempSep();
        QDropEvent *dev = (QDropEvent*)e;
        if ( !KBookmarkDrag::canDecode( dev ) )
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
	if (dptr()->m_actions.count() == 0)
	  return false;
        bool _atFirst;
        KAction *_a; 
        _a = handleToolbarDragMoveEvent(dme->pos(), dptr()->m_actions, _atFirst, m_pManager);
        if (_a)
        {
            a = _a;
            atFirst = _atFirst;
            dme->accept();
        }
    }
    return false;
}

static bool showInToolbar( const KBookmark &bk ) {
    return (bk.internalElement().attributes().namedItem("showintoolbar").toAttr().value() == "yes"); 
}

void ToolbarFilter::visit( const KBookmark &bk ) {
    //kdDebug() << "visit(" << bk.text() << ")" << endl;
    if ( m_visible || showInToolbar(bk) )
        KXBELBookmarkImporterImpl::visit(bk);
}

void ToolbarFilter::visitEnter( const KBookmarkGroup &grp ) {
    //kdDebug() << "visitEnter(" << grp.text() << ")" << endl;
    if ( !m_visible && showInToolbar(grp) )
    {
        m_visibleStart = grp;
        m_visible = true;
    }
    if ( m_visible )
        KXBELBookmarkImporterImpl::visitEnter(grp);
}

void ToolbarFilter::visitLeave( const KBookmarkGroup &grp ) {
    //kdDebug() << "visitLeave()" << endl;
    if ( m_visible )
        KXBELBookmarkImporterImpl::visitLeave(grp);
    if ( m_visible && grp.address() == m_visibleStart.address() )
        m_visible = false;
}

#include "kbookmarkbar.moc"
