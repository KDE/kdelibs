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

#include <kactionmenu.h>
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
    int m_sepIndex;
    bool m_readOnly;
    bool m_atFirst;
    QString m_dropAddress;
    QString m_highlightedAddress;
    RMB* m_rmb;

    KBookmarkBarPrivate() :
        m_filteredMgr( 0 ),
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

    if (KBookmarkSettings::self()->m_contextmenu )
    {
      m_toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(m_toolBar, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
    }

    connect( mgr, SIGNAL( changed(const QString &, const QString &) ),
             SLOT( slotBookmarksChanged(const QString &) ) );

    KBookmarkGroup toolbar = getToolbar();
    fillBookmarkBar( toolbar );
    m_toolBarSeparator = 0;
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

                m_toolBar->addAction(action);

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

            KBookmarkMenu *menu = new KBookmarkMenu(CURRENT_MANAGER(), m_pOwner, action->kMenu(),
                                                    m_actionCollection, false, addEntriesBookmarkBar,
                                                    bm.address());
            connect(menu, SIGNAL( aboutToShowContextMenu(const KBookmark &, QMenu * ) ),
                    this, SIGNAL( aboutToShowContextMenu(const KBookmark &, QMenu * ) ));
            connect(menu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers) ),
                    this, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers) ));
            menu->fillBookmarkMenu();
            m_toolBar->addAction(action);
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

void KBookmarkBar::removeTempSep()
{
    if (m_toolBarSeparator) {
        m_toolBar->removeAction(m_toolBarSeparator);
        m_toolBarSeparator = 0;
    }
}

/**
 * Handle a QDragMoveEvent event on a toolbar drop
 * @return true if the event should be accepted, false if the event should be ignored
 * @param pos the current QDragMoveEvent position
 * @param the toolbar
 * @param actions the list of actions plugged into the bar
 * @param atFirst bool reference, when true the position before the
 *        returned action was dropped on
 */
bool KBookmarkBar::handleToolbarDragMoveEvent(
    const QPoint& pos, const QList<KAction *>& actions,
    bool &atFirst, KBookmarkManager *mgr)
{
    //TODO separators aren't shown if they are the first/last action, 
    // instead insert a dummy action, already with bookmark url
    kDebug()<<"KBookmarkBar::handleToolbarDragMoveEvent "<<pos<<endl;
    Q_UNUSED( mgr );
    Q_ASSERT( actions.isEmpty() || (m_toolBar == qobject_cast<KToolBar*>(actions.first()->container(0))) );
    m_toolBar->setUpdatesEnabled(false);
    if(m_toolBarSeparator)
    {
        m_toolBar->removeAction(m_toolBarSeparator);
        m_toolBarSeparator = 0;
    }

    QWidget* b = 0;

    // Empty toolbar
    if(actions.isEmpty())
    {
        kDebug()<<"actions empty "<<endl;
        atFirst = true;
        d->m_sepIndex = 0;
        m_toolBarSeparator = m_toolBar->addSeparator();
        m_toolBar->setUpdatesEnabled(true);
        return true;
    }

    // else find the toolbar button 
    //TODO contains is wrong, we should only care about .left() and .right() in horizontal bars
    // and about .top() .bottom() for vertical ones
    for (int i = 0; i < m_toolBar->actions().count(); ++i)
      if (QWidget* button = m_toolBar->widgetForAction(m_toolBar->actions()[i]))
        if (button->geometry().contains(pos)) {
          kDebug()<<"button contains pos "<<m_toolBar->actions()[i]->text()<<endl;
          b = button;
          d->m_sepIndex = i;
          break;
        }

    QAction *a = 0;
    QString address;
    atFirst = false;

    if (b) // found the containing button
    {
        QRect r = b->geometry();
        if (pos.x() >= ((r.left() + r.right())/2)) //TODO only works for horizontal toolbars
        {
            kDebug()<<"in second half "<<endl;
            // if in second half of button then
            // we jump to next index
            d->m_sepIndex++;
        }
        if(d->m_sepIndex != actions.count())
        {
            a = m_toolBar->actions()[d->m_sepIndex];
            kDebug()<<"containing widget found, inserting before "<<a->text()<<endl;
            m_toolBarSeparator = m_toolBar->insertSeparator(a);
        }
        else
        {
            kDebug()<<"containing widget found, inserting at the end "<<endl;
            m_toolBarSeparator = m_toolBar->addSeparator();
        }
        m_toolBar->setUpdatesEnabled(true);
        return true;
    }
    else // (!b)
    {
        kDebug()<<"no containing widget found"<<endl;
        a = m_toolBar->actions()[actions.count() - 1];
        b = m_toolBar->widgetForAction(a);
        // if !b and not past last button, we didn't find button
        if (pos.x() <= b->geometry().left()) //TODO only works for horizontal toolbars
        {
            m_toolBar->setUpdatesEnabled(true);
            return false;
        }
        else // location is beyond last action, assuming we want to add in the end
        {
            kDebug()<<" beyond last widget "<<endl;
            d->m_sepIndex = actions.count();
            m_toolBarSeparator = m_toolBar->addSeparator();
            m_toolBar->setUpdatesEnabled(true);
            return true;
        }
    }
}

void KBookmarkBar::contextMenu(const QPoint & pos)
{
    QAction * action = m_toolBar->actionAt(pos);
    if(!action)
        return;
    d->m_highlightedAddress = action->property("address").toString();
    delete d->m_rmb; 
    d->m_rmb = new RMB(parentAddress(), d->m_highlightedAddress, m_pManager, m_pOwner);
    d->m_rmb->fillContextMenu( d->m_highlightedAddress);
    emit aboutToShowContextMenu( d->m_rmb->atAddress( d->m_highlightedAddress ), d->m_rmb->contextMenu() );
    d->m_rmb->fillContextMenu2( d->m_highlightedAddress);
    d->m_rmb->popup( m_toolBar->mapToGlobal(pos) );
}

// TODO    *** drop improvements ***
// open submenus on drop interactions
bool KBookmarkBar::eventFilter( QObject *, QEvent *e )
{
    if (d->m_readOnly || d->m_filteredMgr) // note, we assume m_pManager in various places,
                                                     // this shouldn't really be the case
        return false; // todo: make this limit the actions

    if ( e->type() == QEvent::DragLeave )
    {
        removeTempSep();
        d->m_dropAddress.clear();
    }
    else if ( e->type() == QEvent::Drop )
    {
        removeTempSep();

        QDropEvent *dev = static_cast<QDropEvent*>( e );
        QList<KBookmark> list = KBookmark::List::fromMimeData( dev->mimeData() );
        if ( list.isEmpty() )
            return false;
        if (list.count() > 1)
            kWarning(7043) << "Sorry, currently you can only drop one address "
                "onto the bookmark bar!" << endl;
        KBookmark toInsert = list.first();

        KBookmarkGroup parentBookmark = getToolbar();

        if(d->m_sepIndex == 0)
        {
            KBookmark newBookmark = parentBookmark.addBookmark(
                m_pManager, toInsert.fullText(),
                toInsert.url() );

            parentBookmark.moveItem( newBookmark, KBookmark() );
            m_pManager->emitChanged( parentBookmark );
            return true;
        }
        else
        {
            KBookmark after = parentBookmark.first();

            for(int i=0; i < d->m_sepIndex - 1 ; ++i)
                after = parentBookmark.next(after);
            KBookmark newBookmark = parentBookmark.addBookmark(
                    m_pManager, toInsert.fullText(),
                    toInsert.url() );

            parentBookmark.moveItem( newBookmark, after );
            m_pManager->emitChanged( parentBookmark );
            return true;
        }
    }
    else if ( e->type() == QEvent::DragMove || e->type() == QEvent::DragEnter )
    {
        QDragMoveEvent *dme = static_cast<QDragMoveEvent*>( e );
        if (!KBookmark::List::canDecode( dme->mimeData() ))
            return false;
        bool _atFirst;
        bool accept = handleToolbarDragMoveEvent(dme->pos(), d->m_actions, _atFirst, m_pManager);
        if (accept)
        {
            d->m_atFirst = _atFirst;
            dme->accept();
            return true; //Really?
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
