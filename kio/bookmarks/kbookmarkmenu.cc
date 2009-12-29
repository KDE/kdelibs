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
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandardshortcut.h>
#include <kstandardaction.h>
#include <kstringhandler.h>
#include <krun.h>
#include <kactioncollection.h>

#include <qclipboard.h>
#include <qmimedata.h>


#include <QtCore/QStack>
#include <QtGui/QHeaderView>
#include <QtGui/QApplication>

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

    KAction *newBookmarkFolder;
    KAction *addAddBookmark;
    KAction *bookmarksToFolder;
};


KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, KMenu * _parentMenu,
                              KActionCollection * actionCollection)
  : QObject(),
    m_actionCollection( actionCollection ),
    d (new KBookmarkMenuPrivate()),
    m_bIsRoot(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_parentAddress( QString("") ) //TODO KBookmarkAdress::root
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  //kDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress;

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
                              KBookmarkOwner * _owner, KMenu * _parentMenu,
                              const QString & parentAddress)
  : QObject(),
    m_actionCollection( new KActionCollection(this) ),
    d (new KBookmarkMenuPrivate()),
    m_bIsRoot(false),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_parentAddress( parentAddress )
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );
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
    KMenu * menu = contextMenu(action);
    if(!menu)
        return;
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->popup(m_parentMenu->mapToGlobal(pos));
}

KMenu * KBookmarkMenu::contextMenu( QAction * action )
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

KMenu * KBookmarkMenu::parentMenu() const
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
    : KMenu(parent), bm(bk), m_pManager(manager), m_pOwner(owner)
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
      addAction( KIcon("bookmark-new"), i18n( "Add Bookmark Here" ), this, SLOT(slotInsert()) );
}

void KBookmarkContextMenu::addFolderActions()
{
  addAction( i18n( "Open Folder in Bookmark Editor" ), this, SLOT(slotEditAt()) );
  addProperties();
  addSeparator();
  addAction( KIcon("edit-delete"), i18n( "Delete Folder" ), this, SLOT(slotRemove()) );
}


void KBookmarkContextMenu::addProperties()
{
  addAction( i18n( "Properties" ), this, SLOT(slotProperties()) );
}

void KBookmarkContextMenu::addBookmarkActions()
{
  addAction( i18n( "Copy Link Address" ), this, SLOT(slotCopyLocation()) );
  addProperties();
  addSeparator();
  addAction( KIcon("edit-delete"), i18n( "Delete Bookmark" ), this, SLOT(slotRemove()) );
}

void KBookmarkContextMenu::addOpenFolderInTabs()
{
   if(m_pOwner->supportsTabs())
      addAction(KIcon("tab-new"), i18n( "Open Folder in Tabs" ), this, SLOT( slotOpenFolderInTabs() ) );
}

void KBookmarkContextMenu::slotEditAt()
{
  //kDebug(7043) << "KBookmarkMenu::slotEditAt" << m_highlightedAddress;
  m_pManager->slotEditBookmarksAtAddress( bm.address() );
}

void KBookmarkContextMenu::slotProperties()
{
  //kDebug(7043) << "KBookmarkMenu::slotProperties" << m_highlightedAddress;

    KBookmarkDialog *  dlg = m_pOwner->bookmarkDialog(m_pManager, QApplication::activeWindow());
    dlg->editBookmark(bm);
    delete dlg;
}

void KBookmarkContextMenu::slotInsert()
{
  //kDebug(7043) << "KBookmarkMenu::slotInsert" << m_highlightedAddress;

  QString url = m_pOwner->currentUrl();
  if (url.isEmpty())
  {
    KMessageBox::error( QApplication::activeWindow(), i18n("Cannot add bookmark with empty URL."));
    return;
  }
  QString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  if (bm.isGroup())
  {
    KBookmarkGroup parentBookmark = bm.toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    parentBookmark.addBookmark( title, KUrl(url) );
    m_pManager->emitChanged( parentBookmark );
  }
  else
  {
    KBookmarkGroup parentBookmark = bm.parentGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmark newBookmark = parentBookmark.addBookmark( title, KUrl(m_pOwner->currentUrl()) );
    parentBookmark.moveBookmark( newBookmark, parentBookmark.previous(bm) );
    m_pManager->emitChanged( parentBookmark );
  }
}

void KBookmarkContextMenu::slotRemove()
{
  //kDebug(7043) << "KBookmarkMenu::slotRemove" << m_highlightedAddress;

  bool folder = bm.isGroup();

  if (KMessageBox::warningContinueCancel(
          QApplication::activeWindow(),
          folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?", bm.text())
                 : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?", bm.text()),
          folder ? i18n("Bookmark Folder Deletion")
                 : i18n("Bookmark Deletion"),
          KStandardGuiItem::del())
        != KMessageBox::Continue
     )
    return;

  KBookmarkGroup parentBookmark = bm.parentGroup();
  parentBookmark.deleteBookmark( bm );
  m_pManager->emitChanged( parentBookmark );
}

void KBookmarkContextMenu::slotCopyLocation()
{
  //kDebug(7043) << "KBookmarkMenu::slotCopyLocation" << m_highlightedAddress;

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
  kDebug(7043)<<"KBookmarkMenu::slotBookmarksChanged( "<<groupAddress;
  if ( groupAddress == m_parentAddress )
  {
    //kDebug(7043) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress;
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
  //kDebug(7043) << "KBookmarkMenu::refill()";
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

    QString title = i18n( "Open Folder in Tabs" );

    KAction * paOpenFolderInTabs = new KAction( title, this );
    paOpenFolderInTabs->setIcon( KIcon("tab-new") );
    paOpenFolderInTabs->setHelpText( i18n( "Open all bookmarks in this folder as a new tab." ) );
    connect( paOpenFolderInTabs, SIGNAL( triggered( bool ) ), this, SLOT( slotOpenFolderInTabs() ) );

    m_parentMenu->addAction(paOpenFolderInTabs);
    m_actions.append( paOpenFolderInTabs );
}

void KBookmarkMenu::addAddBookmarksList()
{
    if( !m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !m_pOwner->supportsTabs() || !KAuthorized::authorizeKAction("bookmarks") )
        return;

    if (d->bookmarksToFolder == 0) {
        QString title = i18n( "Bookmark Tabs as Folder..." );
        d->bookmarksToFolder = new KAction( title, this );
        m_actionCollection->addAction( m_bIsRoot ? "add_bookmarks_list" : 0, d->bookmarksToFolder);
        d->bookmarksToFolder->setIcon( KIcon( "bookmark-new-list" ) );
        d->bookmarksToFolder->setHelpText( i18n( "Add a folder of bookmarks for all open tabs." ) );
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

  KAction * m_paEditBookmarks = m_actionCollection->addAction(KStandardAction::EditBookmarks, "edit_bookmarks",
                                                              m_pManager, SLOT(slotEditBookmarks()));
  m_parentMenu->addAction(m_paEditBookmarks);
  m_paEditBookmarks->setHelpText( i18n( "Edit your bookmark collection in a separate window" ) );
}

void KBookmarkMenu::addNewFolder()
{
    if( !m_pOwner || !m_pOwner->enableOption(KBookmarkOwner::ShowAddBookmark) || !KAuthorized::authorizeKAction("bookmarks"))
        return;

    if (d->newBookmarkFolder == 0) {
        d->newBookmarkFolder = new KAction( i18n( "New Bookmark Folder..." ), this );
        d->newBookmarkFolder->setIcon( KIcon( "folder-new" ) );
        d->newBookmarkFolder->setHelpText( i18n( "Create a new bookmark folder in this menu" ) );
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
    //kDebug(7043) << "Creating bookmark submenu named " << bm.text();
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
    //kDebug(7043) << "Creating bookmark menu item for " << bm.text();
    KAction * action = new KBookmarkAction( bm, m_pOwner, this );
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
      dlg->addBookmark(m_pOwner->currentTitle(), KUrl(m_pOwner->currentUrl()), parentBookmark );
      delete dlg;
  }
  else
  {
      parentBookmark.addBookmark(m_pOwner->currentTitle(), KUrl(m_pOwner->currentUrl()));
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
  kDebug(7043)<<"**** slotNSLoad  ****"<<m_type<<"  "<<m_location;
  // only fill menu once
  parentMenu()->disconnect(SIGNAL(aboutToShow()));

  // not NSImporter, but kept old name for BC reasons
  KBookmarkMenuImporter importer( manager(), this );
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
  KBookmark bm = KBookmark::standaloneBookmark(text, url, QString("html"));
  KAction * action = new KBookmarkAction(bm, mstack.top()->owner(), this);
  mstack.top()->parentMenu()->addAction(action);
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuImporter::newFolder( const QString & text, bool, const QString & )
{
  QString _text = KStringHandler::csqueeze(text).replace( '&', "&&" );
  KActionMenu * actionMenu = new KImportedBookmarkActionMenu( KIcon("folder"), _text, this );
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
  : KAction( bk.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bk),
    m_pOwner(owner)
{
  setIcon(KIcon(bookmark().icon()));
  setHelpText( bookmark().url().pathOrUrl() );
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
  : KActionMenu(KIcon(bm.icon()), bm.text().replace('&', "&&"), parent),
    KBookmarkActionInterface(bm)
{
  setToolTip( bm.description() );
}

KBookmarkActionMenu::KBookmarkActionMenu(const KBookmark &bm, const QString & text, QObject *parent)
  : KActionMenu(text, parent),
    KBookmarkActionInterface(bm)
{
}

KBookmarkActionMenu::~KBookmarkActionMenu()
{
}

#include "kbookmarkmenu.moc"
#include "kbookmarkmenu_p.moc"
