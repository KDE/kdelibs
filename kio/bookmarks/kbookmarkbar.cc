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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <qregexp.h>
#include <qfile.h>
#include <qevent.h>

#include <kbookmarkbar.h>

#include <kbookmarkmenu.h>
#include <kdebug.h>

#include <ktoolbar.h>

#include <kconfig.h>
#include <kmenu.h>

#include "kbookmarkmenu_p.h"
#include "kbookmarkdombuilder.h"

#include <qapplication.h>

class KBookmarkBarPrivate
{
public:
    QList<KAction *> m_actions;
    KBookmarkManager* m_filteredMgr;
    KToolBar* m_sepToolBar;
    int m_sepIndex;
    bool m_readOnly;
    bool m_atFirst;
    QString m_dropAddress;
    QString m_highlightedAddress;
    RMB* m_rmb;

    KBookmarkBarPrivate() :
        m_filteredMgr( 0 ),
        m_sepToolBar( 0 ),
        m_sepIndex( -1 ),
        m_readOnly( false ),
        m_atFirst( false ),
        m_rmb( 0 )
    {}
};

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
      m_actionCollection( coll ), m_pManager( mgr ), d( new KBookmarkBarPrivate )
{
    m_toolBar->setAcceptDrops( true );
    m_toolBar->installEventFilter( this ); // for drops

    connect( mgr, SIGNAL( changed(const QString &, const QString &) ),
             SLOT( slotBookmarksChanged(const QString &) ) );

    KBookmarkGroup toolbar = getToolbar();
    fillBookmarkBar( toolbar );
}

QString KBookmarkBar::parentAddress()
{
    return d->m_filteredMgr ? QString() : m_pManager->toolbar().address();
}

#define CURRENT_TOOLBAR() ( \
    d->m_filteredMgr ? d->m_filteredMgr->root()  \
                          : m_pManager->toolbar() )

#define CURRENT_MANAGER() ( \
    d->m_filteredMgr ? d->m_filteredMgr  \
                          : m_pManager )

KBookmarkGroup KBookmarkBar::getToolbar()
{
    if ( KBookmarkSettings::self()->m_filteredtoolbar )
    {
        if ( !d->m_filteredMgr ) {
            d->m_filteredMgr = KBookmarkManager::createTempManager();
        } else {
            KBookmarkGroup bkRoot = d->m_filteredMgr->root();
            QList<KBookmark> bks;
            for (KBookmark bm = bkRoot.first(); !bm.isNull(); bm = bkRoot.next(bm))
                bks.append( bm );
            for ( QList<KBookmark>::const_iterator bkit = bks.begin(), bkend = bks.end() ; bkit != bkend ; ++bkit ) {
                bkRoot.deleteBookmark( (*bkit) );
            }
        }
        ToolbarFilter filter;
        KBookmarkDomBuilder builder( d->m_filteredMgr->root(),
                                     d->m_filteredMgr );
        builder.connectImporter( &filter );
        filter.filter( m_pManager->root() );
    }

    return CURRENT_TOOLBAR();
}

KBookmarkBar::~KBookmarkBar()
{
    //clear();
    qDeleteAll( d->m_actions );
    qDeleteAll( m_lstSubMenus );
    delete d->m_rmb;
    delete d;
}

void KBookmarkBar::clear()
{
    m_toolBar->clear();
    qDeleteAll(d->m_actions);
    d->m_actions.clear();
    qDeleteAll( m_lstSubMenus );
    m_lstSubMenus.clear();
}

void KBookmarkBar::slotBookmarksChanged( const QString & group )
{
    KBookmarkGroup tb = getToolbar(); // heavy for non cached toolbar version
    kDebug(7043) << "slotBookmarksChanged( " << group << " )" << endl;

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
        for ( QList<KBookmarkMenu *>::ConstIterator smit = m_lstSubMenus.begin(), smend = m_lstSubMenus.end();
              smit != smend; ++smit )
        {
            (*smit)->slotBookmarksChanged( group );
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
                m_toolBar->addSeparator();
            else
            {
                KAction *action = new KBookmarkAction( text, bm.icon(), 0, m_actionCollection, 0 );
                connect(action, SIGNAL( triggered(bool) ),
                        this, SLOT( slotBookmarkSelected() ));

                action->setProperty( "url", bm.url().url() );
                action->setProperty( "address", bm.address() );

                action->setToolTip( bm.url().pathOrURL() );

                action->plug(m_toolBar);

                d->m_actions.append( action );
            }
        }
        else
        {
            KActionMenu *action = new KBookmarkActionMenu( KIcon(bm.icon()),
                                                           text,
                                                           m_actionCollection,
                                                           "bookmarkbar-actionmenu");
            action->setProperty( "address", bm.address() );
            action->setProperty( "readOnly", d->m_readOnly );
            action->setDelayed( false );

            // this flag doesn't have any UI yet
            KGlobal::config()->setGroup( "Settings" );
            bool addEntriesBookmarkBar = KGlobal::config()->readEntry("AddEntriesBookmarkBar", true);

            KBookmarkMenu *menu = new KBookmarkMenu(CURRENT_MANAGER(), m_pOwner, action->popupMenu(),
                                                    m_actionCollection, false, addEntriesBookmarkBar,
                                                    bm.address());
            connect(menu, SIGNAL( aboutToShowContextMenu(const KBookmark &, QMenu * ) ),
                    this, SIGNAL( aboutToShowContextMenu(const KBookmark &, QMenu * ) ));
            connect(menu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers) ),
                    this, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers) ));
            menu->fillBookmarkMenu();
            action->plug(m_toolBar);
            m_lstSubMenus.append( menu );

            d->m_actions.append( action );
        }
    }
}

void KBookmarkBar::setReadOnly(bool readOnly)
{
    d->m_readOnly = readOnly;
}

bool KBookmarkBar::isReadOnly() const
{
    return d->m_readOnly;
}

void KBookmarkBar::slotBookmarkSelected()
{
    if (!m_pOwner) return; // this view doesn't handle bookmarks...

    if (const KAction* action = qobject_cast<const KAction*>(sender()))
    {
        const QString & url = sender()->property("url").toString();
        m_pOwner->openBookmarkURL(url);
        emit openBookmark( url, QApplication::mouseButtons(), QApplication::keyboardModifiers() );
    }
}

void KBookmarkBar::removeTempSep(KBookmarkBarPrivate* p)
{
    if (p->m_sepToolBar) {
        p->m_sepToolBar->removeAction(m_toolBarSeparator);
        p->m_sepToolBar = 0; // needed?
    }
}

//TODO: kill me
static KAction* findPluggedAction(const QList<KAction *>& actions, KToolBar *tb, int id)
{
    /*for ( QList<KAction *>::const_iterator it = actions.begin(), end = actions.end() ; it != end ; ++it ) {
        if ((*it)->isPlugged(tb, id))
            return (*it);
    }*/
    return 0;
}

/**
 * Handle a QDragMoveEvent event on a toolbar drop
 * @return the address of the bookmark to be dropped after/before
 *         else a QString() if event should be ignored
 * @param pos the current QDragMoveEvent position
 * @param the toolbar
 * @param actions the list of actions plugged into the bar
 * @param atFirst bool reference, when true the position before the
 *        returned action was dropped on
 */
QString KBookmarkBar::handleToolbarDragMoveEvent(
    KBookmarkBarPrivate *p, KToolBar *tb, const QPoint& pos, const QList<KAction *>& actions,
    bool &atFirst, KBookmarkManager *mgr)
{
    Q_UNUSED( mgr );
    Q_ASSERT( actions.isEmpty() || (tb == qobject_cast<KToolBar*>(actions.first()->container(0))) );
    p->m_sepToolBar = tb;
    p->m_sepToolBar->removeAction(m_toolBarSeparator);

    int index = 0;
    QWidget* b = 0;

    for (int i = 0; i < tb->actions().count(); ++i)
      if (QWidget* button = tb->widgetForAction(tb->actions()[i]))
        if (b->geometry().contains(pos)) {
          b = button;
          index = i;
          break;
        }

    QAction *a = 0;
    QString address;
    atFirst = false;

    if (b)
    {
        QRect r = b->geometry();
        if (pos.x() < ((r.left() + r.right())/2))
        {
            // if in first half of button then
            // we jump to previous index
            if ( index == 0 )
                atFirst = true;
            else {
                index--;
                a = tb->actions()[index];
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
        a = tb->actions()[index];
        b = tb->widgetForAction(a);
        // if !b and not past last button, we didn't find button
        if (pos.x() <= b->geometry().left())
            goto skipact; // TODO - rename
    }

    if ( !a )
        return QString(); // TODO Make it works for that case

    address = a->property("address").toString();
    p->m_sepIndex = index + (atFirst ? 0 : 1);

#if 0
    { // ugly workaround to fix the goto scoping problems...
        KBookmark bk = mgr->findByAddress( address );
        if (bk.isGroup()) // TODO - fix this ****!!!, manhatten distance should be used!!!
        {
            kDebug() << "kbookmarkbar:: popping up " << bk.text() << endl;
            KBookmarkActionMenu *menu = dynamic_cast<KBookmarkActionMenu*>(a);
            Q_ASSERT(menu);
            menu->popup(tb->mapToGlobal(b->geometry().center()));
        }
    }
#endif

skipact:
    QAction* before = 0L;
    if (p->m_sepIndex > 0 && p->m_sepIndex < tb->actions().count())
      before = tb->actions().at(p->m_sepIndex - 1);
    m_toolBarSeparator = tb->insertSeparator(before);
    return address;
}

// TODO - document!!!!
static QAction* handleToolbarMouseButton(const QPoint& pos, const QList<KAction *>& actions,
                                         KBookmarkManager * /*mgr*/, QPoint & pt)
{
    if (actions.isEmpty() || !actions.first()) {
        return 0;
    }

    QAction *act = actions.first();

    KToolBar *tb = qobject_cast<KToolBar*>(act->associatedWidgets().first());
    Q_ASSERT(tb);

    QAction* a = 0;

    foreach (QAction* action, tb->actions())
      if (QWidget* button = tb->widgetForAction(action))
        if (button->geometry().contains(pos)) {
          a = action;
          break;
        }

    pt = tb->mapToGlobal(pos);

    return a;
}

// TODO    *** drop improvements ***
// open submenus on drop interactions

// TODO    *** generic rmb improvements ***
// don't *ever* show the rmb on press, always relase, possible???

RMB::RMB(QString parentAddress, QString highlightedAddress,
         KBookmarkManager *pManager, KBookmarkOwner *pOwner)
: m_parentAddress(parentAddress), m_highlightedAddress(highlightedAddress),
  m_pManager(pManager), m_pOwner(pOwner), m_parentMenu(0)
{
}

bool KBookmarkBar::eventFilter( QObject *o, QEvent *e )
{
    if (d->m_readOnly || d->m_filteredMgr) // note, we assume m_pManager in various places,
                                                     // this shouldn't really be the case
        return false; // todo: make this limit the actions

    if ( (e->type() == QEvent::MouseButtonRelease) || (e->type() == QEvent::MouseButtonPress) ) // FIXME, which one?
    {
        QMouseEvent *mev = (QMouseEvent*)e;

        QPoint pt;
        QAction *_a;

        // FIXME, see how this holds up on an empty toolbar
        _a = handleToolbarMouseButton( mev->pos(), d->m_actions, m_pManager, pt );
        if (_a && mev->button() == Qt::RightButton)
        {
            d->m_highlightedAddress = _a->property("address").toString();
            KBookmark bookmark = m_pManager->findByAddress( d->m_highlightedAddress );
            delete d->m_rmb; 
            d->m_rmb = new RMB(parentAddress(), d->m_highlightedAddress, m_pManager, m_pOwner);
            d->m_rmb->fillContextMenu( d->m_highlightedAddress);
            emit aboutToShowContextMenu( d->m_rmb->atAddress( d->m_highlightedAddress ), d->m_rmb->contextMenu() );
            d->m_rmb->fillContextMenu2( d->m_highlightedAddress);
            d->m_rmb->popup( pt );
            mev->accept();
        }

        return !!_a; // ignore the event if we didn't find the button
    }
    else if ( e->type() == QEvent::DragLeave )
    {
        removeTempSep(d);
        d->m_dropAddress.clear();
    }
    else if ( e->type() == QEvent::Drop )
    {
        removeTempSep(d);
        QDropEvent *dev = static_cast<QDropEvent*>( e );
        QList<KBookmark> list = KBookmark::List::fromMimeData( dev->mimeData() );
        if ( list.isEmpty() )
            return false;
        if (list.count() > 1)
            kWarning(7043) << "Sorry, currently you can only drop one address "
                "onto the bookmark bar!" << endl;
        KBookmark toInsert = list.first();
        KBookmark bookmark = m_pManager->findByAddress( d->m_dropAddress );
        Q_ASSERT(!bookmark.isNull());
        kDebug(7043) << "inserting "
            << QString(d->m_atFirst ? "before" : "after")
            << " d->m_dropAddress == " << d->m_dropAddress << endl;
        KBookmarkGroup parentBookmark = bookmark.parentGroup();
        Q_ASSERT(!parentBookmark.isNull());
        KBookmark newBookmark = parentBookmark.addBookmark(
                m_pManager, toInsert.fullText(),
                toInsert.url() );
        parentBookmark.moveItem( newBookmark, d->m_atFirst ? KBookmark() : bookmark );
        m_pManager->emitChanged( parentBookmark );
        return true;
    }
    else if ( e->type() == QEvent::DragMove )
    {
        QDragMoveEvent *dme = static_cast<QDragMoveEvent*>( e );
        if (!KBookmark::List::canDecode( dme->mimeData() ))
            return false;
        bool _atFirst;
        QString dropAddress;
        KToolBar *tb = (KToolBar*)o;
        dropAddress = handleToolbarDragMoveEvent(d, tb, dme->pos(), d->m_actions, _atFirst, m_pManager);
        if (!dropAddress.isNull())
        {
            d->m_dropAddress = dropAddress;
            d->m_atFirst = _atFirst;
            dme->accept();
        }
    }
    return false;
}

static bool showInToolbar( const KBookmark &bk ) {
    return (bk.internalElement().attributes().namedItem("showintoolbar").toAttr().value() == "yes");
}

void ToolbarFilter::visit( const KBookmark &bk ) {
    //kDebug() << "visit(" << bk.text() << ")" << endl;
    if ( m_visible || showInToolbar(bk) )
        KXBELBookmarkImporterImpl::visit(bk);
}

void ToolbarFilter::visitEnter( const KBookmarkGroup &grp ) {
    //kDebug() << "visitEnter(" << grp.text() << ")" << endl;
    if ( !m_visible && showInToolbar(grp) )
    {
        m_visibleStart = grp;
        m_visible = true;
    }
    if ( m_visible )
        KXBELBookmarkImporterImpl::visitEnter(grp);
}

void ToolbarFilter::visitLeave( const KBookmarkGroup &grp ) {
    //kDebug() << "visitLeave()" << endl;
    if ( m_visible )
        KXBELBookmarkImporterImpl::visitLeave(grp);
    if ( m_visible && grp.address() == m_visibleStart.address() )
        m_visible = false;
}

#include "kbookmarkbar.moc"
