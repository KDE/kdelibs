//  -*- c-basic-offset:2; indent-tabs-mode:nil -*-
// vim: set ts=2 sts=2 sw=2 et:
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2006 Daniel Teske <teske@squorn.de>

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

#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <krun.h>
#include <kseparatoraction.h>
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
                              KActionCollection *)
  : QObject(),
    m_bIsRoot(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_actionCollection( 0 ),
    m_parentAddress( QString("") ) //TODO KBookmarkAdress::root
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  //kDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress << endl;

  connect( _parentMenu, SIGNAL( aboutToShow() ),
            SLOT( slotAboutToShow() ) );

  if ( KBookmarkSettings::self()->m_contextmenu )
  {
    m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_parentMenu, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
  }

  connect( m_pManager, SIGNAL( changed(const QString &, const QString &) ),
              SLOT( slotBookmarksChanged(const QString &) ) );

  m_bDirty = true;

  // add entries that possibly have a shortcut, so they are available _before_ first popup
  addAddBookmark();
  addAddBookmarksList();
  addEditBookmarks();
}

void KBookmarkMenu::addActions()
{
  if ( m_bIsRoot )
  {
    addAddBookmark();
    addAddBookmarksList();
    addEditBookmarks();
    addNewFolder();
  }
  else
  {
    if ( m_parentMenu->actions().count() > 0 )
      m_parentMenu->addSeparator();

    addAddBookmark();
    addAddBookmarksList();
    addNewFolder();
  }
}

KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, KMenu * _parentMenu,
                              const QString & parentAddress)
  : QObject(),
    m_bIsRoot(false),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_actionCollection( 0 ),
    m_parentAddress( parentAddress )
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );
  connect( _parentMenu, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
  if ( KBookmarkSettings::self()->m_contextmenu )
  {
    m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_parentMenu, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));
  }
  m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
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
    clear();
    refill();
    m_parentMenu->adjustSize();
  }
}

void KBookmarkMenu::contextMenu( const QPoint & pos )
{
  QAction * action = m_parentMenu->actionAt(pos);
  KBookmarkActionInterface* act = dynamic_cast<KBookmarkActionInterface *>(action);
  if (!act)
      return;
  act->contextMenu(m_parentMenu->mapToGlobal(pos), m_pManager, m_pOwner);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkActionInterface::KBookmarkActionInterface(KBookmark bk)
: bm(bk)
{}

KBookmarkActionInterface::~KBookmarkActionInterface()
{
}

void KBookmarkActionInterface::contextMenu(QPoint, KBookmarkManager*, KBookmarkOwner*)
{
}

const KBookmark KBookmarkActionInterface::bookmark() const
{
  return bm;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkActionContextMenu & KBookmarkActionContextMenu::self()
{
  static KBookmarkActionContextMenu s;
  return s;
}

void KBookmarkActionContextMenu::contextMenu(QPoint pos, QString highlightedAddress,
	KBookmarkManager *pManager, KBookmarkOwner *pOwner)
{
  m_parentAddress = KBookmark::parentAddress(highlightedAddress) ;
  m_highlightedAddress = highlightedAddress;
  m_pManager = pManager;
  m_pOwner = pOwner;
  delete m_contextMenu;
  m_contextMenu = new QMenu();

  KBookmark bookmark = atAddress(m_highlightedAddress);
  addBookmark();

  if (bookmark.isGroup())
    addFolderActions();
  else
    addBookmarkActions();

  addProperties();

  if(!m_highlightedAddress.isNull())
    m_contextMenu->popup(pos);
}

void KBookmarkActionContextMenu::addBookmark()
{
  if( m_pOwner && m_pOwner->addBookmarkEntry() )
    m_contextMenu->addAction( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), this, SLOT(slotInsert()) );
}

void KBookmarkActionContextMenu::addFolderActions()
{
  m_contextMenu->addAction( i18n( "Open Folder in Bookmark Editor" ), this, SLOT(slotEditAt()) );
  m_contextMenu->addSeparator();
  m_contextMenu->addAction( SmallIcon("editdelete"), i18n( "Delete Folder" ), this, SLOT(slotRemove()) );
  m_contextMenu->addSeparator();
}


void KBookmarkActionContextMenu::addProperties()
{
  m_contextMenu->addAction( i18n( "Properties" ), this, SLOT(slotProperties()) );
}

void KBookmarkActionContextMenu::addBookmarkActions()
{
  m_contextMenu->addAction( i18n( "Copy Link Address" ), this, SLOT(slotCopyLocation()) );
  m_contextMenu->addSeparator();
  m_contextMenu->addAction( SmallIcon("editdelete"), i18n( "Delete Bookmark" ), this, SLOT(slotRemove()) );
  m_contextMenu->addSeparator();
}

KBookmarkActionContextMenu::~KBookmarkActionContextMenu()
{
    delete m_contextMenu;
}

KBookmark KBookmarkActionContextMenu::atAddress(const QString & address)
{
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());
  return bookmark;
}

void KBookmarkActionContextMenu::slotEditAt()
{
  //kDebug(7043) << "KBookmarkMenu::slotEditAt" << m_highlightedAddress << endl;
  KBookmark bookmark = atAddress(m_highlightedAddress);
  m_pManager->slotEditBookmarksAtAddress( m_highlightedAddress );
}

void KBookmarkActionContextMenu::slotProperties()
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

void KBookmarkActionContextMenu::slotInsert()
{
  //kDebug(7043) << "KBookmarkMenu::slotInsert" << m_highlightedAddress << endl;

  QString url = m_pOwner->currentUrl();
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

void KBookmarkActionContextMenu::slotRemove()
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
          KStandardGuiItem::del())
        != KMessageBox::Continue
     )
    return;

  KBookmarkGroup parentBookmark = atAddress( m_parentAddress ).toGroup();
  parentBookmark.deleteBookmark( bookmark );
  m_pManager->emitChanged( parentBookmark );
  if (m_parentMenu)
    m_parentMenu->hide();
}

void KBookmarkActionContextMenu::slotCopyLocation()
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
void KBookmarkMenu::slotBookmarksChanged( const QString & groupAddress )
{
  kDebug(7043)<<"KBookmarkMenu::slotBookmarksChanged( "<<groupAddress<<endl;
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

void KBookmarkMenu::clear()
{
  qDeleteAll( m_lstSubMenus );
  m_lstSubMenus.clear();

  for ( QList<KAction *>::iterator it = m_actions.begin(), end = m_actions.end() ;
        it != end ; ++it )
  {
        m_parentMenu->removeAction(*it);
  }

  m_parentMenu->clear();
  m_actions.clear();
}

void KBookmarkMenu::refill()
{
  //kDebug(7043) << "KBookmarkMenu::refill()" << endl;
  if(m_bIsRoot)
    addActions();
  fillBookmarks();
  if(!m_bIsRoot)
    addActions();
}

void KBookmarkMenu::addAddBookmarksList()
{
  if( !m_pOwner || !m_pOwner->addBookmarkEntry() || !m_pOwner->supportsTabs() || !KAuthorized::authorizeKAction("bookmarks") )
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
  if( !m_pOwner || !m_pOwner->addBookmarkEntry() || !KAuthorized::authorizeKAction("bookmarks") )
    return;

  QString title = i18n( "Add Bookmark" );

  KAction * paAddBookmarks = new KAction( title, m_actionCollection, m_bIsRoot ? "add_bookmark" : 0 );
  paAddBookmarks->setIcon( KIcon( "bookmark_add" ) );
  paAddBookmarks->setShortcut( m_bIsRoot ? KStandardShortcut::addBookmark() : KShortcut() );
  paAddBookmarks->setToolTip( i18n( "Add a bookmark for the current document" ) );
  connect( paAddBookmarks, SIGNAL( triggered( bool ) ), this, SLOT( slotAddBookmark() ) );

  m_parentMenu->addAction(paAddBookmarks);
  m_actions.append( paAddBookmarks );
}

void KBookmarkMenu::addEditBookmarks()
{
  if( m_pOwner && !m_pOwner->editBookmarkEntry() || !KAuthorized::authorizeKAction("bookmarks") )
    return;

  KAction * m_paEditBookmarks = KStandardAction::editBookmarks( m_pManager, SLOT( slotEditBookmarks() ),
                                                             m_actionCollection, "edit_bookmarks" );
  m_parentMenu->addAction(m_paEditBookmarks);
  m_paEditBookmarks->setToolTip( i18n( "Edit your bookmark collection in a separate window" ) );
  m_actions.append( m_paEditBookmarks );
}

void KBookmarkMenu::addNewFolder()
{
  if( !m_pOwner || !m_pOwner->addBookmarkEntry() || !KAuthorized::authorizeKAction("bookmarks"))
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

void KBookmarkMenu::fillBookmarks()
{
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());

  if ( m_bIsRoot && !parentBookmark.first().isNull() ) // at least one bookmark
  {
      m_parentMenu->addSeparator();
  }

  for ( KBookmark bm = parentBookmark.first(); !bm.isNull();  bm = parentBookmark.next(bm) )
  {
    m_parentMenu->addAction(actionForBookmark(bm));
  }
}

KAction* KBookmarkMenu::actionForBookmark(KBookmark bm)
{
  if ( bm.isGroup() )
  {
    //kDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
    KActionMenu * actionMenu = new KBookmarkActionMenu( bm, m_actionCollection, "kbookmarkmenu" );
    m_actions.append( actionMenu );
    KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->menu(), bm.address() );
    m_lstSubMenus.append( subMenu );
    return actionMenu;
  }
  else if ( bm.isSeparator() )
  {
    KSeparatorAction * sa = new KSeparatorAction(m_actionCollection);
    m_actions.append(sa);
    return sa;
  }
  else
  {
    //kDebug(7043) << "Creating bookmark menu item for " << bm.text() << endl;
    KAction * action = new KBookmarkAction( bm, m_actionCollection, m_pOwner );
    m_actions.append( action );
    return action;
  }
}

void KBookmarkMenu::slotAddBookmarksList()
{
  if( !m_pOwner || !m_pOwner->supportsTabs())
    return;

  const QList<QPair<QString, QString> > & list = m_pOwner->currentBookmarkList();

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_pManager );
  if ( group.isNull() )
    return; // user canceled i guess

  QList<QPair<QString, QString> >::const_iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    group.addBookmark( m_pManager, (*it).first, KUrl((*it).second) );

  m_pManager->emitChanged( parentBookmark );
}


void KBookmarkMenu::slotAddBookmark()
{
  if( !m_pOwner ) return;
  KBookmarkGroup parentBookmark;
  parentBookmark = m_pManager->addBookmarkDialog(m_pOwner->currentUrl(), m_pOwner->currentTitle(), m_parentAddress);
  if (!parentBookmark.isNull())
  {
    m_pManager->emitChanged( parentBookmark );
  }
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
                 const QString & type, const QString & location )
    :KBookmarkMenu(mgr, owner, parentMenu, QString()), m_type(type), m_location(location)
{
    connect(parentMenu, SIGNAL(aboutToShow()), this, SLOT(slotNSLoad()));
}

KImportedBookmarkMenu::KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, KMenu * parentMenu)
    :KBookmarkMenu(mgr, owner, parentMenu, QString()), m_type(QString()), m_location(QString())
{

}

KImportedBookmarkMenu::~KImportedBookmarkMenu()
{

}

void KImportedBookmarkMenu::refill()
{

}

void KImportedBookmarkMenu::clear()
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
    setButtonGuiItem( KDialog::Ok, KGuiItem(i18n( "&Update" )) );
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
  connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
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
  KBookmark bm = KBookmark::standaloneBookmark(text, url, QString("html"));
  KAction * action = new KImportedBookmarkAction(bm, m_actionCollection, mstack.top()->m_pOwner);
  mstack.top()->m_parentMenu->addAction(action);
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuNSImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = KStringHandler::csqueeze(text).replace( '&', "&&" );
  KActionMenu * actionMenu = new KImportedBookmarkActionMenu( KIcon("folder"), _text, m_actionCollection, 0L );
  mstack.top()->m_parentMenu->addAction(actionMenu);
  mstack.top()->m_actions.append( actionMenu );
  KImportedBookmarkMenu *subMenu = new KImportedBookmarkMenu( m_pManager, m_menu->m_pOwner, actionMenu->menu());
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


KBookmarkAction::KBookmarkAction(KBookmark bk, KActionCollection* parent, KBookmarkOwner* owner)
  : KAction( KStringHandler::csqueeze(bk.text()).replace('&', "&&"), parent, 0),
    KBookmarkActionInterface(bk),
    m_pOwner(owner)
{
  setIcon(KIcon(bookmark().icon()));
  setToolTip( bookmark().url().pathOrUrl() );
  connect(this, SIGNAL( triggered(Qt::MouseButtons, Qt::KeyboardModifiers) ),
     SLOT( slotSelected(Qt::MouseButtons, Qt::KeyboardModifiers) ));
}

KBookmarkAction::~KBookmarkAction()
{
}

void KBookmarkAction::contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
{
  KBookmarkActionContextMenu::self().contextMenu(pos, bookmark().address(), m_pManager, m_pOwner);
}

void KBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
  if( !m_pOwner )
    new KRun( bookmark().url() ,(QWidget*)0);
  else
    m_pOwner->openBookmark( bookmark(), mb, km );
}

KBookmarkActionMenu::KBookmarkActionMenu(KBookmark bm, KActionCollection* parent, const char* name)
  : KActionMenu(KIcon(bm.icon()), KStringHandler::csqueeze(bm.text()).replace('&', "&&"), parent, name),
    KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::KBookmarkActionMenu(KBookmark bm, const QString & text, KActionCollection* parent, const char* name)
  : KActionMenu(text, parent, name),
    KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

void KBookmarkActionMenu::contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
{
  KBookmarkActionContextMenu::self().contextMenu(pos, bookmark().address(), m_pManager, m_pOwner );
}

KImportedBookmarkAction::KImportedBookmarkAction(KBookmark bk, KActionCollection* parent, KBookmarkOwner* owner )
: KAction( KStringHandler::csqueeze(bk.text()).replace('&', "&&"), parent, 0),
  KBookmarkActionInterface(bk),
  m_pOwner(owner)
{
  setIcon(KIcon(bookmark().icon()));
  setToolTip( bookmark().url().pathOrUrl() );
  connect(this, SIGNAL( triggered(Qt::MouseButtons, Qt::KeyboardModifiers) ),
          SLOT( slotSelected(Qt::MouseButtons, Qt::KeyboardModifiers) ));
}

KImportedBookmarkAction::~KImportedBookmarkAction()
{
}

void KImportedBookmarkAction::contextMenu(QPoint pos, KBookmarkManager* m_pManager, KBookmarkOwner* m_pOwner)
{
}

void KImportedBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
  if( !m_pOwner )
    new KRun( bookmark().url() ,(QWidget*)0);
  else
    m_pOwner->openBookmark( bookmark(), mb, km );
}

#include "kbookmarkmenu.moc"
#include "kbookmarkmenu_p.moc"
