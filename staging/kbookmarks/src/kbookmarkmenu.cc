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

#include "kbookmarkdialog.h"

#include <kauthorized.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <krun.h>
#include <kactioncollection.h>

#include <qclipboard.h>
#include <qmimedata.h>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtCore/QStack>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>

/********************************************************************/
/********************************************************************/
/********************************************************************/
class KBookmarkMenuPrivate
{
public:
    KBookmarkMenuPrivate()
        : newBookmarkFolder(0),
        addAddBookmark(0),
        bookmarksToFolder(0)
    {
    }

    QAction *newBookmarkFolder;
    QAction *addAddBookmark;
    QAction *bookmarksToFolder;
};


KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, QMenu * _parentMenu,
                              KActionCollection * actionCollection)
  : QObject(),
    m_actionCollection( actionCollection ),
    d (new KBookmarkMenuPrivate()),
    m_bIsRoot(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_parentAddress( QString("") ) //TODO KBookmarkAdress::root
{
  // TODO KDE5 find a QMenu equvalnet for this one
  //m_parentMenu->setKeyboardShortcutsEnabled( true );

  // qDebug() << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress;

  connect( _parentMenu, SIGNAL( aboutToShow() ),
            SLOT( slotAboutToShow() ) );

  if ( KBookmarkSettings::self()->m_contextmenu )
  {
    m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_parentMenu, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotCustomContextMenu(const QPoint &)));
  }

  connect( m_pManager, SIGNAL( changed(const QString &, const QString &) ),
              SLOT( slotBookmarksChanged(const QString &) ) );

  m_bDirty = true;
    addActions();
}

void KBookmarkMenu::addActions()
{
  if ( m_bIsRoot )
  {
    addAddBookmark();
    addAddBookmarksList();
    addNewFolder();
    addEditBookmarks();
  }
  else
  {
    if ( m_parentMenu->actions().count() > 0 )
      m_parentMenu->addSeparator();

    addOpenInTabs();
    addAddBookmark();
    addAddBookmarksList();
    addNewFolder();
  }
}

KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, QMenu * _parentMenu,
                              const QString & parentAddress)
  : QObject(),
    m_actionCollection( new KActionCollection(this) ),
    d (new KBookmarkMenuPrivate()),
    m_bIsRoot(false),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_parentAddress( parentAddress )
{
  // TODO KDE5 find a QMenu equvalnet for this one
  //m_parentMenu->setKeyboardShortcutsEnabled( true );
  connect( _parentMenu, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
  if ( KBookmarkSettings::self()->m_contextmenu )
  {
    m_parentMenu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_parentMenu, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slotCustomContextMenu(const QPoint &)));
  }
  m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
    qDeleteAll( m_lstSubMenus );
    qDeleteAll( m_actions );
    delete d;
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

void KBookmarkMenu::slotCustomContextMenu( const QPoint & pos)
{
    QAction * action = m_parentMenu->actionAt(pos);
    QMenu * menu = contextMenu(action);
    if(!menu)
        return;
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(m_parentMenu->mapToGlobal(pos));
}

QMenu * KBookmarkMenu::contextMenu( QAction * action )
{
    KBookmarkActionInterface* act = dynamic_cast<KBookmarkActionInterface *>(action);
    if (!act)
        return 0;
    return new KBookmarkContextMenu(act->bookmark(), m_pManager, m_pOwner);
}

bool KBookmarkMenu::isRoot() const
{
    return m_bIsRoot;
}

bool KBookmarkMenu::isDirty() const
{
    return m_bDirty;
}

QString KBookmarkMenu::parentAddress() const
{
    return m_parentAddress;
}

KBookmarkManager * KBookmarkMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner * KBookmarkMenu::owner() const
{
    return m_pOwner;
}

QMenu* KBookmarkMenu::parentMenu() const
{
    return m_parentMenu;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkActionInterface::KBookmarkActionInterface(const KBookmark &bk)
: bm(bk)
{}

KBookmarkActionInterface::~KBookmarkActionInterface()
{
}

const KBookmark KBookmarkActionInterface::bookmark() const
{
  return bm;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/


KBookmarkContextMenu::KBookmarkContextMenu(const KBookmark & bk, KBookmarkManager * manager, KBookmarkOwner *owner, QWidget * parent)
    : QMenu(parent), bm(bk), m_pManager(manager), m_pOwner(owner)
{
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void KBookmarkContextMenu::slotAboutToShow()
{
    addActions();
}

void KBookmarkContextMenu::addActions()
{
  if (bm.isGroup())
  {
    addOpenFolderInTabs();
    addBookmark();
    addFolderActions();
  }
  else
  {
    addBookmark();
    addBookmarkActions();
  }
}

KBookmarkContextMenu::~KBookmarkContextMenu()
{

}


void KBookmarkContextMenu::addBookmark()
{
  if( m_pOwner && m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark)  )
      addAction( QIcon::fromTheme("bookmark-new"), tr("Add Bookmark Here"), this, SLOT(slotInsert()) );
}

void KBookmarkContextMenu::addFolderActions()
{
  addAction( tr("Open Folder in Bookmark Editor" ), this, SLOT(slotEditAt()) );
  addProperties();
  addSeparator();
  addAction( QIcon::fromTheme("edit-delete"), tr( "Delete Folder" ), this, SLOT(slotRemove()) );
}


void KBookmarkContextMenu::addProperties()
{
  addAction( tr( "Properties" ), this, SLOT(slotProperties()) );
}

void KBookmarkContextMenu::addBookmarkActions()
{
  addAction( tr( "Copy Link Address" ), this, SLOT(slotCopyLocation()) );
  addProperties();
  addSeparator();
  addAction( QIcon::fromTheme("edit-delete"), tr( "Delete Bookmark" ), this, SLOT(slotRemove()) );
}

void KBookmarkContextMenu::addOpenFolderInTabs()
{
   if(m_pOwner->supportsTabs())
      addAction(QIcon::fromTheme("tab-new"), tr( "Open Folder in Tabs" ), this, SLOT( slotOpenFolderInTabs() ) );
}

void KBookmarkContextMenu::slotEditAt()
{
  // qDebug() << "KBookmarkMenu::slotEditAt" << m_highlightedAddress;
  m_pManager->slotEditBookmarksAtAddress( bm.address() );
}

void KBookmarkContextMenu::slotProperties()
{
  // qDebug() << "KBookmarkMenu::slotProperties" << m_highlightedAddress;

    KBookmarkDialog *  dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->editBookmark(bm);
    delete dlg;
}

void KBookmarkContextMenu::slotInsert()
{
  // qDebug() << "KBookmarkMenu::slotInsert" << m_highlightedAddress;

  QString url = m_pOwner->currentUrl();
  if (url.isEmpty())
  {
    QMessageBox::critical( QApplication::activeWindow(), QApplication::applicationName(), 
                           tr("Cannot add bookmark with empty URL."));
    return;
  }
  QString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  if (bm.isGroup())
  {
    KBookmarkGroup parentBookmark = bm.toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    parentBookmark.addBookmark(title, QUrl(url));
    m_pManager->emitChanged( parentBookmark );
  }
  else
  {
    KBookmarkGroup parentBookmark = bm.parentGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmark newBookmark = parentBookmark.addBookmark(title, QUrl(m_pOwner->currentUrl()));
    parentBookmark.moveBookmark( newBookmark, parentBookmark.previous(bm) );
    m_pManager->emitChanged( parentBookmark );
  }
}

void KBookmarkContextMenu::slotRemove()
{
  // qDebug() << "KBookmarkMenu::slotRemove" << m_highlightedAddress;

  bool folder = bm.isGroup();

  if (QMessageBox::warning(
          QApplication::activeWindow(),
          folder ? tr("Bookmark Folder Deletion")
                 : tr("Bookmark Deletion"),
          folder ? tr("Are you sure you wish to remove the bookmark folder\n\"%1\"?").arg(bm.text())
                 : tr("Are you sure you wish to remove the bookmark\n\"%1\"?").arg(bm.text()),
          QMessageBox::Yes | QMessageBox::Cancel)
        != QMessageBox::Yes
     )
    return;

  KBookmarkGroup parentBookmark = bm.parentGroup();
  parentBookmark.deleteBookmark( bm );
  m_pManager->emitChanged( parentBookmark );
}

void KBookmarkContextMenu::slotCopyLocation()
{
  // qDebug() << "KBookmarkMenu::slotCopyLocation" << m_highlightedAddress;

  if ( !bm.isGroup() )
  {
    QMimeData* mimeData = new QMimeData;
    bm.populateMimeData( mimeData );
    QApplication::clipboard()->setMimeData( mimeData, QClipboard::Selection );
    mimeData = new QMimeData;
    bm.populateMimeData( mimeData );
    QApplication::clipboard()->setMimeData( mimeData, QClipboard::Clipboard );
  }
}

void KBookmarkContextMenu::slotOpenFolderInTabs()
{
  owner()->openFolderinTabs(bookmark().toGroup());
}

KBookmarkManager * KBookmarkContextMenu::manager() const
{
    return m_pManager;
}

KBookmarkOwner * KBookmarkContextMenu::owner() const
{
    return m_pOwner;
}

KBookmark KBookmarkContextMenu::bookmark() const
{
    return bm;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::slotBookmarksChanged( const QString & groupAddress )
{
  qDebug()<<"KBookmarkMenu::slotBookmarksChanged( "<<groupAddress;
  if ( groupAddress == m_parentAddress )
  {
    //qDebug() << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress;
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

  for ( QList<QAction *>::iterator it = m_actions.begin(), end = m_actions.end() ;
        it != end ; ++it )
  {
        m_parentMenu->removeAction(*it);
        delete *it;
  }

  m_parentMenu->clear();
  m_actions.clear();
}

void KBookmarkMenu::refill()
{
  //qDebug() << "KBookmarkMenu::refill()";
  if(m_bIsRoot)
    addActions();
  fillBookmarks();
  if(!m_bIsRoot)
    addActions();
}

void KBookmarkMenu::addOpenInTabs()
{
    if( !m_pOwner || !m_pOwner->supportsTabs() || !KAuthorized::authorizeKAction("bookmarks") )
        return;

    QString title = tr( "Open Folder in Tabs" );

    QAction * paOpenFolderInTabs = new QAction( title, this );
    paOpenFolderInTabs->setIcon( QIcon::fromTheme("tab-new") );
    paOpenFolderInTabs->setToolTip( tr( "Open all bookmarks in this folder as a new tab." ) );
    paOpenFolderInTabs->setStatusTip(paOpenFolderInTabs->toolTip());
    connect( paOpenFolderInTabs, SIGNAL( triggered( bool ) ), this, SLOT( slotOpenFolderInTabs() ) );

    m_parentMenu->addAction(paOpenFolderInTabs);
    m_actions.append( paOpenFolderInTabs );
}

void KBookmarkMenu::addAddBookmarksList()
{
    if( !m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !m_pOwner->supportsTabs() || !KAuthorized::authorizeKAction("bookmarks") )
        return;

    if (d->bookmarksToFolder == 0) {
        QString title = tr( "Bookmark Tabs as Folder..." );
        d->bookmarksToFolder = new QAction( title, this );
        m_actionCollection->addAction( m_bIsRoot ? "add_bookmarks_list" : 0, d->bookmarksToFolder);
        d->bookmarksToFolder->setIcon( QIcon::fromTheme( "bookmark-new-list" ) );
        d->bookmarksToFolder->setToolTip( tr( "Add a folder of bookmarks for all open tabs." ) );
        d->bookmarksToFolder->setStatusTip(d->bookmarksToFolder->toolTip());
        connect( d->bookmarksToFolder, SIGNAL( triggered( bool ) ), this, SLOT( slotAddBookmarksList() ) );
    }

    m_parentMenu->addAction(d->bookmarksToFolder);
}

void KBookmarkMenu::addAddBookmark()
{
  if( !m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeKAction("bookmarks") )
    return;

    if (d->addAddBookmark == 0) {
        d->addAddBookmark = m_actionCollection->addAction(
                KStandardAction::AddBookmark,
                m_bIsRoot ? "add_bookmark" : 0,
                this,
                SLOT(slotAddBookmark()));
        if (!m_bIsRoot)
            d->addAddBookmark->setShortcut( QKeySequence() );
    }

    m_parentMenu->addAction(d->addAddBookmark);
}

void KBookmarkMenu::addEditBookmarks()
{
  if( ( m_pOwner && !m_pOwner->enableOption(KBookmarkOwner::ShowEditBookmark) ) || !KAuthorized::authorizeKAction("bookmarks") )
    return;

  QAction * m_paEditBookmarks = m_actionCollection->addAction(KStandardAction::EditBookmarks, "edit_bookmarks",
                                                              m_pManager, SLOT(slotEditBookmarks()));
  m_parentMenu->addAction(m_paEditBookmarks);
  m_paEditBookmarks->setToolTip( tr( "Edit your bookmark collection in a separate window" ) );
  m_paEditBookmarks->setStatusTip(m_paEditBookmarks->toolTip());
}

void KBookmarkMenu::addNewFolder()
{
    if( !m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeKAction("bookmarks"))
        return;

    if (d->newBookmarkFolder == 0) {
        d->newBookmarkFolder = new QAction( tr( "New Bookmark Folder..." ), this );
        d->newBookmarkFolder->setIcon( QIcon::fromTheme( "folder-new" ) );
        d->newBookmarkFolder->setToolTip( tr( "Create a new bookmark folder in this menu" ) );
        d->newBookmarkFolder->setStatusTip(d->newBookmarkFolder->toolTip());
        connect( d->newBookmarkFolder, SIGNAL( triggered( bool ) ), this, SLOT( slotNewFolder() ) );
    }

    m_parentMenu->addAction(d->newBookmarkFolder);

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

QAction* KBookmarkMenu::actionForBookmark(const KBookmark &bm)
{
  if ( bm.isGroup() )
  {
    //qDebug() << "Creating bookmark submenu named " << bm.text();
    KActionMenu * actionMenu = new KBookmarkActionMenu( bm, this );
    m_actions.append( actionMenu );
    KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->menu(), bm.address() );
    m_lstSubMenus.append( subMenu );
    return actionMenu;
  }
  else if ( bm.isSeparator() )
  {
    QAction *sa = new QAction(this);
    sa->setSeparator(true);
    m_actions.append(sa);
    return sa;
  }
  else
  {
    //qDebug() << "Creating bookmark menu item for " << bm.text();
    QAction * action = new KBookmarkAction( bm, m_pOwner, this );
    m_actions.append( action );
    return action;
  }
}

void KBookmarkMenu::slotAddBookmarksList()
{
  if( !m_pOwner || !m_pOwner->supportsTabs())
    return;

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();

  KBookmarkDialog * dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
  dlg->addBookmarks(m_pOwner->currentBookmarkList(), "", parentBookmark);
  delete dlg;
}


void KBookmarkMenu::slotAddBookmark()
{
  if( !m_pOwner ) return;
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();

  if(KBookmarkSettings::self()->m_advancedaddbookmark)
  {
      KBookmarkDialog * dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow() );
      dlg->addBookmark(m_pOwner->currentTitle(), QUrl(m_pOwner->currentUrl()), parentBookmark);
      delete dlg;
  }
  else
  {
      parentBookmark.addBookmark(m_pOwner->currentTitle(), QUrl(m_pOwner->currentUrl()));
      m_pManager->emitChanged( parentBookmark );
  }

}

void KBookmarkMenu::slotOpenFolderInTabs()
{
  m_pOwner->openFolderinTabs(m_pManager->findByAddress( m_parentAddress ).toGroup());
}

void KBookmarkMenu::slotNewFolder()
{
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkDialog * dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
  dlg->createNewFolder("", parentBookmark);
  delete dlg;
}

void KImportedBookmarkMenu::slotNSLoad()
{
  // qDebug()<<"**** slotNSLoad  ****"<<m_type<<"  "<<m_location;
  // only fill menu once
  parentMenu()->disconnect(SIGNAL(aboutToShow()));

  // not NSImporter, but kept old name for BC reasons
  KBookmarkMenuImporter importer( manager(), this );
  importer.openBookmarks(m_location, m_type);
}

KImportedBookmarkMenu::KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, QMenu * parentMenu,
                 const QString & type, const QString & location )
    :KBookmarkMenu(mgr, owner, parentMenu, QString()), m_type(type), m_location(location)
{
    connect(parentMenu, SIGNAL(aboutToShow()), this, SLOT(slotNSLoad()));
}

KImportedBookmarkMenu::KImportedBookmarkMenu( KBookmarkManager* mgr,
                 KBookmarkOwner * owner, QMenu * parentMenu)
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

void KBookmarkMenuImporter::openBookmarks( const QString &location, const QString &type )
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

void KBookmarkMenuImporter::connectToImporter(const QObject &importer)
{
  connect( &importer, SIGNAL( newBookmark( const QString &, const QString &, const QString & ) ),
           SLOT( newBookmark( const QString &, const QString &, const QString & ) ) );
  connect( &importer, SIGNAL( newFolder( const QString &, bool, const QString & ) ),
           SLOT( newFolder( const QString &, bool, const QString & ) ) );
  connect( &importer, SIGNAL( newSeparator() ), SLOT( newSeparator() ) );
  connect( &importer, SIGNAL( endFolder() ), SLOT( endFolder() ) );
}

void KBookmarkMenuImporter::newBookmark( const QString & text, const QString & url, const QString & )
{
  KBookmark bm = KBookmark::standaloneBookmark(text, QUrl(url), QString("html"));
  QAction * action = new KBookmarkAction(bm, mstack.top()->owner(), this);
  mstack.top()->parentMenu()->addAction(action);
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = KStringHandler::csqueeze(text).replace( '&', "&&" );
  KActionMenu * actionMenu = new KImportedBookmarkActionMenu( QIcon::fromTheme("folder"), _text, this );
  mstack.top()->parentMenu()->addAction(actionMenu);
  mstack.top()->m_actions.append( actionMenu );
  KImportedBookmarkMenu *subMenu = new KImportedBookmarkMenu( m_pManager, m_menu->owner(), actionMenu->menu());
  mstack.top()->m_lstSubMenus.append( subMenu );

  mstack.push(subMenu);
}

void KBookmarkMenuImporter::newSeparator()
{
  mstack.top()->parentMenu()->addSeparator();
}

void KBookmarkMenuImporter::endFolder()
{
  mstack.pop();
}

/********************************************************************/
/********************************************************************/
/********************************************************************/


KBookmarkAction::KBookmarkAction(const KBookmark &bk, KBookmarkOwner* owner, QObject *parent )
  : QAction( bk.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bk),
    m_pOwner(owner)
{
  setIcon(QIcon::fromTheme(bookmark().icon()));
  setIconText(text());
  setToolTip(bookmark().url().toDisplayString(QUrl::PreferLocalFile));
  setStatusTip(toolTip());
  setWhatsThis(toolTip());
  const QString description = bk.description();
  if (!description.isEmpty())
    setToolTip( description );
  connect(this, SIGNAL( triggered(Qt::MouseButtons, Qt::KeyboardModifiers) ),
     SLOT( slotSelected(Qt::MouseButtons, Qt::KeyboardModifiers) ));
}

KBookmarkAction::~KBookmarkAction()
{
}

void KBookmarkAction::slotSelected(Qt::MouseButtons mb, Qt::KeyboardModifiers km)
{
  if( !m_pOwner )
    new KRun( bookmark().url() ,(QWidget*)0);
  else
    m_pOwner->openBookmark( bookmark(), mb, km );
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, QObject *parent)
  : KActionMenu(QIcon::fromTheme(bm.icon()), bm.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bm)
{
  setToolTip( bm.description() );
  setIconText(text());
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, const QString & text, QObject *parent)
  : KActionMenu(text, parent),
    KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

#include "moc_kbookmarkmenu.cpp"
#include "moc_kbookmarkmenu_p.cpp"
