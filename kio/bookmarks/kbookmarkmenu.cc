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


#include <QStack>
#include <QHeaderView>

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
    m_parentAddress( parentAddress ),
    m_rmb(0)
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  m_bNSBookmark = m_parentAddress.isNull();
  if ( !m_bNSBookmark ) // not for the netscape bookmark
  {
    //kDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress << endl;

    connect( _parentMenu, SIGNAL( aboutToShow() ),
             SLOT( slotAboutToShow() ) );

    if ( KBookmarkSettings::self()->m_contextmenu )
    {
      m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(m_parentMenu, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
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
  //kDebug(7043) << "KBookmarkMenu::~KBookmarkMenu() " << this << endl;
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

/********************************************************************/
/********************************************************************/
/********************************************************************/

RMB::RMB(QString parentAddress, QString highlightedAddress,
         KBookmarkManager *pManager, KBookmarkOwner *pOwner, QWidget *parentMenu)
: m_parentAddress(parentAddress), m_highlightedAddress(highlightedAddress),
  m_pManager(pManager), m_pOwner(pOwner), m_parentMenu(parentMenu)
{
    m_contextMenu = new QMenu();
}

RMB::~RMB()
{
    delete m_contextMenu;
}

void RMB::popup(const QPoint & pos)
{
    if(!m_highlightedAddress.isNull())
        m_contextMenu->popup(pos);
}

QMenu * RMB::contextMenu()
{
    return m_contextMenu;
}

KBookmark RMB::atAddress(const QString & address)
{
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());
  return bookmark;
}

void KBookmarkMenu::contextMenu( const QPoint & pos )
{
  QAction * action = m_parentMenu->actionAt(pos);
  showContextMenu(action->property("address").toString(), pos);
}

void RMB::fillContextMenu( const QString & address)
{
  m_contextMenu->clear();
  KBookmark bookmark = atAddress(address);
  m_contextMenu->addAction( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), this, SLOT(slotInsert()) );
}

void RMB::fillContextMenu2( const QString & address)
{
  KBookmark bookmark = atAddress(address);

  if (bookmark.isGroup()) {
    m_contextMenu->addAction( i18n( "Open Folder in Bookmark Editor" ), this, SLOT(slotEditAt()) );
    m_contextMenu->addSeparator();
    m_contextMenu->addAction( SmallIcon("editdelete"), i18n( "Delete Folder" ), this, SLOT(slotRemove()) );
    m_contextMenu->addSeparator();
    m_contextMenu->addAction( i18n( "Properties" ), this, SLOT(slotProperties()) );
  }
  else
  {
    m_contextMenu->addAction( i18n( "Copy Link Address" ), this, SLOT(slotCopyLocation()) );
    m_contextMenu->addSeparator();
    m_contextMenu->addAction( SmallIcon("editdelete"), i18n( "Delete Bookmark" ), this, SLOT(slotRemove()) );
    m_contextMenu->addSeparator();
    m_contextMenu->addAction( i18n( "Properties" ), this, SLOT(slotProperties()) );
  }
}

void RMB::slotEditAt()
{
  //kDebug(7043) << "KBookmarkMenu::slotEditAt" << m_highlightedAddress << endl;

  KBookmark bookmark = atAddress(m_highlightedAddress);

  m_pManager->slotEditBookmarksAtAddress( m_highlightedAddress );
}

void RMB::slotProperties()
{
  //kDebug(7043) << "KBookmarkMenu::slotProperties" << m_highlightedAddress << endl;

  KBookmark bookmark = atAddress(m_highlightedAddress);

  QString folder = bookmark.isGroup() ? QString() : bookmark.url().pathOrUrl();
  KBookmarkEditDialog dlg( bookmark.fullText(), folder,
                           m_pManager, KBookmarkEditDialog::ModifyMode, 0,
                           0, i18n("Bookmark Properties") );
  if ( dlg.exec() != KDialog::Accepted )
    return;

  makeTextNodeMod(bookmark, "title", dlg.finalTitle());
  if ( !dlg.finalUrl().isNull() )
  {
    KUrl u(dlg.finalUrl());
    bookmark.internalElement().setAttribute("href", u.url()); // utf8
  }

  KBookmarkGroup parentBookmark = atAddress(m_parentAddress).toGroup();
  m_pManager->emitChanged( parentBookmark );
}

void RMB::slotInsert()
{
  //kDebug(7043) << "KBookmarkMenu::slotInsert" << m_highlightedAddress << endl;

  QString url = m_pOwner->currentURL();
  if (url.isEmpty())
  {
    KMessageBox::error( 0L, i18n("Cannot add bookmark with empty URL."));
    return;
  }
  QString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  KBookmark bookmark = atAddress( m_highlightedAddress );

  // TODO use unique title

  if (bookmark.isGroup())
  {
    KBookmarkGroup parentBookmark = bookmark.toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    parentBookmark.addBookmark( m_pManager, title, KUrl(  url ) );
    m_pManager->emitChanged( parentBookmark );
  }
  else
  {
    KBookmarkGroup parentBookmark = bookmark.parentGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmark newBookmark = parentBookmark.addBookmark( m_pManager, title, KUrl( url ) );
    parentBookmark.moveItem( newBookmark, parentBookmark.previous(bookmark) );
    m_pManager->emitChanged( parentBookmark );
  }
}

void RMB::slotRemove()
{
  //kDebug(7043) << "KBookmarkMenu::slotRemove" << m_highlightedAddress << endl;

  KBookmark bookmark = atAddress( m_highlightedAddress );
  bool folder = bookmark.isGroup();

  if (KMessageBox::warningContinueCancel(
          m_parentMenu,
          folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", bookmark.text())
                 : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", bookmark.text()),
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

void RMB::slotCopyLocation()
{
  //kDebug(7043) << "KBookmarkMenu::slotCopyLocation" << m_highlightedAddress << endl;

  const KBookmark bookmark = atAddress( m_highlightedAddress );

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

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::showContextMenu( const QString & address, const QPoint & pos)
{

  delete m_rmb;
  m_rmb = new RMB(m_parentAddress, address, m_pManager, m_pOwner, m_parentMenu);
  m_rmb->fillContextMenu(address);
  emit aboutToShowContextMenu( m_rmb->atAddress(address), m_rmb->contextMenu());
  m_rmb->fillContextMenu2(address);
  m_rmb->popup( m_parentMenu->mapToGlobal(pos) );
}

void KBookmarkMenu::slotBookmarksChanged( const QString & groupAddress )
{
  if (m_bNSBookmark)
    return;

  if ( groupAddress == m_parentAddress )
  {
    //kDebug(7043) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress << endl;
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
  kDebug(7043) << "KBookmarkMenu::refill()" << endl;
  qDeleteAll( m_lstSubMenus );
  m_lstSubMenus.clear();

  for ( QList<KAction *>::iterator it = m_actions.begin(), end = m_actions.end() ;
        it != end ; ++it )
  {
        m_parentMenu->removeAction(*it);
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

  KAction * paAddBookmarksList = new KAction( title, m_actionCollection, m_bIsRoot ? "add_bookmarks_list" : 0 );
  paAddBookmarksList->setIcon( KIcon( "bookmarks_list_add" ) );
  paAddBookmarksList->setToolTip( i18n( "Add a folder of bookmarks for all open tabs." ) );
  connect( paAddBookmarksList, SIGNAL( triggered( bool ) ), this, SLOT( slotAddBookmarksList() ) );

  m_parentMenu->addAction(paAddBookmarksList);
  m_actions.append( paAddBookmarksList );
}

void KBookmarkMenu::addAddBookmark()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  QString title = i18n( "Add Bookmark" );

  KAction * paAddBookmarks = new KAction( title, m_actionCollection, m_bIsRoot ? "add_bookmark" : 0 );
  paAddBookmarks->setIcon( KIcon( "bookmark_add" ) );
  paAddBookmarks->setShortcut( m_bIsRoot && m_bAddShortcuts ? KStdAccel::addBookmark() : KShortcut() );
  paAddBookmarks->setToolTip( i18n( "Add a bookmark for the current document" ) );
  connect( paAddBookmarks, SIGNAL( triggered( bool ) ), this, SLOT( slotAddBookmark() ) );

  m_parentMenu->addAction(paAddBookmarks);
  m_actions.append( paAddBookmarks );
}

void KBookmarkMenu::addEditBookmarks()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  KAction * m_paEditBookmarks = KStdAction::editBookmarks( m_pManager, SLOT( slotEditBookmarks() ),
                                                             m_actionCollection, "edit_bookmarks" );
  m_parentMenu->addAction(m_paEditBookmarks);
  m_paEditBookmarks->setToolTip( i18n( "Edit your bookmark collection in a separate window" ) );
  m_actions.append( m_paEditBookmarks );
}

void KBookmarkMenu::addNewFolder()
{
  if (!KAuthorized::authorizeKAction("bookmarks"))
     return;

  QString title = i18n( "&New Bookmark Folder..." );
  title.remove( QChar( '&' ) ); //FIXME Hmm, why?

  KAction * paNewFolder = new KAction( title, m_actionCollection,"dummyname" );
  paNewFolder->setIcon( KIcon( "folder_new" ) );
  paNewFolder->setToolTip( i18n( "Create a new bookmark folder in this menu" ) );
  connect( paNewFolder, SIGNAL( triggered( bool ) ), this, SLOT( slotNewFolder() ) );

  m_parentMenu->addAction(paNewFolder);
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
          m_parentMenu->addSeparator();
          haveSep = true;
       }

       KActionMenu * actionMenu;
       actionMenu = new KActionMenu(
                              KIcon(info.type), info.name,
                              m_actionCollection, "kbookmarkmenu" );

       m_parentMenu->addAction(actionMenu);
       m_actions.append( actionMenu );

       KImportedBookmarkMenu *subMenu =
          new KImportedBookmarkMenu( m_pManager, m_pOwner, actionMenu->menu(),
                             m_actionCollection, false,
                             m_bAddBookmark, QString(), info.type, info.location);
       connect( subMenu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ),
                this, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ));
       m_lstSubMenus.append( subMenu );
    }
  }

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());

  if ( m_bIsRoot && parentBookmark.first().isNull() ) // at least one bookmark
  {
      m_parentMenu->addSeparator();
  }

  for ( KBookmark bm = parentBookmark.first(); !bm.isNull();  bm = parentBookmark.next(bm) )
  {
    QString text = KStringHandler::csqueeze(bm.fullText(), 60);
    text.replace( '&', "&&" );
    if ( !bm.isGroup() )
    {
      if ( bm.isSeparator() )
      {
        m_parentMenu->addSeparator();
      }
      else
      {
        //kDebug(7043) << "Creating URL bookmark menu item for " << bm.text() << endl;
        KAction * action = new KBookmarkAction( bm, m_actionCollection);
        connect(action, SIGNAL( triggered(Qt::MouseButtons, Qt::KeyboardModifiers) ), SLOT( slotBookmarkSelected(Qt::MouseButtons, Qt::KeyboardModifiers) ));
        m_parentMenu->addAction(action);
        m_actions.append( action );
      }
    }
    else
    {
      //kDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
      KActionMenu * actionMenu = new KBookmarkActionMenu( KIcon(bm.icon()), text,
                                                          m_actionCollection,
                                                          "kbookmarkmenu" );
      actionMenu->setProperty( "address", bm.address() );
      m_parentMenu->addAction(actionMenu);
      m_actions.append( actionMenu );

      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->menu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark,
                                                  bm.address() );

      connect(subMenu, SIGNAL( aboutToShowContextMenu( const KBookmark &, QMenu * ) ),
                 this, SIGNAL( aboutToShowContextMenu( const KBookmark &, QMenu * ) ));
      connect(subMenu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ),
                this, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ));
      m_lstSubMenus.append( subMenu );
    }
  }

  if ( !m_bIsRoot && m_bAddBookmark )
  {
    if ( m_parentMenu->actions().count() > 0 )
      m_parentMenu->addSeparator();

    addAddBookmark();
    if ( extOwner() )
      addAddBookmarksList(); // FIXME
    addNewFolder();
  }
}

void KBookmarkMenu::slotAddBookmarksList()
{
  KExtendedBookmarkOwner *extOwner = dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
  if (!extOwner)
  {
    kWarning() << "erm, sorry ;-)" << endl;
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
    group.addBookmark( m_pManager, (*it).first, KUrl((*it).second) );

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

void KBookmarkMenu::slotBookmarkSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
  //kDebug(7043) << "KBookmarkMenu::slotBookmarkSelected()" << endl;
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...

  if (qobject_cast<const KAction*>(sender()))
  {
      const QString& url = sender()->property("url").toString();
      m_pOwner->openBookmarkURL( url );
      emit openBookmark( url, mb, km );
  }
}

KExtendedBookmarkOwner* KBookmarkMenu::extOwner()
{
  return dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
}

void KImportedBookmarkMenu::slotNSLoad()
{
  kDebug(7043)<<"**** slotNSLoad  ****"<<m_type<<"  "<<m_location<<endl;
  // only fill menu once
  m_parentMenu->disconnect(SIGNAL(aboutToShow()));

  // not NSImporter, but kept old name for BC reasons
  KBookmarkMenuNSImporter importer( m_pManager, this, m_actionCollection );
  importer.openBookmarks(m_location, m_type);
}

KImportedBookmarkMenu::KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu,
                 KActionCollection * collec, bool root, bool add,
                 const QString & parentAddress, const QString & type, const QString & location )
    :KBookmarkMenu(mgr, owner, parentMenu, collec, root, add, parentAddress), m_type(type), m_location(location)
{
    connect(parentMenu, SIGNAL(aboutToShow()), this, SLOT(slotNSLoad()));
}

KImportedBookmarkMenu::~KImportedBookmarkMenu()
{

}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkEditFields::KBookmarkEditFields(QWidget *main, QBoxLayout *vbox, FieldsSet fieldsSet)
{
  QLabel *tmpLabel;
  bool isF = (fieldsSet != FolderFieldsSet);

  QGridLayout *grid = new QGridLayout();
  vbox->addLayout(grid);

  m_title = new KLineEdit( main );
  tmpLabel = new QLabel( main );
  tmpLabel->setText( i18n( "Name:" ) );
  tmpLabel->setBuddy( m_title );
  grid->addWidget( m_title, 0, 1 );
  grid->addWidget( tmpLabel, 0, 0 );
  m_title->setFocus();
  if (isF)
  {
    m_url = new KLineEdit( main );
    tmpLabel = new QLabel( main );
    tmpLabel->setText( i18n( "Location:" ) );
    tmpLabel->setBuddy( m_url );
    grid->addWidget( m_url, 1, 1 );
    grid->addWidget( tmpLabel, 1, 0 );
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

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidget * tree)
    : QTreeWidgetItem(tree), m_address("") //TODO KBookmarkAddress::root()
{
    setText(0, i18n("Bookmarks"));
    setIcon(0, SmallIcon("bookmark"));
    tree->expandItem(this);
    tree->setCurrentItem( this );
    tree->setItemSelected( this, true );
}

KBookmarkTreeItem::KBookmarkTreeItem(QTreeWidgetItem * parent, QTreeWidget * tree, KBookmarkGroup bk)
    : QTreeWidgetItem(parent)
{
    setIcon(0, SmallIcon(bk.icon()));
    setText(0, bk.fullText() );
    tree->expandItem(this);
    m_address = bk.address();
    kDebug()<<"** setting address to "<<bk.address()<<endl;
}

KBookmarkTreeItem::~KBookmarkTreeItem()
{
}

QString KBookmarkTreeItem::address()
{
    return m_address;
}

KBookmarkEditDialog::KBookmarkEditDialog(const QString& title, const QString& url, KBookmarkManager * mgr, BookmarkEditType editType, const QString& address,
                                         QWidget * parent, const QString& caption )
  : KDialog(parent),
    m_folderTree(0), m_mgr(mgr), m_editType(editType)
{
  setCaption( caption );

  if ( editType == InsertionMode ) {
    setButtons( User1 | Ok | Cancel );
    setButtonGuiItem( KDialog::Ok,  KGuiItem( i18n( "&Add" ), "bookmark_add") );
  } else {
    setButtons( Ok | Cancel );
    setButtonGuiItem( KDialog::Ok, i18n( "&Update" ) );
  }

  setDefaultButton( KDialog::Ok );
  if (editType == InsertionMode) {
    setButtonGuiItem( User1, KGuiItem( i18n( "&New Folder..." ), "folder_new") );
  }

  bool folder = url.isNull();

  m_main = new QWidget( this );
  setMainWidget( m_main );

  QBoxLayout *vbox = new QVBoxLayout( m_main );
  vbox->setMargin( 0 );
  vbox->setSpacing( spacingHint() );
  KBookmarkEditFields::FieldsSet fs =
    folder ? KBookmarkEditFields::FolderFieldsSet
           : KBookmarkEditFields::BookmarkFieldsSet;
  m_fields = new KBookmarkEditFields(m_main, vbox, fs);
  m_fields->setName(title);
  if ( !folder )
    m_fields->setLocation(url);

  if ( editType == InsertionMode )
  {
    m_folderTree = new QTreeWidget(m_main);
    m_folderTree->setColumnCount(1);
    m_folderTree->header()->hide();
    m_folderTree->setSortingEnabled(false);
    m_folderTree->setSelectionMode( QTreeWidget::SingleSelection );
    m_folderTree->setSelectionBehavior( QTreeWidget::SelectRows );
    m_folderTree->setMinimumSize( 60, 100 );

    vbox->addWidget(m_folderTree);

    QTreeWidgetItem *root = new KBookmarkTreeItem(m_folderTree);

    fillGroup( m_folderTree, root, mgr->root(), address );

    connect( m_folderTree, SIGNAL( itemDoubleClicked(QTreeWidgetItem *, int) ),
             this,         SLOT( slotDoubleClicked(QTreeWidgetItem *) ) );
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotUser1() ) );
  }
}

void KBookmarkEditDialog::slotDoubleClicked( QTreeWidgetItem * item )
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
  return dynamic_cast<KBookmarkTreeItem *>(m_folderTree->selectedItems().first())->address();
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
  // kDebug(7043) << "KBookmarkEditDialog::slotUser1" << endl;
  Q_ASSERT( m_folderTree );

  QString address = finalAddress();
  KBookmarkGroup bm = m_mgr->findByAddress( address ).toGroup();
  Q_ASSERT(!bm.isNull());
  Q_ASSERT(m_editType == InsertionMode);

  KBookmarkGroup group = bm.createNewFolder( m_mgr );
  if ( !group.isNull() )
  {
     KBookmarkGroup parentGroup = group.parentGroup();
     m_mgr->emitChanged( parentGroup );
     m_folderTree->clear();
     QTreeWidgetItem *root = new KBookmarkTreeItem(m_folderTree);
     fillGroup( m_folderTree, root, m_mgr->root(), address );
   }
}

void KBookmarkEditDialog::fillGroup( QTreeWidget* view, QTreeWidgetItem * parentItem, KBookmarkGroup group, const QString& address)
{
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    if ( bk.isGroup() )
    {
      KBookmarkGroup childGrp = bk.toGroup();
      QTreeWidgetItem* item = new KBookmarkTreeItem(parentItem, view, childGrp );
      fillGroup( view, item, childGrp , address );
      if (childGrp.address() == address)
      {
        view->setCurrentItem( item );
        view->scrollToItem( item );
      }
    }
  }
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
  connect(action, SIGNAL( triggered(Qt::MouseButtons, Qt::KeyboardModifiers) ),
          m_menu, SLOT( slotBookmarkSelected(Qt::MouseButtons, Qt::KeyboardModifiers) ));
  action->setProperty( "url", url );
  action->setToolTip( url );
  mstack.top()->m_parentMenu->addAction(action);
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuNSImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = KStringHandler::csqueeze(text);
  _text.replace( '&', "&&" );
  KActionMenu * actionMenu = new KActionMenu( KIcon("folder"), _text, m_actionCollection, 0L );
  mstack.top()->m_parentMenu->addAction(actionMenu);
  mstack.top()->m_actions.append( actionMenu );
  KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_menu->m_pOwner, actionMenu->menu(),
                                              m_actionCollection, false,
                                              m_menu->m_bAddBookmark, QString() );
  connect( subMenu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ),
           m_menu, SIGNAL( openBookmark( const QString &, Qt::MouseButtons, Qt::KeyboardModifiers ) ));
  mstack.top()->m_lstSubMenus.append( subMenu );

  mstack.push(subMenu);
}

void KBookmarkMenuNSImporter::newSeparator()
{
  mstack.top()->m_parentMenu->addSeparator();
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
