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
#include "kbookmarktree.h"
#include "kbookmarkimporter.h"

#include <qstring.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "klineedit.h"

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

#include <kiconloader.h>

template class QPtrList<KBookmarkMenu>;

static bool *s_advanced = 0;

static bool isAdvanced()
{
  if (!s_advanced) {
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

QString KBookmarkMenu::contextMenuItemAddress()
{
  // calc parentgroup size
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  int length = 0;
  for ( KBookmark bm = parentBookmark.first(); !bm.isNull(); bm = parentBookmark.next(bm) ) 
    length++;

  // find relative position
  int idx = 0 - (KPopupMenu::contextMenuFocusItem() - m_parentMenu->idAt(0));

  // take into account the menu items and seperator in main bookmarks menu
  if ( (m_parentMenu->count() - length) == 3 )
    idx = idx - 3;

  // bounds checks
  if ( idx >= 0 && idx <= length )
    return QString("%1/%2").arg(m_parentAddress).arg(idx);
  else 
    return QString::null;
}

void KBookmarkMenu::slotAboutToShowContextMenu( KPopupMenu*, int, QPopupMenu* contextMenu )
{
  QString address = contextMenuItemAddress();
  kdDebug(7043) << "KBookmarkMenu::slotAboutToShowContextMenu" << address << endl;
  if (address.isNull())
    return; // TODO cancel about to show context menu
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());
  
  contextMenu->clear();

  if (bookmark.isGroup()) {
    contextMenu->insertItem( i18n( "Delete Folder" ), this, SLOT(slotRMBActionRemove()) );
    contextMenu->insertItem( i18n( "Open Folder as Tabset" ), this, SLOT(slotRMBActionOpen()) );
    // contextMenu->insertItem( i18n( "Properties" ), this, SLOT(slotRMBActionEdit()) );
    contextMenu->insertItem( i18n( "Open in Bookmark Editor" ), this, SLOT(slotRMBActionEditAt()) );
  } 
  else if (bookmark.isSeparator()) 
  {
    // cancel context menu ?
  }
  else
  {
    contextMenu->insertItem( i18n( "Delete bookmark" ), this, SLOT(slotRMBActionRemove()) );
    contextMenu->insertItem( i18n( "Open Bookmark" ), this, SLOT(slotRMBActionOpen()) );
  }
}

void KBookmarkMenu::slotRMBActionEditAt()
{
  QString address = contextMenuItemAddress();
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionEditAt" << address << endl;
  if (address.isNull())
    return;
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());

  emit m_pManager->slotEditBookmarksAtAddress( address );
}

void KBookmarkMenu::slotRMBActionRemove()
{
  QString address = contextMenuItemAddress();
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionRemove" << address << endl;
  if (address.isNull())
    return;
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  parentBookmark.deleteBookmark( bookmark );
  m_pManager->emitChanged( parentBookmark );
  m_parentMenu->hide();
}

void KBookmarkMenu::slotRMBActionOpen()
{
  QString address = contextMenuItemAddress();
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionOpen" << address << endl;
  if (address.isNull())
    return;
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());

  if ( !bookmark.isGroup() )
  {
    m_pOwner->openBookmarkURL( bookmark.url().url() );
  }
  else 
  {
    KBookmarkOwnerListCapable* owner = dynamic_cast<KBookmarkOwnerListCapable*>( m_pOwner ); 
    if (!owner) {
      kdWarning(7043) << "KBookmarkMenu::slotRMBActionOpen - not KBookmarkOwnerListCapable!" << endl;
      return;
    }

    QStringList urlList;
    KBookmarkGroup parentBookmark = bookmark.toGroup();
    for ( KBookmark bm = parentBookmark.first(); !bm.isNull(); bm = parentBookmark.next(bm) )
    {
      if ( bm.isSeparator() || bm.isGroup() ) 
        continue;
      urlList << bm.url().url().utf8();
    }
    
    owner->openBookmarkURLList( urlList );
  } 
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

    if ( m_pManager->showNSBookmarks()
         && QFile::exists( KNSBookmarkImporter::netscapeBookmarksFile() ) )
    {
      m_parentMenu->insertSeparator();

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
        // kdDebug(7043) << "Creating URL bookmark menu item for " << bm.text() << endl;
        // create a normal URL item, with ID as a name
        KAction * action = new KAction( text, bm.icon(), 0,
                                        this, SLOT( slotBookmarkSelected() ),
                                        m_actionCollection, bm.url().url().utf8() );

        action->setToolTip( bm.url().prettyURL() );

        action->plug( m_parentMenu );
        m_actions.append( action );
      }
    }
    else
    {
      // kdDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
      KActionMenu * actionMenu = new KActionMenu( text, bm.icon(),
                                                  m_actionCollection, 0L );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );
      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark,
                                                  bm.address() );

      m_lstSubMenus.append( subMenu );
    }
  }

  if ( !m_bIsRoot && m_bAddBookmark && !isAdvanced() )
  {
    if ( m_parentMenu->count() > 0 )
      m_parentMenu->insertSeparator();
    addAddBookmark();
    addNewFolder();
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
    BookmarkEditDialog dlg( title, url, m_pManager );
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
  // The name of the action is the URL to open
  m_pOwner->openBookmarkURL( QString::fromUtf8(sender()->name()) );
}

// -----------------------------------------------------------------------------

void KBookmarkMenu::slotNSBookmarkSelected()
{
  QString link(sender()->name()+8);
  m_pOwner->openBookmarkURL( link );
}

void KBookmarkMenu::slotNSLoad()
{
  m_parentMenu->disconnect(SIGNAL(aboutToShow()));

  KBookmarkMenuNSImporter importer( m_pManager, this, m_actionCollection );
  importer.openNSBookmarks();
}

void KBookmarkMenuNSImporter::openNSBookmarks()
{
  mstack.push(m_menu);
  KNSBookmarkImporter importer( KNSBookmarkImporter::netscapeBookmarksFile() );
  connect( &importer, SIGNAL( newBookmark( const QString &, const QCString &, const QString & ) ),
           SLOT( newBookmark( const QString &, const QCString &, const QString & ) ) );
  connect( &importer, SIGNAL( newFolder( const QString &, bool, const QString & ) ),
           SLOT( newFolder( const QString &, bool, const QString & ) ) );
  connect( &importer, SIGNAL( newSeparator() ), SLOT( newSeparator() ) );
  connect( &importer, SIGNAL( endFolder() ), SLOT( endFolder() ) );
  importer.parseNSBookmarks();
}

void KBookmarkMenuNSImporter::newBookmark( const QString & text, const QCString & url, const QString & )
{
  QCString actionLink = "bookmark" + url;
  QString _text = text;
  _text.replace( QRegExp( "&" ), "&&" );
  KAction * action = new KAction( _text, "html", 0, m_menu, SLOT( slotNSBookmarkSelected() ),
                                  m_actionCollection, actionLink.data());
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

BookmarkEditDialog::BookmarkEditDialog(const QString& title, const QString& url, KBookmarkManager * mgr,
                                       QWidget * parent, const char * name, const QString& caption)
  : KDialogBase(parent, name, true, caption, User1|Ok|Cancel, Ok, false, KGuiItem(i18n("New Folder...")))
{
  m_mgr = mgr;

  setButtonOKText( i18n( "Add" ) );

  m_main = new QWidget( this );
  setMainWidget( m_main );

  QBoxLayout *vert = new QVBoxLayout( m_main, spacingHint() );
  QGridLayout *grid = new QGridLayout( vert, 2, 2 );

  m_title = new KLineEdit( m_main );
  m_title->setText( title );
  grid->addWidget( m_title, 0, 1 );
  grid->addWidget( new QLabel( m_title, i18n( "Name:" ), m_main ), 0, 0 );

  m_url = new KLineEdit( m_main );
  m_url->setText( url );
  grid->addWidget( m_url, 1, 1 );
  grid->addWidget( new QLabel( m_url, i18n( "Location:" ), m_main ), 1, 0 );

  m_folderTree = KBookmarkFolderTree::createTree( m_mgr, m_main, name );
  vert->addWidget( m_folderTree );

  connect( this, SIGNAL( user1Clicked() ), SLOT( slotInsertFolder() ) );
}

void BookmarkEditDialog::slotOk() { accept(); }
void BookmarkEditDialog::slotCancel() { reject(); } 

QString BookmarkEditDialog::finalAddress()
{ 
  return KBookmarkFolderTree::selectedAddress( m_folderTree ); 
}

QString BookmarkEditDialog::finalUrl() { return m_url->text(); }
QString BookmarkEditDialog::finalTitle() { return m_title->text(); }

void BookmarkEditDialog::slotInsertFolder()
{
  // kdDebug(7043) << "BookmarkEditDialog::slotInsertFolder" << endl;
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
  KBookmarkFolderTree::recreateTree( m_folderTree, m_mgr );
}

#include "kbookmarkmenu.moc"
