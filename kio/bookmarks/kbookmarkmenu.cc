//  -*- c-basic-offset:2; indent-tabs-mode:nil -*-
// vim: set ts=2 sts=2 sw=2 et:
/* This file is part of the KDE project
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

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"

#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstringhandler.h>

#include <qclipboard.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qmimedata.h>

#include <q3header.h>
#include <q3listview.h>
#include <QStack>
#include <dptrtemplate.h>

static QString makeTextNodeMod(KBookmark bk, const QString &m_nodename, const QString &m_newText) {
  QDomNode subnode = bk.internalElement().namedItem(m_nodename);
  if (subnode.isNull()) {
    subnode = bk.internalElement().ownerDocument().createElement(m_nodename);
    bk.internalElement().appendChild(subnode);
  }

  if (subnode.firstChild().isNull()) {
    QDomText domtext = subnode.ownerDocument().createTextNode("");
    subnode.appendChild(domtext);
  }

  QDomText domtext = subnode.firstChild().toText();

  QString m_oldText = domtext.data();
  domtext.setData(m_newText);

  return m_oldText;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, KMenu * _parentMenu,
                              KActionCollection *collec, bool _isRoot, bool _add,
                              const QString & parentAddress )
  : QObject(),
    m_bIsRoot(_isRoot), m_bAddBookmark(_add),
    m_bAddShortcuts(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_actionCollection( collec ),
    m_parentAddress( parentAddress )
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  if (m_actionCollection)
  {
    m_actionCollection->setHighlightingEnabled(true);
    disconnect( m_actionCollection, SIGNAL( actionHighlighted( KAction * ) ), 0, 0 );
    connect( m_actionCollection, SIGNAL( actionHighlighted( KAction * ) ),
             this, SLOT( slotActionHighlighted( KAction * ) ) );
  }

  m_bNSBookmark = m_parentAddress.isNull();
  if ( !m_bNSBookmark ) // not for the netscape bookmark
  {
    //kdDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress << endl;

    connect( _parentMenu, SIGNAL( aboutToShow() ),
             SLOT( slotAboutToShow() ) );

    if ( KBookmarkSettings::self()->m_contextmenu )
    {
      (void) _parentMenu->contextMenu();
      connect( _parentMenu, SIGNAL( aboutToShowContextMenu(KMenu*, QAction*, QMenu*) ),
               this, SLOT( slotAboutToShowContextMenu(KMenu*, QAction*, QMenu*) ));
    }

    if ( m_bIsRoot )
    {
      connect( m_pManager, SIGNAL( changed(const QString &, const QString &) ),
               SLOT( slotBookmarksChanged(const QString &) ) );
    }
  }

  // add entries that possibly have a shortcut, so they are available _before_ first popup
  if ( m_bIsRoot )
  {
    if ( m_bAddBookmark )
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
    }

    addEditBookmarks();
  }

  m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
  //kdDebug(7043) << "KBookmarkMenu::~KBookmarkMenu() " << this << endl;
  QList<KAction *>::Iterator it = m_actions.begin();
  for (; it != m_actions.end(); ++it )
    (*it)->unplugAll();

  qDeleteAll( m_lstSubMenus );
  qDeleteAll( m_actions );
}

void KBookmarkMenu::ensureUpToDate()
{
  slotAboutToShow();
}

void KBookmarkMenu::slotAboutToShow()
{
  // Did the bookmarks change since the last time we showed them ?
  if ( m_bDirty )
  {
    m_bDirty = false;
    refill();
  }
}

QString KBookmarkMenu::s_highlightedAddress;
QString KBookmarkMenu::s_highlightedImportType;
QString KBookmarkMenu::s_highlightedImportLocation;

void KBookmarkMenu::slotActionHighlighted( KAction* action )
{
  if (action->isA("KBookmarkActionMenu") || action->isA("KBookmarkAction"))
  {
    s_highlightedAddress = action->property("address").toString();
    //kdDebug() << "KBookmarkMenu::slotActionHighlighted" << s_highlightedAddress << endl;
  }
  else if (action->isA("KImportedBookmarksActionMenu"))
  {
    s_highlightedImportType = action->property("type").toString();
    s_highlightedImportLocation = action->property("location").toString();
  }
  else
  {
    s_highlightedAddress.clear();
    s_highlightedImportType.clear();
    s_highlightedImportLocation.clear();
  }
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// TODO use d pointer instead - or add a pointer for the RMB instance.

class KBookmarkMenuRMBAssoc : public dPtrTemplate<KBookmarkMenu, RMB> { };
template<> Q3PtrDict<RMB>* dPtrTemplate<KBookmarkMenu, RMB>::d_ptr = 0;

static RMB* rmbSelf(KBookmarkMenu *m) { return KBookmarkMenuRMBAssoc::d(m); }

// TODO check via dcop before making any changes to the bookmarks file???

void RMB::begin_rmb_action(KBookmarkMenu *self)
{
  RMB *s = rmbSelf(self);
  s->recv = self;
  s->m_parentAddress = self->m_parentAddress;
  s->s_highlightedAddress = KBookmarkMenu::s_highlightedAddress;
  s->m_pManager = self->m_pManager;
  s->m_pOwner = self->m_pOwner;
  s->m_parentMenu = self->m_parentMenu;
}

bool RMB::invalid( int val )
{
  bool valid = true;

  if (val == 1)
    s_highlightedAddress = m_parentAddress;

  if (s_highlightedAddress.isNull())
    valid = false;

  return !valid;
}

KBookmark RMB::atAddress(const QString & address)
{
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());
  return bookmark;
}

void KBookmarkMenu::slotAboutToShowContextMenu( KMenu*, QAction*, QMenu* contextMenu )
{
  //kdDebug(7043) << "KBookmarkMenu::slotAboutToShowContextMenu" << s_highlightedAddress << endl;
  if (s_highlightedAddress.isNull())
  {
    KMenu::contextMenuFocus()->hideContextMenu();
    return;
  }
  contextMenu->clear();
  fillContextMenu( contextMenu, s_highlightedAddress, 0 );
}

void RMB::fillContextMenu( QMenu* contextMenu, const QString & address, int val )
{
  KBookmark bookmark = atAddress(address);

  int id;

  // binner:
  // "Add Bookmark Here" when pointing at a bookmark looks strange and if you
  // call it you have to close and reopen the menu to see an entry was added?
  //
  // TODO rename these, but, message freeze... umm...

//  if (bookmark.isGroup()) {
    id = contextMenu->insertItem( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), recv, SLOT(slotRMBActionInsert(int)) );
    contextMenu->setItemParameter( id, val );
/*  }
  else
  {
    id = contextMenu->insertItem( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), recv, SLOT(slotRMBActionInsert(int)) );
    contextMenu->setItemParameter( id, val );
  }*/
}

void RMB::fillContextMenu2( QMenu* contextMenu, const QString & address, int val )
{
  KBookmark bookmark = atAddress(address);

  int id;

  if (bookmark.isGroup()) {
    id = contextMenu->insertItem( i18n( "Open Folder in Bookmark Editor" ), recv, SLOT(slotRMBActionEditAt(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( SmallIcon("editdelete"), i18n( "Delete Folder" ), recv, SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( i18n( "Properties" ), recv, SLOT(slotRMBActionProperties(int)) );
    contextMenu->setItemParameter( id, val );
  }
  else
  {
    id = contextMenu->insertItem( i18n( "Copy Link Address" ), recv, SLOT(slotRMBActionCopyLocation(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( SmallIcon("editdelete"), i18n( "Delete Bookmark" ), recv, SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( i18n( "Properties" ), recv, SLOT(slotRMBActionProperties(int)) );
    contextMenu->setItemParameter( id, val );
  }
}

void RMB::slotRMBActionEditAt( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionEditAt" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress(s_highlightedAddress);

  m_pManager->slotEditBookmarksAtAddress( s_highlightedAddress );
}

void RMB::slotRMBActionProperties( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionProperties" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress(s_highlightedAddress);

  QString folder = bookmark.isGroup() ? QString() : bookmark.url().pathOrURL();
  KBookmarkEditDialog dlg( bookmark.fullText(), folder,
                           m_pManager, KBookmarkEditDialog::ModifyMode, 0,
                           0, 0, i18n("Bookmark Properties") );
  if ( dlg.exec() != KDialogBase::Accepted )
    return;

  makeTextNodeMod(bookmark, "title", dlg.finalTitle());
  if ( !dlg.finalUrl().isNull() )
  {
    KURL u = KURL::fromPathOrURL(dlg.finalUrl());
    bookmark.internalElement().setAttribute("href", u.url(0, 106));
  }

#ifdef __GNUC__
#warning "Here dlg.finalAddress crash because when we modify a bookmark m_foldertree is null ! Bug ?"
#endif
  //kdDebug(7043) << "Requested move to " << dlg.finalAddress() << "!" << endl;

  KBookmarkGroup parentBookmark = atAddress(m_parentAddress).toGroup();
  m_pManager->emitChanged( parentBookmark );
}

void RMB::slotRMBActionInsert( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionInsert" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  QString url = m_pOwner->currentURL();
  if (url.isEmpty())
  {
    KMessageBox::error( 0L, i18n("Cannot add bookmark with empty URL."));
    return;
  }
  QString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  KBookmark bookmark = atAddress( s_highlightedAddress );

  // TODO use unique title

  if (bookmark.isGroup())
  {
    KBookmarkGroup parentBookmark = bookmark.toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    parentBookmark.addBookmark( m_pManager, title, KURL(  url ) );
    m_pManager->emitChanged( parentBookmark );
  }
  else
  {
    KBookmarkGroup parentBookmark = bookmark.parentGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmark newBookmark = parentBookmark.addBookmark( m_pManager, title, KURL( url ) );
    parentBookmark.moveItem( newBookmark, parentBookmark.previous(bookmark) );
    m_pManager->emitChanged( parentBookmark );
  }
}

void RMB::slotRMBActionRemove( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionRemove" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress( s_highlightedAddress );
  bool folder = bookmark.isGroup();

  if (KMessageBox::warningContinueCancel(
          m_parentMenu,
          folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?").arg(bookmark.text())
                 : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?").arg(bookmark.text()),
          folder ? i18n("Bookmark Folder Deletion")
                 : i18n("Bookmark Deletion"),
          KStdGuiItem::del())
        != KMessageBox::Continue
     )
    return;

  KBookmarkGroup parentBookmark = atAddress( m_parentAddress ).toGroup();
  parentBookmark.deleteBookmark( bookmark );
  m_pManager->emitChanged( parentBookmark );
  if (m_parentMenu)
    m_parentMenu->hide();
}

void RMB::slotRMBActionCopyLocation( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionCopyLocation" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  const KBookmark bookmark = atAddress( s_highlightedAddress );

  if ( !bookmark.isGroup() )
  {
    QMimeData* mimeData = new QMimeData;
    bookmark.populateMimeData( mimeData );
    QApplication::clipboard()->setMimeData( mimeData, QClipboard::Selection );
    mimeData = new QMimeData;
    bookmark.populateMimeData( mimeData );
    QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
  }
}

void RMB::hidePopup() {
  KMenu::contextMenuFocus()->hideContextMenu();
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::fillContextMenu( QMenu* contextMenu, const QString & address, int val )
{
  RMB::begin_rmb_action(this);
  rmbSelf(this)->fillContextMenu(contextMenu, address, val);
  emit aboutToShowContextMenu( rmbSelf(this)->atAddress(address), contextMenu);
  rmbSelf(this)->fillContextMenu2(contextMenu, address, val);
}

void KBookmarkMenu::slotRMBActionEditAt( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionEditAt( val ); }

void KBookmarkMenu::slotRMBActionProperties( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionProperties( val ); }

void KBookmarkMenu::slotRMBActionInsert( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionInsert( val ); }

void KBookmarkMenu::slotRMBActionRemove( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionRemove( val ); }

void KBookmarkMenu::slotRMBActionCopyLocation( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionCopyLocation( val ); }

void KBookmarkMenu::slotBookmarksChanged( const QString & groupAddress )
{
  if (m_bNSBookmark)
    return;

  if ( groupAddress == m_parentAddress )
  {
    //kdDebug(7043) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress << endl;
    m_bDirty = true;
  }
  else
  {
    // Iterate recursively into child menus

    for ( QList<KBookmarkMenu *>::iterator it = m_lstSubMenus.begin(), end = m_lstSubMenus.end() ;
          it != end ; ++it ) {
      (*it)->slotBookmarksChanged( groupAddress );
    }
  }
}

void KBookmarkMenu::refill()
{
  //kdDebug(7043) << "KBookmarkMenu::refill()" << endl;
  qDeleteAll( m_lstSubMenus );
  m_lstSubMenus.clear();

  for ( QList<KAction *>::iterator it = m_actions.begin(), end = m_actions.end() ;
        it != end ; ++it ) {
    (*it)->unplug( m_parentMenu );
  }

  m_parentMenu->clear();
  m_actions.clear();

  fillBookmarkMenu();
  m_parentMenu->adjustSize();
}

void KBookmarkMenu::addAddBookmarksList()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  QString title = i18n( "Bookmark Tabs as Folder..." );

  KAction * paAddBookmarksList = new KAction( title,
                                          "bookmarks_list_add",
                                          0,
                                          this,
                                          SLOT( slotAddBookmarksList() ),
                                          m_actionCollection, m_bIsRoot ? "add_bookmarks_list" : 0 );

  paAddBookmarksList->setToolTip( i18n( "Add a folder of bookmarks for all open tabs." ) );

  paAddBookmarksList->plug( m_parentMenu );
  m_actions.append( paAddBookmarksList );
}

void KBookmarkMenu::addAddBookmark()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  QString title = i18n( "Add Bookmark" );

  KAction * paAddBookmarks = new KAction( title,
                                          "bookmark_add",
                                          m_bIsRoot && m_bAddShortcuts ? KStdAccel::addBookmark() : KShortcut(),
                                          this,
                                          SLOT( slotAddBookmark() ),
                                          m_actionCollection, m_bIsRoot ? "add_bookmark" : 0 );

  paAddBookmarks->setToolTip( i18n( "Add a bookmark for the current document" ) );

  paAddBookmarks->plug( m_parentMenu );
  m_actions.append( paAddBookmarks );
}

void KBookmarkMenu::addEditBookmarks()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  KAction * m_paEditBookmarks = KStdAction::editBookmarks( m_pManager, SLOT( slotEditBookmarks() ),
                                                             m_actionCollection, "edit_bookmarks" );
  m_paEditBookmarks->plug( m_parentMenu );
  m_paEditBookmarks->setToolTip( i18n( "Edit your bookmark collection in a separate window" ) );
  m_actions.append( m_paEditBookmarks );
}

void KBookmarkMenu::addNewFolder()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  QString title = i18n( "&New Bookmark Folder..." );
  int p;
  while ( ( p = title.find( '&' ) ) >= 0 )
    title.remove( p, 1 );

  KAction * paNewFolder = new KAction( title,
                                       "folder_new", //"folder",
                                       0,
                                       this,
                                       SLOT( slotNewFolder() ),
                                       m_actionCollection,"dummyname" );

  paNewFolder->setToolTip( i18n( "Create a new bookmark folder in this menu" ) );

  paNewFolder->plug( m_parentMenu );
  m_actions.append( paNewFolder );
}

void KBookmarkMenu::fillBookmarkMenu()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  if ( m_bIsRoot )
  {
    if ( m_bAddBookmark )
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
    }

    addEditBookmarks();

    if ( m_bAddBookmark && !KBookmarkSettings::self()->m_advancedaddbookmark )
      addNewFolder();
  }

  if ( m_bIsRoot
    && KBookmarkManager::userBookmarksManager()->path() == m_pManager->path() )
  {
    bool haveSep = false;

    const QStringList keys = KBookmarkMenu::dynamicBookmarksList();
    for ( QStringList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
       DynMenuInfo info;
       info = showDynamicBookmarks((*it));

       if ( !info.show || !QFile::exists( info.location ) )
          continue;

       if (!haveSep)
       {
          m_parentMenu->insertSeparator();
          haveSep = true;
       }

       KActionMenu * actionMenu;
       actionMenu = new KImportedBookmarksActionMenu(
                              info.name, info.type,
                              m_actionCollection, "kbookmarkmenu" );

       actionMenu->setProperty( "type", info.type );
       actionMenu->setProperty( "location", info.location );

       actionMenu->plug( m_parentMenu );
       m_actions.append( actionMenu );

       KBookmarkMenu *subMenu =
          new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                             m_actionCollection, false,
                             m_bAddBookmark, QString() );
       connect( subMenu, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ),
                this, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ));
       m_lstSubMenus.append( subMenu );

       connect(actionMenu->popupMenu(), SIGNAL(aboutToShow()), subMenu, SLOT(slotNSLoad()));
    }
  }

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  bool separatorInserted = false;
  for ( KBookmark bm = parentBookmark.first(); !bm.isNull();  bm = parentBookmark.next(bm) )
  {
    QString text = KStringHandler::csqueeze(bm.fullText(), 60);
    text.replace( '&', "&&" );
    if ( !separatorInserted && m_bIsRoot) {
      // inserted before the first konq bookmark, to avoid the separator if no konq bookmark
      m_parentMenu->insertSeparator();
      separatorInserted = true;
    }
    if ( !bm.isGroup() )
    {
      if ( bm.isSeparator() )
      {
        m_parentMenu->insertSeparator();
      }
      else
      {
        //kdDebug(7043) << "Creating URL bookmark menu item for " << bm.text() << endl;
        KAction * action = new KBookmarkAction( text, bm.icon(), 0, m_actionCollection, 0 );
        connect(action, SIGNAL( activated ( KAction::ActivationReason, Qt::ButtonState )),
                this, SLOT( slotBookmarkSelected( KAction::ActivationReason, Qt::ButtonState ) ));

        action->setProperty( "url", bm.url().url() );
        action->setProperty( "address", bm.address() );

        action->setToolTip( bm.url().pathOrURL() );

        action->plug( m_parentMenu );
        m_actions.append( action );
      }
    }
    else
    {
      //kdDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
      KActionMenu * actionMenu = new KBookmarkActionMenu( text, bm.icon(),
                                                          m_actionCollection,
                                                          "kbookmarkmenu" );
      actionMenu->setProperty( "address", bm.address() );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );

      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark,
                                                  bm.address() );

      connect(subMenu, SIGNAL( aboutToShowContextMenu( const KBookmark &, QMenu * ) ),
                 this, SIGNAL( aboutToShowContextMenu( const KBookmark &, QMenu * ) ));
      connect(subMenu, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ),
                this, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ));
      m_lstSubMenus.append( subMenu );
    }
  }

  if ( !m_bIsRoot && m_bAddBookmark )
  {
    if ( m_parentMenu->count() > 0 )
      m_parentMenu->insertSeparator();

    if ( KBookmarkSettings::self()->m_quickactions )
    {
      KActionMenu * actionMenu = new KActionMenu( i18n("Quick Actions"), m_actionCollection, 0L );
      fillContextMenu( actionMenu->popupMenu(), m_parentAddress, 1 );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );
    }
    else
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
      addNewFolder();
    }
  }
}

void KBookmarkMenu::slotAddBookmarksList()
{
  KExtendedBookmarkOwner *extOwner = dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
  if (!extOwner)
  {
    kdWarning() << "erm, sorry ;-)" << endl;
    return;
  }

  KExtendedBookmarkOwner::QStringPairList list;
  extOwner->fillBookmarksList( list );

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_pManager );
  if ( group.isNull() )
    return; // user canceled i guess

  KExtendedBookmarkOwner::QStringPairList::const_iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    group.addBookmark( m_pManager, (*it).first, KURL((*it).second) );

  m_pManager->emitChanged( parentBookmark );
}


void KBookmarkMenu::slotAddBookmark()
{
  KBookmarkGroup parentBookmark;
  parentBookmark = m_pManager->addBookmarkDialog(m_pOwner->currentURL(), m_pOwner->currentTitle(), m_parentAddress);
  if (!parentBookmark.isNull())
    m_pManager->emitChanged( parentBookmark );
}

void KBookmarkMenu::slotNewFolder()
{
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_pManager );
  if ( !group.isNull() )
  {
    KBookmarkGroup parentGroup = group.parentGroup();
    m_pManager->emitChanged( parentGroup );
  }
}

void KBookmarkMenu::slotBookmarkSelected( KAction::ActivationReason /*reason*/, Qt::ButtonState state )
{
  kdDebug(7043) << "KBookmarkMenu::slotBookmarkSelected()" << endl;
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  const KAction* action =  dynamic_cast<const KAction *>(sender());
  if(action)
  {
      const QString& url = sender()->property("url").toString();
      m_pOwner->openBookmarkURL( url );
      emit openBookmark( url, state );
  }
}

void KBookmarkMenu::slotBookmarkSelected()
{
    slotBookmarkSelected(KAction::PopupMenuActivation, Qt::NoButton);
}

KExtendedBookmarkOwner* KBookmarkMenu::extOwner()
{
  return dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
}

void KBookmarkMenu::slotNSLoad()
{
  // only fill menu once
  m_parentMenu->disconnect(SIGNAL(aboutToShow()));

  // not NSImporter, but kept old name for BC reasons
  KBookmarkMenuNSImporter importer( m_pManager, this, m_actionCollection );
  importer.openBookmarks(s_highlightedImportLocation, s_highlightedImportType);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkEditFields::KBookmarkEditFields(QWidget *main, QBoxLayout *vbox, FieldsSet fieldsSet)
{
  bool isF = (fieldsSet != FolderFieldsSet);

  QGridLayout *grid = new QGridLayout( vbox, 2, isF ? 2 : 1 );

  m_title = new KLineEdit( main );
  grid->addWidget( m_title, 0, 1 );
  grid->addWidget( new QLabel( m_title, i18n( "Name:" ), main ), 0, 0 );
  m_title->setFocus();
  if (isF)
  {
    m_url = new KLineEdit( main );
    grid->addWidget( m_url, 1, 1 );
    grid->addWidget( new QLabel( m_url, i18n( "Location:" ), main ), 1, 0 );
  }
  else
  {
    m_url = 0;
  }

  main->setMinimumSize( 300, 0 );
}

void KBookmarkEditFields::setName(const QString &str)
{
  m_title->setText(str);
}

void KBookmarkEditFields::setLocation(const QString &str)
{
  m_url->setText(str);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// TODO - make the dialog use Properties as a title when in Modify mode... (dirk noticed the bug...)
KBookmarkEditDialog::KBookmarkEditDialog(const QString& title, const QString& url, KBookmarkManager * mgr, BookmarkEditType editType, const QString& address,
                                         QWidget * parent, const char * name, const QString& caption )
  : KDialogBase(parent, name, true, caption,
                (editType == InsertionMode) ? (User1|Ok|Cancel) : (Ok|Cancel),
                Ok, false, KGuiItem()),
    m_folderTree(0), m_mgr(mgr), m_editType(editType), m_address(address)
{
  setButtonGuiItem(KDialogBase::Ok, (editType == InsertionMode) ? KGuiItem( i18n( "&Add" ), "bookmark_add") : i18n( "&Update" ) );
  if (editType == InsertionMode) {
    setButtonGuiItem( User1, KGuiItem( i18n( "&New Folder..." ), "folder_new") );
  }

  bool folder = url.isNull();

  m_main = new QWidget( this );
  setMainWidget( m_main );

  QBoxLayout *vbox = new QVBoxLayout( m_main, 0, spacingHint() );
  KBookmarkEditFields::FieldsSet fs =
    folder ? KBookmarkEditFields::FolderFieldsSet
           : KBookmarkEditFields::BookmarkFieldsSet;
  m_fields = new KBookmarkEditFields(m_main, vbox, fs);
  m_fields->setName(title);
  if ( !folder )
    m_fields->setLocation(url);

  if ( editType == InsertionMode )
  {
    m_folderTree = KBookmarkFolderTree::createTree( m_mgr, m_main, name, m_address );
    connect( m_folderTree, SIGNAL( doubleClicked(Q3ListViewItem*) ),
             this,         SLOT( slotDoubleClicked(Q3ListViewItem*) ) );
    vbox->addWidget( m_folderTree );
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotUser1() ) );
  }
}

void KBookmarkEditDialog::slotDoubleClicked( Q3ListViewItem* item )
{
  Q_ASSERT( m_folderTree );
  m_folderTree->setCurrentItem( item );
  accept();
}

void KBookmarkEditDialog::slotOk()
{
  accept();
}

void KBookmarkEditDialog::slotCancel()
{
  reject();
}

QString KBookmarkEditDialog::finalAddress() const
{
  Q_ASSERT( m_folderTree );
  return KBookmarkFolderTree::selectedAddress( m_folderTree );
}

QString KBookmarkEditDialog::finalUrl() const
{
  return m_fields->m_url ? m_fields->m_url->text() : QString();
}

QString KBookmarkEditDialog::finalTitle() const
{
  return m_fields->m_title ? m_fields->m_title->text() : QString();
}

void KBookmarkEditDialog::slotUser1()
{
  // kdDebug(7043) << "KBookmarkEditDialog::slotUser1" << endl;
  Q_ASSERT( m_folderTree );

  QString address = KBookmarkFolderTree::selectedAddress( m_folderTree );
  if ( address.isNull() ) return;
  KBookmarkGroup bm = m_mgr->findByAddress( address ).toGroup();
  Q_ASSERT(!bm.isNull());
  Q_ASSERT(m_editType == InsertionMode);

  KBookmarkGroup group = bm.createNewFolder( m_mgr );
  if ( !group.isNull() )
  {
    KBookmarkGroup parentGroup = group.parentGroup();
    m_mgr->emitChanged( parentGroup );
  }
  KBookmarkFolderTree::fillTree( m_folderTree, m_mgr );
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

static void fillGroup( Q3ListView* listview, KBookmarkFolderTreeItem * parentItem, KBookmarkGroup group, bool expandOpenGroups = true, const QString& address = QString() )
{
  bool noSubGroups = true;
  KBookmarkFolderTreeItem * lastItem = 0L;
  KBookmarkFolderTreeItem * item = 0L;
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    if ( bk.isGroup() )
    {
      KBookmarkGroup grp = bk.toGroup();
      item = new KBookmarkFolderTreeItem( parentItem, lastItem, grp );
      fillGroup( listview, item, grp, expandOpenGroups, address );
      if ( expandOpenGroups && grp.isOpen() )
        item->setOpen( true );
      lastItem = item;
      noSubGroups = false;
    }
    if (bk.address() == address) {
      listview->setCurrentItem( lastItem );
      listview->ensureItemVisible( item );
    }
  }
  if ( noSubGroups ) {
     parentItem->setOpen( true );
  }
}

Q3ListView* KBookmarkFolderTree::createTree( KBookmarkManager* mgr, QWidget* parent, const char* name, const QString& address )
{
  Q3ListView *listview = new Q3ListView( parent, name );

  listview->setRootIsDecorated( false );
  listview->header()->hide();
  listview->addColumn( i18n("Bookmark"), 200 );
  listview->setSorting( -1, false );
  listview->setSelectionMode( Q3ListView::Single );
  listview->setAllColumnsShowFocus( true );
  listview->setResizeMode( Q3ListView::AllColumns );
  listview->setMinimumSize( 60, 100 );

  fillTree( listview, mgr, address );

  return listview;
}

void KBookmarkFolderTree::fillTree( Q3ListView *listview, KBookmarkManager* mgr, const QString& address )
{
  listview->clear();

  KBookmarkGroup root = mgr->root();
  KBookmarkFolderTreeItem * rootItem = new KBookmarkFolderTreeItem( listview, root );
  listview->setCurrentItem( rootItem );
  rootItem->setSelected( true );
  fillGroup( listview, rootItem, root, (address == root.groupAddress() || address.isNull()) ? true : false, address );
  rootItem->setOpen( true );
}

static KBookmarkFolderTreeItem* ft_cast( Q3ListViewItem *i )
{
  return static_cast<KBookmarkFolderTreeItem*>( i );
}

QString KBookmarkFolderTree::selectedAddress( Q3ListView *listview )
{
  if ( !listview)
    return QString();
  KBookmarkFolderTreeItem *item = ft_cast( listview->currentItem() );
  return item ? item->m_bookmark.address() : QString();
}

void KBookmarkFolderTree::setAddress( Q3ListView *listview, const QString & address )
{
  KBookmarkFolderTreeItem* it = ft_cast( listview->firstChild() );
  while ( true ) {
    kdDebug(7043) << it->m_bookmark.address() << endl;
    it = ft_cast( it->itemBelow() );
    if ( !it )
      return;
    if ( it->m_bookmark.address() == address )
      break;
  }
  it->setSelected( true );
  listview->setCurrentItem( it );
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// toplevel item
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( Q3ListView *parent, const KBookmark & gp )
   : Q3ListViewItem(parent, i18n("Bookmarks")), m_bookmark(gp)
{
  setPixmap(0, SmallIcon("bookmark"));
  setExpandable(true);
}

// group
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *parent, Q3ListViewItem *after, const KBookmarkGroup & gp )
   : Q3ListViewItem(parent, after, gp.fullText()), m_bookmark(gp)
{
  setPixmap(0, SmallIcon( gp.icon() ) );
  setExpandable(true);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// NOTE - KBookmarkMenuNSImporter is really === KBookmarkMenuImporter
//        i.e, it is _not_ ns specific. and in KDE4 it should be renamed.

void KBookmarkMenuNSImporter::openNSBookmarks()
{
  openBookmarks( KNSBookmarkImporter::netscapeBookmarksFile(), "netscape" );
}

void KBookmarkMenuNSImporter::openBookmarks( const QString &location, const QString &type )
{
  mstack.push(m_menu);

  KBookmarkImporterBase *importer = KBookmarkImporterBase::factory(type);
  if (!importer)
     return;
  importer->setFilename(location);
  connectToImporter(*importer);
  importer->parse();

  delete importer;
}

void KBookmarkMenuNSImporter::connectToImporter(const QObject &importer)
{
  connect( &importer, SIGNAL( newBookmark( const QString &, const QString &, const QString & ) ),
           SLOT( newBookmark( const QString &, const QString &, const QString & ) ) );
  connect( &importer, SIGNAL( newFolder( const QString &, bool, const QString & ) ),
           SLOT( newFolder( const QString &, bool, const QString & ) ) );
  connect( &importer, SIGNAL( newSeparator() ), SLOT( newSeparator() ) );
  connect( &importer, SIGNAL( endFolder() ), SLOT( endFolder() ) );
}

void KBookmarkMenuNSImporter::newBookmark( const QString & text, const QString & url, const QString & )
{
  QString _text = KStringHandler::csqueeze(text);
  _text.replace( '&', "&&" );
  KAction * action = new KBookmarkAction(_text, "html",0,m_actionCollection, 0);
  connect(action, SIGNAL( activated ( KAction::ActivationReason, Qt::ButtonState )),
          m_menu, SLOT( slotBookmarkSelected( KAction::ActivationReason, Qt::ButtonState ) ));
  action->setProperty( "url", url );
  action->setToolTip( url );
  action->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuNSImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = KStringHandler::csqueeze(text);
  _text.replace( '&', "&&" );
  KActionMenu * actionMenu = new KActionMenu( _text, "folder", m_actionCollection, 0L );
  actionMenu->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( actionMenu );
  KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_menu->m_pOwner, actionMenu->popupMenu(),
                                              m_actionCollection, false,
                                              m_menu->m_bAddBookmark, QString() );
  connect( subMenu, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ),
           m_menu, SIGNAL( openBookmark( const QString &, Qt::ButtonState ) ));
  mstack.top()->m_lstSubMenus.append( subMenu );

  mstack.push(subMenu);
}

void KBookmarkMenuNSImporter::newSeparator()
{
  mstack.top()->m_parentMenu->insertSeparator();
}

void KBookmarkMenuNSImporter::endFolder()
{
  mstack.pop();
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkMenu::DynMenuInfo KBookmarkMenu::showDynamicBookmarks( const QString &id )
{
  KConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  DynMenuInfo info;
  info.show = false;

  if (!config.hasKey("DynamicMenus")) {
    // upgrade path
    if (id == "netscape") {
      KBookmarkManager *manager = KBookmarkManager::userBookmarksManager();
      info.show = manager->root().internalElement().attribute("hide_nsbk") != "yes";
      info.location = KNSBookmarkImporter::netscapeBookmarksFile();
      info.type = "netscape";
      info.name = i18n("Netscape Bookmarks");
    } // else, no show

  } else {
    // have new version config
    if (config.hasGroup("DynamicMenu-" + id)) {
      config.setGroup("DynamicMenu-" + id);
      info.show = config.readEntry("Show", false);
      info.location = config.readPathEntry("Location");
      info.type = config.readEntry("Type");
      info.name = config.readEntry("Name");
    } // else, no show
  }

  return info;
}

QStringList KBookmarkMenu::dynamicBookmarksList()
{
  KConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  QStringList mlist;
  if (config.hasKey("DynamicMenus"))
    mlist = config.readEntry("DynamicMenus", QStringList());
  else
    mlist << "netscape";

  return mlist;
}

void KBookmarkMenu::setDynamicBookmarks(const QString &id, const DynMenuInfo &newMenu)
{
  KConfig config("kbookmarkrc", false, false);

  // add group unconditionally
  config.setGroup("DynamicMenu-" + id);
  config.writeEntry("Show", newMenu.show);
  config.writePathEntry("Location", newMenu.location);
  config.writeEntry("Type", newMenu.type);
  config.writeEntry("Name", newMenu.name);

  QStringList elist;

  config.setGroup("Bookmarks");
  if (!config.hasKey("DynamicMenus")) {
    if (newMenu.type != "netscape") {
      // update from old xbel method to new rc method
      // though only if not writing the netscape setting
      config.setGroup("DynamicMenu-" "netscape");
      DynMenuInfo xbelSetting;
      xbelSetting = showDynamicBookmarks("netscape");
      config.writeEntry("Show", xbelSetting.show);
      config.writePathEntry("Location", xbelSetting.location);
      config.writeEntry("Type", xbelSetting.type);
      config.writeEntry("Name", xbelSetting.name);
    }
  } else {
    elist = config.readEntry("DynamicMenus", QStringList());
  }

  // make sure list includes type
  config.setGroup("Bookmarks");
  if (!elist.contains(id)) {
    elist << id;
    config.writeEntry("DynamicMenus", elist);
  }

  config.sync();
}

#include "kbookmarkmenu.moc"
#include "kbookmarkmenu_p.moc"
