// -*- mode: c++; c-basic-offset: 2 -*-
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include "kbookmarkdrag.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <qclipboard.h>

#include <klineedit.h>

#include <qfile.h>
#include <qregexp.h>

#include <kapplication.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kconfig.h>

#include <qlistview.h>
#include <qheader.h>

#include <kiconloader.h>

template class QPtrList<KBookmarkMenu>;

static bool *s_advanced = 0;

static bool isAdvanced()
{
  if (!s_advanced) 
  {
    s_advanced = new bool;
    KGlobal::config()->setGroup( "Settings" );
    (*s_advanced) = KGlobal::config()->readBoolEntry( "AdvancedAddBookmark", false );
  }
  return (*s_advanced);
}

/********************************************************************
 *
 * KBookmarkMenu
 *
 ********************************************************************/

KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, KPopupMenu * _parentMenu,
                              KActionCollection *collec, bool _isRoot, bool _add,
                              const QString & parentAddress )
  : m_bIsRoot(_isRoot), m_bAddBookmark(_add),
    m_bAddShortcuts(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_actionCollection( collec ),
    m_parentAddress( parentAddress )
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  m_lstSubMenus.setAutoDelete( true );
  m_actions.setAutoDelete( true );

  m_bNSBookmark = m_parentAddress.isNull();
  if ( !m_bNSBookmark ) // not for the netscape bookmark
  {
    //kdDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress << endl;

    connect( _parentMenu, SIGNAL( aboutToShow() ),
             SLOT( slotAboutToShow() ) );

    if ( isAdvanced() ) 
    {
      (void) _parentMenu->contextMenu();
      connect( _parentMenu, SIGNAL( aboutToShowContextMenu(KPopupMenu*, int, QPopupMenu*) ),
               this, SLOT( slotAboutToShowContextMenu(KPopupMenu*, int, QPopupMenu*) ));
      disconnect( m_actionCollection, SIGNAL( actionHighlighted( KAction * ) ), 0, 0 );
      connect( m_actionCollection, SIGNAL( actionHighlighted( KAction * ) ),
               this, SLOT( slotActionHighlighted( KAction * ) ) );
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
      addAddBookmark();

    addEditBookmarks();
  }

  m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
  //kdDebug(7043) << "KBookmarkMenu::~KBookmarkMenu() " << this << endl;
  QPtrListIterator<KAction> it( m_actions );
  for (; it.current(); ++it )
    it.current()->unplugAll();

  m_lstSubMenus.clear();
  m_actions.clear();
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

void KBookmarkMenu::slotActionHighlighted( KAction* action )
{
  if (action->isA("KBookmarkActionMenu") || action->isA("KBookmarkAction")) 
  {
    s_highlightedAddress = action->property("address").toString();
    //kdDebug(7043) << "KBookmarkMenu::slotActionHighlighted" << s_highlightedAddress << endl;
  } 
  else 
  {
    s_highlightedAddress = QString::null;
  }
}

bool KBookmarkMenu::invalid( int val )
{
  bool valid = true;

  if (val == 1) 
    s_highlightedAddress = m_parentAddress;

  if (s_highlightedAddress.isNull()) 
  {
    KPopupMenu::contextMenuFocus()->hideContextMenu();
    valid = false;
  }

  return !valid;
}

void KBookmarkMenu::slotAboutToShowContextMenu( KPopupMenu*, int, QPopupMenu* contextMenu )
{
  //kdDebug(7043) << "KBookmarkMenu::slotAboutToShowContextMenu" << s_highlightedAddress << endl;
  if (s_highlightedAddress.isNull()) 
  {
    KPopupMenu::contextMenuFocus()->hideContextMenu();
    return; 
  }
  contextMenu->clear();
  fillContextMenu( contextMenu, s_highlightedAddress, 0 );
}

void KBookmarkMenu::fillContextMenu( QPopupMenu* contextMenu, const QString & address, int val )
{
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());

  int id;

  if (bookmark.isGroup()) {
    id = contextMenu->insertItem( i18n( "Open in Bookmark Editor" ), this, SLOT(slotRMBActionEditAt(int)) );
    contextMenu->setItemParameter( id, val );
    id = contextMenu->insertItem( i18n( "Delete Folder" ), this, SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    // contextMenu->insertItem( i18n( "Properties" ), this, SLOT(slotRMBActionProperties(int)) );
  } 
  else
  {
    id = contextMenu->insertItem( i18n( "Delete Bookmark" ), this, SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    id = contextMenu->insertItem( i18n( "Copy Link Location" ), this, SLOT(slotRMBActionCopyLocation(int)) );
    contextMenu->setItemParameter( id, val );
    id = contextMenu->insertItem( i18n( "Open Bookmark" ), this, SLOT(slotRMBActionOpen(int)) );
    contextMenu->setItemParameter( id, val );
  }

  emit aboutToShowContextMenu( bookmark, contextMenu );
}

void KBookmarkMenu::slotRMBActionEditAt( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionEditAt" << s_highlightedAddress << endl;
  if (invalid(val)) return;

  KBookmark bookmark = m_pManager->findByAddress( s_highlightedAddress );
  Q_ASSERT(!bookmark.isNull());

  emit m_pManager->slotEditBookmarksAtAddress( s_highlightedAddress );
}

void KBookmarkMenu::slotRMBActionRemove( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionRemove" << s_highlightedAddress << endl;
  //TODO - "are you sure?" box, with "don't show again" option
  if (invalid(val)) return;

  KBookmark bookmark = m_pManager->findByAddress( s_highlightedAddress );
  Q_ASSERT(!bookmark.isNull());

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  parentBookmark.deleteBookmark( bookmark );
  m_pManager->emitChanged( parentBookmark );
  m_parentMenu->hide();
}

void KBookmarkMenu::slotRMBActionCopyLocation( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionOpen" << s_highlightedAddress << endl;
  if (invalid(val)) return;

  KBookmark bookmark = m_pManager->findByAddress( s_highlightedAddress );
  Q_ASSERT(!bookmark.isNull());

  if ( !bookmark.isGroup() )
  {
    kapp->clipboard()->setData( KBookmarkDrag::newDrag(bookmark, 0), 
                                QClipboard::Clipboard );
  }
}

void KBookmarkMenu::slotRMBActionOpen( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionOpen" << s_highlightedAddress << endl;
  if (invalid(val)) return;

  KBookmark bookmark = m_pManager->findByAddress( s_highlightedAddress );
  Q_ASSERT(!bookmark.isNull());
  Q_ASSERT(!bookmark.isGroup());

  m_pOwner->openBookmarkURL( bookmark.url().url() );
}

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
    QPtrListIterator<KBookmarkMenu> it( m_lstSubMenus );
    for (; it.current(); ++it )
    {
      it.current()->slotBookmarksChanged( groupAddress );
    }
  }
}

void KBookmarkMenu::refill()
{
  //kdDebug(7043) << "KBookmarkMenu::refill()" << endl;
  m_lstSubMenus.clear();

  QPtrListIterator<KAction> it( m_actions );
  for (; it.current(); ++it )
    it.current()->unplug( m_parentMenu );

  m_parentMenu->clear();
  m_actions.clear();

  fillBookmarkMenu();
  m_parentMenu->adjustSize();
}

void KBookmarkMenu::addAddBookmark()
{
  if (!kapp->authorizeKAction("bookmarks"))
     return;
  
  QString title = i18n( "&Add Bookmark" );
  int p;
  while ( ( p = title.find( '&' ) ) >= 0 )
    title.remove( p, 1 );

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
  if (!kapp->authorizeKAction("bookmarks"))
     return;

  KAction * m_paEditBookmarks = KStdAction::editBookmarks( m_pManager, SLOT( slotEditBookmarks() ),
                                                             m_actionCollection, "edit_bookmarks" );
  m_paEditBookmarks->plug( m_parentMenu );
  m_paEditBookmarks->setToolTip( i18n( "Edit your bookmark collection in a separate window" ) );
  m_actions.append( m_paEditBookmarks );
}

void KBookmarkMenu::addNewFolder()
{
  if (!kapp->authorizeKAction("bookmarks"))
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
                                       m_actionCollection );

  paNewFolder->setToolTip( i18n( "Create a new bookmark folder in this menu" ) );

  paNewFolder->plug( m_parentMenu );
  m_actions.append( paNewFolder );
}

void KBookmarkMenu::fillBookmarkMenu()
{
  if (!kapp->authorizeKAction("bookmarks"))
     return;

  if ( m_bIsRoot )
  {
    if ( m_bAddBookmark )
      addAddBookmark();

    addEditBookmarks();

    if ( m_bAddBookmark && !isAdvanced() )
      addNewFolder();

    bool haveSep = false;

    QString type = "netscape";
    QPair<bool, QString> info = m_pManager->showDynamicBookmarks(type);

    if ( info.first && QFile::exists( info.second ) )
    {
      if (!haveSep) {
         m_parentMenu->insertSeparator();
         haveSep = true;
      }

      KActionMenu * actionMenu = new KActionMenu( i18n("Netscape Bookmarks"), "netscape",
                                                  m_actionCollection, 0L );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );
      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark, QString::null );
      m_lstSubMenus.append(subMenu);
      connect(actionMenu->popupMenu(), SIGNAL(aboutToShow()), subMenu, SLOT(slotNSLoad()));
    }
  }

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  bool separatorInserted = false;
  for ( KBookmark bm = parentBookmark.first(); !bm.isNull();  bm = parentBookmark.next(bm) )
  {
    QString text = bm.text();
    text.replace( QRegExp( "&" ), "&&" );
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
        KAction * action = new KBookmarkAction( text, bm.icon(), 0,
                                                this, SLOT( slotBookmarkSelected() ),
                                                m_actionCollection, 0 );

        action->setProperty( "url", bm.url().url() );
        action->setProperty( "address", bm.address() );

        action->setToolTip( bm.url().prettyURL() );

        action->plug( m_parentMenu );
        m_actions.append( action );
      }
    }
    else
    {
      //kdDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
      KActionMenu * actionMenu = new KBookmarkActionMenu( text, bm.icon(),
                                                          m_actionCollection, 0L );
      actionMenu->setProperty( "address", bm.address() );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );

      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark,
                                                  bm.address() );
      m_lstSubMenus.append( subMenu );
    }
  }

  if ( !m_bIsRoot && m_bAddBookmark )
  {
    if ( m_parentMenu->count() > 0 )
      m_parentMenu->insertSeparator();
    if (isAdvanced())
    {
      KActionMenu * actionMenu = new KActionMenu( i18n("Actions"), m_actionCollection, 0L );
      fillContextMenu( actionMenu->popupMenu(), m_parentAddress, 1 );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );
    }
    else 
    {
      addAddBookmark();
      addNewFolder();
    }
  }
}

void KBookmarkMenu::slotAddBookmark()
{
  QString url = m_pOwner->currentURL();
  if (url.isEmpty())
  {
    KMessageBox::error( 0L, i18n("Can't add bookmark with empty URL"));
    return;
  }
  QString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  KBookmarkGroup parentBookmark;

  if ( !isAdvanced() ) 
  {
    parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
    Q_ASSERT(!parentBookmark.isNull());

    // If this title is already used, we'll try to find something unused.
    KBookmark ch = parentBookmark.first();
    int count = 1;
    QString uniqueTitle = title;
    do
    {
      while ( !ch.isNull() )
      {
        if ( uniqueTitle == ch.text() )
        {
          // Title already used !
          if ( url != ch.url().url() )
          {
            uniqueTitle = title + QString(" (%1)").arg(++count);
            // New title -> restart search from the beginning
            ch = parentBookmark.first();
            break;
          }
          else
          {
            // this exact URL already exists
            return;
          }
        }
        ch = parentBookmark.next( ch );
      }
    } while ( !ch.isNull() );
    parentBookmark.addBookmark( m_pManager, uniqueTitle, url );

  } 
  else 
  {
    KBookmarkEditDialog dlg( title, url, m_pManager );
    if ( dlg.exec() != KDialogBase::Accepted )
      return;

    parentBookmark = m_pManager->findByAddress( dlg.finalAddress() ).toGroup();
    Q_ASSERT(!parentBookmark.isNull());

    parentBookmark.addBookmark( m_pManager, dlg.finalTitle(), dlg.finalUrl() );
  }

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

void KBookmarkMenu::slotBookmarkSelected()
{
  //kdDebug(7043) << "KBookmarkMenu::slotBookmarkSelected()" << endl;
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  m_pOwner->openBookmarkURL( sender()->property("url").toString() );
}

// -----------------------------------------------------------------------------

void KBookmarkMenu::slotNSLoad()
{
  m_parentMenu->disconnect(SIGNAL(aboutToShow()));

  KBookmarkMenuNSImporter importer( m_pManager, this, m_actionCollection );
  importer.openNSBookmarks();
}

// -----------------------------------------------------------------------------

KBookmarkEditDialog::KBookmarkEditDialog(const QString& title, const QString& url, KBookmarkManager * mgr,
                                       QWidget * parent, const char * name, const QString& caption)
  : KDialogBase(parent, name, true, caption, User1|Ok|Cancel, Ok, false, KGuiItem(i18n("New Folder..."))), m_mgr(mgr)
{
  setButtonOKText( i18n( "Add" ) );

  m_main = new QWidget( this );
  setMainWidget( m_main );

  QBoxLayout *vbox = new QVBoxLayout( m_main, spacingHint() );
  QGridLayout *grid = new QGridLayout( vbox, 2, 2 );

  m_title = new KLineEdit( m_main );
  m_title->setText( title );
  grid->addWidget( m_title, 0, 1 );
  grid->addWidget( new QLabel( m_title, i18n( "Name:" ), m_main ), 0, 0 );

  m_url = new KLineEdit( m_main );
  m_url->setText( url );
  grid->addWidget( m_url, 1, 1 );
  grid->addWidget( new QLabel( m_url, i18n( "Location:" ), m_main ), 1, 0 );

  m_folderTree = KBookmarkFolderTree::createTree( m_mgr, m_main, name );
  connect( m_folderTree, SIGNAL( doubleClicked(QListViewItem*) ), 
           this,         SLOT( slotDoubleClicked(QListViewItem*) ) );
  vbox->addWidget( m_folderTree );

  connect( this, SIGNAL( user1Clicked() ), SLOT( slotInsertFolder() ) );
}

void KBookmarkEditDialog::slotDoubleClicked(QListViewItem* item)
{
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

QString KBookmarkEditDialog::finalAddress()
{ 
  return KBookmarkFolderTree::selectedAddress( m_folderTree ); 
}

QString KBookmarkEditDialog::finalUrl() 
{ 
  return m_url->text(); 
}

QString KBookmarkEditDialog::finalTitle() 
{ 
  return m_title->text(); 
}

void KBookmarkEditDialog::slotInsertFolder()
{
  // kdDebug(7043) << "KBookmarkEditDialog::slotInsertFolder" << endl;
  QString address = KBookmarkFolderTree::selectedAddress( m_folderTree ); 
  if ( address.isNull() ) return;
  KBookmarkGroup parentBookmark = m_mgr->findByAddress( address ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_mgr );
  if ( !group.isNull() )
  {
    KBookmarkGroup parentGroup = group.parentGroup();
    m_mgr->emitChanged( parentGroup );
  }
  KBookmarkFolderTree::fillTree( m_folderTree, m_mgr );
}

// -----------------------------------------------------------------------------

static void fillGroup( KBookmarkFolderTreeItem * parentItem, KBookmarkGroup group )
{
  bool noSubGroups = true;
  KBookmarkFolderTreeItem * lastItem = 0L;
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    if ( bk.isGroup() )
    {
      KBookmarkGroup grp = bk.toGroup();
      KBookmarkFolderTreeItem * item = new KBookmarkFolderTreeItem( parentItem, lastItem, grp );
      fillGroup( item, grp );
      if ( grp.isOpen() )
        item->setOpen( true );
      lastItem = item;
      noSubGroups = false;
    }
  }
  if ( noSubGroups ) {
     parentItem->setOpen( true );
  }
}

QListView* KBookmarkFolderTree::createTree( KBookmarkManager* mgr, QWidget* parent, const char* name )
{
  QListView *listview = new QListView( parent, name );

  listview->setRootIsDecorated( false );
  listview->header()->hide();
  listview->addColumn( i18n("Bookmark"), 200 );
  listview->setSorting( -1, false );
  listview->setSelectionMode( QListView::Single );
  listview->setAllColumnsShowFocus( true );
  listview->setResizeMode( QListView::AllColumns );
  listview->setMinimumSize( 60, 100 );

  fillTree( listview, mgr );

  return listview;
}

void KBookmarkFolderTree::fillTree( QListView *listview, KBookmarkManager* mgr ) 
{
  listview->clear();

  KBookmarkGroup root = mgr->root();
  KBookmarkFolderTreeItem * rootItem = new KBookmarkFolderTreeItem( listview, root );
  fillGroup( rootItem, root );
  rootItem->setOpen( true );

  listview->setFocus();
  listview->setCurrentItem( rootItem );
  listview->firstChild()->setSelected( true );
}

QString KBookmarkFolderTree::selectedAddress( QListView *listview )
{
  KBookmarkFolderTreeItem *item = static_cast<KBookmarkFolderTreeItem*>( listview->currentItem() );
  return item ? item->m_bookmark.address() : QString::null;
}

// -----------------------------------------------------------------------------

// toplevel item
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( QListView *parent, const KBookmark & gp )
   : QListViewItem(parent, i18n("Bookmarks")), m_bookmark(gp)
{
  setPixmap(0, SmallIcon("bookmark"));
  setExpandable(true);
}

// group
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *parent, QListViewItem *after, const KBookmarkGroup & gp )
   : QListViewItem(parent, after, gp.fullText()), m_bookmark(gp)
{
  setPixmap(0, SmallIcon( gp.icon() ) );
  setExpandable(true);
}

// -----------------------------------------------------------------------------

void KBookmarkMenuNSImporter::openNSBookmarks()
{
  mstack.push(m_menu);
  KNSBookmarkImporter importer( KNSBookmarkImporter::netscapeBookmarksFile() );
  connectToImporter(importer);
  importer.parseNSBookmarks();
}

void KBookmarkMenuNSImporter::connectToImporter(const QObject &importer)
{
  connect( &importer, SIGNAL( newBookmark( const QString &, const QCString &, const QString & ) ),
           SLOT( newBookmark( const QString &, const QCString &, const QString & ) ) );
  connect( &importer, SIGNAL( newFolder( const QString &, bool, const QString & ) ),
           SLOT( newFolder( const QString &, bool, const QString & ) ) );
  connect( &importer, SIGNAL( newSeparator() ), SLOT( newSeparator() ) );
  connect( &importer, SIGNAL( endFolder() ), SLOT( endFolder() ) );
}

void KBookmarkMenuNSImporter::newBookmark( const QString & text, const QCString & url, const QString & )
{
  QString _text = text;
  _text.replace( QRegExp( "&" ), "&&" );
  KAction * action = new KBookmarkAction(_text, "html", 0, 
                                         m_menu, SLOT( slotBookmarkSelected() ),
                                         m_actionCollection, 0);
  action->setProperty( "url", url );
  action->setToolTip( url );
  action->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuNSImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = text;
  _text.replace( QRegExp( "&" ), "&&" );
  KActionMenu * actionMenu = new KActionMenu( _text, "folder", m_actionCollection, 0L );
  actionMenu->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( actionMenu );
  KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_menu->m_pOwner, actionMenu->popupMenu(),
                                              m_actionCollection, false,
                                              m_menu->m_bAddBookmark, QString::null );
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


#include "kbookmarkmenu.moc"
#include "kbookmarkmenu_p.moc"
