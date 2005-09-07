//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
// vim: set ts=4 sts=4 sw=4 et:
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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <qregexp.h>
#include <qfile.h>
#include <qevent.h>

#include <kbookmarkbar.h>
#include <kbookmarkdrag.h>

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

class KBookmarkBarPrivate : public dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate>
{
public:
    Q3PtrList<KAction> m_actions;
    bool m_readOnly;
    KBookmarkManager* m_filteredMgr;
    KToolBar* m_sepToolBar;
    int m_sepIndex;
    bool m_atFirst;
    QString m_dropAddress;
    QString m_highlightedAddress;
public:
    KBookmarkBarPrivate() {
        m_readOnly = false;
        m_filteredMgr = 0;
        m_sepToolBar = 0;
        m_sepIndex = -1;
        m_atFirst = false;
    }
};
template<> Q3PtrDict<KBookmarkBarPrivate>* dPtrTemplate<KBookmarkBar, KBookmarkBarPrivate>::d_ptr = 0;

KBookmarkBarPrivate* KBookmarkBar::dptr() const
{
    return KBookmarkBarPrivate::d( this );
}

// usage of KXBELBookmarkImporterImpl is just plain evil, but it reduces code dup. so...
class ToolbarFilter : public KXBELBookmarkImporterImpl
{
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
                            QObject *parent )
    : QObject( parent ), m_pOwner(_owner), m_toolBar(_toolBar),
      m_actionCollection( coll ), m_pManager(mgr)
{
    m_lstSubMenus.setAutoDelete( true );

    m_toolBar->setAcceptDrops( true );
    m_toolBar->installEventFilter( this ); // for drops

    dptr()->m_actions.setAutoDelete( true );

    connect( mgr, SIGNAL( changed(const QString &, const QString &) ),
             SLOT( slotBookmarksChanged(const QString &) ) );

    KBookmarkGroup toolbar = getToolbar();
    fillBookmarkBar( toolbar );
}

QString KBookmarkBar::parentAddress()
{
    return dptr()->m_filteredMgr ? QString::null : m_pManager->toolbar().address();
}

#define CURRENT_TOOLBAR() ( \
    dptr()->m_filteredMgr ? dptr()->m_filteredMgr->root()  \
                          : m_pManager->toolbar() )

#define CURRENT_MANAGER() ( \
    dptr()->m_filteredMgr ? dptr()->m_filteredMgr  \
                          : m_pManager )

KBookmarkGroup KBookmarkBar::getToolbar()
{
    if ( KBookmarkSettings::self()->m_filteredtoolbar )
    {
        if ( !dptr()->m_filteredMgr ) {
            dptr()->m_filteredMgr = KBookmarkManager::createTempManager();
        } else {
            KBookmarkGroup bkRoot = dptr()->m_filteredMgr->root();
            Q3ValueList<KBookmark> bks;
            for (KBookmark bm = bkRoot.first(); !bm.isNull(); bm = bkRoot.next(bm))
                bks << bm;
            for ( Q3ValueListConstIterator<KBookmark> it = bks.begin(); it != bks.end(); ++it )
                bkRoot.deleteBookmark( (*it) );
        }
        ToolbarFilter filter;
        KBookmarkDomBuilder builder( dptr()->m_filteredMgr->root(),
                                     dptr()->m_filteredMgr );
        builder.connectImporter( &filter );
        filter.filter( m_pManager->root() );
    }

    return CURRENT_TOOLBAR();
}

KBookmarkBar::~KBookmarkBar()
{
    //clear();
    KBookmarkBarPrivate::delete_d(this);
}

void KBookmarkBar::clear()
{
    Q3PtrListIterator<KAction> it( dptr()->m_actions );
    m_toolBar->clear();
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

    if ( KBookmark::commonParent(group, tb.address()) == group  // Is group a parent of tb.address?
         || KBookmarkSettings::self()->m_filteredtoolbar )
    {
        clear();
        fillBookmarkBar( tb );
    }
    else
    {
        // Iterate recursively into child menus
        Q3PtrListIterator<KBookmarkMenu> it( m_lstSubMenus );
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
                KAction *action = new KBookmarkAction( text, bm.icon(), 0, m_actionCollection, 0 );
                connect(action, SIGNAL( activated ( KAction::ActivationReason, Qt::ButtonState )),
                        this, SLOT( slotBookmarkSelected( KAction::ActivationReason, Qt::ButtonState ) ));

                action->setProperty( "url", bm.url().url() );
                action->setProperty( "address", bm.address() );

                action->setToolTip( bm.url().pathOrURL() );

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

            KBookmarkMenu *menu = new KBookmarkMenu(CURRENT_MANAGER(), m_pOwner, action->popupMenu(),
                                                    m_actionCollection, false, addEntriesBookmarkBar,
                                                    bm.address());
            connect(menu, SIGNAL( aboutToShowContextMenu(const KBookmark &, Q3PopupMenu * ) ),
                    this, SIGNAL( aboutToShowContextMenu(const KBookmark &, Q3PopupMenu * ) ));
            connect(menu, SIGNAL( openBookmark( const QString &, Qt::ButtonState) ),
                    this, SIGNAL( openBookmark( const QString &, Qt::ButtonState) ));
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

void KBookmarkBar::slotBookmarkSelected( KAction::ActivationReason /*reason*/, Qt::ButtonState state )
{
    if (!m_pOwner) return; // this view doesn't handle bookmarks...

    const KAction* action = dynamic_cast<const KAction *>(sender());
    if(action)
    {
        const QString & url = sender()->property("url").toString();
        m_pOwner->openBookmarkURL(url);
        emit openBookmark( url, state );
    }
}

void KBookmarkBar::slotBookmarkSelected()
{
    slotBookmarkSelected(KAction::ToolBarActivation, Qt::NoButton);
}

static const int const_sepId = -9999; // FIXME this is ugly,
                                      // surely there is another
                                      // way of doing this...

static void removeTempSep(KBookmarkBarPrivate* p)
{
    if (p->m_sepToolBar) {
        p->m_sepToolBar->removeItem(const_sepId);
        p->m_sepToolBar = 0; // needed?
    }
}

static KAction* findPluggedAction(Q3PtrList<KAction> actions, KToolBar *tb, int id)
{
    Q3PtrListIterator<KAction> it( actions );
    for (; (*it); ++it )
        if ((*it)->isPlugged(tb, id))
            return (*it);
    return 0;
}

/**
 * Handle a QDragMoveEvent event on a toolbar drop
 * @return the address of the bookmark to be dropped after/before
 *         else a QString::null if event should be ignored
 * @param pos the current QDragMoveEvent position
 * @param the toolbar
 * @param actions the list of actions plugged into the bar
 * @param atFirst bool reference, when true the position before the
 *        returned action was dropped on
 */
static QString handleToolbarDragMoveEvent(
    KBookmarkBarPrivate *p, KToolBar *tb, QPoint pos, Q3PtrList<KAction> actions,
    bool &atFirst, KBookmarkManager *mgr
) {
    Q_UNUSED( mgr );
    Q_ASSERT( actions.isEmpty() || (tb == dynamic_cast<KToolBar*>(actions.first()->container(0))) );
    p->m_sepToolBar = tb;
    p->m_sepToolBar->removeItemDelayed(const_sepId);

    int index;
    KToolBarButton* b;

    b = dynamic_cast<KToolBarButton*>(tb->childAt(pos));
    KAction *a = 0;
    QString address;
    atFirst = false;

    if (b)
    {
        index = tb->itemIndex(b->id());
        QRect r = b->geometry();
        if (pos.x() < ((r.left() + r.right())/2))
        {
            // if in first half of button then
            // we jump to previous index
            if ( index == 0 )
                atFirst = true;
            else {
                index--;
                b = tb->getButton(tb->idAt(index));
            }
        }
    }
    else if (actions.isEmpty())
    {
        atFirst = true;
        index = 0;
        // we skip the action related stuff
        // and do what it should have...
        // FIXME - here we want to get the
        // parent address of the bookmark
        // bar itself and return that + "/0"
        p->m_sepIndex = 0;
        goto skipact;
    }
    else // (!b)
    {
        index = actions.count() - 1;
        b = tb->getButton(tb->idAt(index));
        // if !b and not past last button, we didn't find button
        if (pos.x() <= b->geometry().left())
            goto skipact; // TODO - rename
    }

    if ( !b )
        return QString::null; // TODO Make it works for that case

    a = findPluggedAction(actions, tb, b->id());
    Q_ASSERT(a);
    address = a->property("address").toString();
    p->m_sepIndex = index + (atFirst ? 0 : 1);

#if 0
    { // ugly workaround to fix the goto scoping problems...
        KBookmark bk = mgr->findByAddress( address );
        if (bk.isGroup()) // TODO - fix this ****!!!, manhatten distance should be used!!!
        {
            kdDebug() << "kbookmarkbar:: popping up " << bk.text() << endl;
            KBookmarkActionMenu *menu = dynamic_cast<KBookmarkActionMenu*>(a);
            Q_ASSERT(menu);
            menu->popup(tb->mapToGlobal(b->geometry().center()));
        }
    }
#endif

skipact:
    tb->insertLineSeparator(p->m_sepIndex, const_sepId);
    return address;
}

// TODO - document!!!!
static KAction* handleToolbarMouseButton(QPoint pos, Q3PtrList<KAction> actions,
	                                     KBookmarkManager * /*mgr*/, QPoint & pt)
{
    KAction *act = actions.first();
    if (!act) {
        return 0;
    }

    KToolBar *tb = dynamic_cast<KToolBar*>(act->container(0));
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
template<> Q3PtrDict<RMB>* dPtrTemplate<KBookmarkBar, RMB>::d_ptr = 0;

static RMB* rmbSelf(KBookmarkBar *m) { return KBookmarkBarRMBAssoc::d(m); }

void RMB::begin_rmb_action(KBookmarkBar *self)
{
    RMB *s = rmbSelf(self);
    s->recv = self;
    s->m_parentAddress = self->parentAddress();
    s->s_highlightedAddress = self->dptr()->m_highlightedAddress; // rename in RMB
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

bool KBookmarkBar::eventFilter( QObject *o, QEvent *e )
{
    if (dptr()->m_readOnly || dptr()->m_filteredMgr) // note, we assume m_pManager in various places,
                                                     // this shouldn't really be the case
        return false; // todo: make this limit the actions

    if ( (e->type() == QEvent::MouseButtonRelease) || (e->type() == QEvent::MouseButtonPress) ) // FIXME, which one?
    {
        QMouseEvent *mev = (QMouseEvent*)e;

        QPoint pt;
        KAction *_a;

        // FIXME, see how this holds up on an empty toolbar
        _a = handleToolbarMouseButton( mev->pos(), dptr()->m_actions, m_pManager, pt );
        if (_a && mev->button() == Qt::RightButton)
        {
            dptr()->m_highlightedAddress = _a->property("address").toString();
            KBookmark bookmark = m_pManager->findByAddress( dptr()->m_highlightedAddress );
            RMB::begin_rmb_action(this);
            KPopupMenu *pm = new KPopupMenu;
            rmbSelf(this)->fillContextMenu( pm, dptr()->m_highlightedAddress, 0 );
            emit aboutToShowContextMenu( rmbSelf(this)->atAddress( dptr()->m_highlightedAddress ), pm );
            rmbSelf(this)->fillContextMenu2( pm, dptr()->m_highlightedAddress, 0 );
            pm->popup( pt );
            mev->accept();
        }

        return !!_a; // ignore the event if we didn't find the button
    }
    else if ( e->type() == QEvent::DragLeave )
    {
        removeTempSep(dptr());
        dptr()->m_dropAddress = QString::null;
    }
    else if ( e->type() == QEvent::Drop )
    {
        removeTempSep(dptr());
        QDropEvent *dev = (QDropEvent*)e;
        if ( !KBookmarkDrag::canDecode( dev ) )
            return false;
        Q3ValueList<KBookmark> list = KBookmarkDrag::decode( dev );
        if (list.count() > 1)
            kdWarning(7043) << "Sorry, currently you can only drop one address "
                "onto the bookmark bar!" << endl;
        KBookmark toInsert = list.first();
        KBookmark bookmark = m_pManager->findByAddress( dptr()->m_dropAddress );
        Q_ASSERT(!bookmark.isNull());
        kdDebug(7043) << "inserting "
            << QString(dptr()->m_atFirst ? "before" : "after")
            << " dptr()->m_dropAddress == " << dptr()->m_dropAddress << endl;
        KBookmarkGroup parentBookmark = bookmark.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark(
                m_pManager, toInsert.fullText(),
                toInsert.url() );
        parentBookmark.moveItem( newBookmark, dptr()->m_atFirst ? KBookmark() : bookmark );
        m_pManager->emitChanged( parentBookmark );
        return true;
    }
    else if ( e->type() == QEvent::DragMove )
    {
        QDragMoveEvent *dme = (QDragMoveEvent*)e;
        if (!KBookmarkDrag::canDecode( dme ))
            return false;
        bool _atFirst;
        QString dropAddress;
        KToolBar *tb = (KToolBar*)o;
        dropAddress = handleToolbarDragMoveEvent(dptr(), tb, dme->pos(), dptr()->m_actions, _atFirst, m_pManager);
        if (!dropAddress.isNull())
        {
            dptr()->m_dropAddress = dropAddress;
            dptr()->m_atFirst = _atFirst;
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
