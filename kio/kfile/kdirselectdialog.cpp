/*
    Copyright (C) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdir.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qstringlist.h>
#include <qvaluestack.h>

#include <kactionclasses.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kfilespeedbar.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprotocolinfo.h>
#include <krecentdirs.h>
#include <kurl.h>
#include <kurlcompletion.h>
#include <kurlpixmapprovider.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>

#include "kfiletreeview.h"
#include "kdirselectdialog.h"

// ### add mutator for treeview!

class KDirSelectDialog::KDirSelectDialogPrivate
{
public:
    KDirSelectDialogPrivate()
    {
        urlCombo = 0L;
        branch = 0L;
        comboLocked = false;
    }

    KFileSpeedBar *speedBar;
    KHistoryCombo *urlCombo;
    KFileTreeBranch *branch;
    QString recentDirClass;
    KURL startURL;
    QValueStack<KURL> dirsToList;

    bool comboLocked : 1;
};

static KURL rootUrl(const KURL &url)
{
    KURL root = url;
    root.setPath( "/" );
    
    if (!kapp->authorizeURLAction("list", KURL(), root))
    {
        root = KURL::fromPathOrURL( QDir::homeDirPath() );
        if (!kapp->authorizeURLAction("list", KURL(), root))
        {
            root = url;
        }
    }
    return root;
}

KDirSelectDialog::KDirSelectDialog(const QString &startDir, bool localOnly,
                                   QWidget *parent, const char *name,
                                   bool modal)
    : KDialogBase( parent, name, modal, i18n("Select Folder"), Ok|Cancel),
      m_localOnly( localOnly )
{
    d = new KDirSelectDialogPrivate;
    d->branch = 0L;

    QFrame *page = makeMainWidget();
    QHBoxLayout *hlay = new QHBoxLayout( page, 0, spacingHint() );
    m_mainLayout = new QVBoxLayout();
    d->speedBar = new KFileSpeedBar( page, "speedbar" );
    connect( d->speedBar, SIGNAL( activated( const KURL& )),
             SLOT( setCurrentURL( const KURL& )) );
    hlay->addWidget( d->speedBar, 0 );
    hlay->addLayout( m_mainLayout, 1 );

    // Create dir list
    m_treeView = new KFileTreeView( page );
    m_treeView->addColumn( i18n("Folder") );
    m_treeView->setColumnWidthMode( 0, QListView::Maximum );
    m_treeView->setResizeMode( QListView::AllColumns );

    d->urlCombo = new KHistoryCombo( page, "url combo" );
    d->urlCombo->setTrapReturnKey( true );
    d->urlCombo->setPixmapProvider( new KURLPixmapProvider() );
    KURLCompletion *comp = new KURLCompletion();
    comp->setMode( KURLCompletion::DirCompletion );
    d->urlCombo->setCompletionObject( comp, true );
    d->urlCombo->setAutoDeleteCompletionObject( true );
    d->urlCombo->setDuplicatesEnabled( false );
    connect( d->urlCombo, SIGNAL( textChanged( const QString& ) ),
             SLOT( slotComboTextChanged( const QString& ) ));

    m_contextMenu = new QPopupMenu( this );
    KAction* newFolder = new KAction( i18n("New Folder..."), "folder_new", 0, this, SLOT( slotMkdir() ), this);
    newFolder->plug(m_contextMenu);
    m_contextMenu->insertSeparator();
    m_showHiddenFolders = new KToggleAction ( i18n( "Show Hidden Folders" ), 0, this,
                                        SLOT( slotShowHiddenFoldersToggled() ), this);
    m_showHiddenFolders->plug(m_contextMenu);

    d->startURL = KFileDialog::getStartURL( startDir, d->recentDirClass );
    if ( localOnly && !d->startURL.isLocalFile() )
    {
        d->startURL = KURL();
        QString docPath = KGlobalSettings::documentPath();
        if (QDir(docPath).exists())
            d->startURL.setPath( docPath );
        else
            d->startURL.setPath( QDir::homeDirPath() );
    }

    KURL root = rootUrl(d->startURL);

    m_startDir = d->startURL.url();

    d->branch = createBranch( root );

    readConfig( KGlobal::config(), "DirSelect Dialog" );

    m_mainLayout->addWidget( m_treeView, 1 );
    m_mainLayout->addWidget( d->urlCombo, 0 );

    connect( m_treeView, SIGNAL( currentChanged( QListViewItem * )),
             SLOT( slotCurrentChanged() ));
    connect( m_treeView, SIGNAL( contextMenu( KListView *, QListViewItem *, const QPoint & )),
             SLOT( slotContextMenu( KListView *, QListViewItem *, const QPoint & )));

    connect( d->urlCombo, SIGNAL( activated( const QString& )),
             SLOT( slotURLActivated( const QString& )));
    connect( d->urlCombo, SIGNAL( returnPressed( const QString& )),
             SLOT( slotURLActivated( const QString& )));

    setCurrentURL( d->startURL );
}


KDirSelectDialog::~KDirSelectDialog()
{
    delete d;
}

void KDirSelectDialog::setCurrentURL( const KURL& url )
{
    if ( !url.isValid() )
        return;

    KURL root = rootUrl(url);

    d->startURL = url;
    if ( !d->branch ||
         url.protocol() != d->branch->url().protocol() ||
         url.host() != d->branch->url().host() )
    {
        if ( d->branch )
        {
            // removing the root-item causes the currentChanged() signal to be
            // emitted, but we don't want to update the location-combo yet.
            d->comboLocked = true;
            view()->removeBranch( d->branch );
            d->comboLocked = false;
        }

        d->branch = createBranch( root );
    }

    d->branch->disconnect( SIGNAL( populateFinished( KFileTreeViewItem * )),
                           this, SLOT( slotNextDirToList( KFileTreeViewItem *)));
    connect( d->branch, SIGNAL( populateFinished( KFileTreeViewItem * )),
             SLOT( slotNextDirToList( KFileTreeViewItem * ) ));

    KURL dirToList = root;
    d->dirsToList.clear();
    QString path = url.path(+1);
    int pos = path.length();

    if ( path.isEmpty() ) // e.g. ftp://host.com/ -> just list the root dir
        d->dirsToList.push( root );

    else
    {
        while ( pos > 0 )
        {
            pos = path.findRev( '/', pos -1 );
            if ( pos >= 0 )
            {
                dirToList.setPath( path.left( pos +1 ) );
                d->dirsToList.push( dirToList );
//                 qDebug( "List: %s", dirToList.url().latin1());
            }
        }
    }

    if ( !d->dirsToList.isEmpty() )
        openNextDir( d->branch->root() );
}

void KDirSelectDialog::openNextDir( KFileTreeViewItem * /*parent*/ )
{
    if ( !d->branch )
        return;

    KURL url = d->dirsToList.pop();

    KFileTreeViewItem *item = view()->findItem( d->branch, url.path().mid(1));
    if ( item )
    {
        if ( !item->isOpen() )
            item->setOpen( true );
        else // already open -> go to next one
            slotNextDirToList( item );
    }
//     else
//         qDebug("###### openNextDir: item not found!");
}

void KDirSelectDialog::slotNextDirToList( KFileTreeViewItem *item )
{
    // scroll to make item the topmost item
    view()->ensureItemVisible( item );
    QRect r = view()->itemRect( item );
    if ( r.isValid() )
    {
        int x, y;
        view()->viewportToContents( view()->contentsX(), r.y(), x, y );
        view()->setContentsPos( x, y );
    }

    if ( !d->dirsToList.isEmpty() )
        openNextDir( item );
    else
    {
        d->branch->disconnect( SIGNAL( populateFinished( KFileTreeViewItem * )),
                               this, SLOT( slotNextDirToList( KFileTreeViewItem *)));
        view()->setCurrentItem( item );
        item->setSelected( true );
    }
}

void KDirSelectDialog::readConfig( KConfig *config, const QString& group )
{
    d->urlCombo->clear();

    KConfigGroup conf( config, group );
    d->urlCombo->setHistoryItems( conf.readPathListEntry( "History Items" ));

    QSize defaultSize( 400, 450 );
    resize( conf.readSizeEntry( "DirSelectDialog Size", &defaultSize ));
}

void KDirSelectDialog::saveConfig( KConfig *config, const QString& group )
{
    KConfigGroup conf( config, group );
    conf.writePathEntry( "History Items", d->urlCombo->historyItems(), ',',
                     true, true);
    conf.writeEntry( "DirSelectDialog Size", size(), true, true );

    d->speedBar->save( config );

    config->sync();
}

void KDirSelectDialog::accept()
{
    KFileTreeViewItem *item = m_treeView->currentKFileTreeViewItem();
    if ( !item )
        return;

    if ( !d->recentDirClass.isEmpty() )
    {
        KURL dir = item->url();
        if ( !item->isDir() )
            dir = dir.upURL();

        KRecentDirs::add(d->recentDirClass, dir.url());
    }

    d->urlCombo->addToHistory( item->url().prettyURL() );
    KFileDialog::setStartDir( url() );

    KDialogBase::accept();
    saveConfig( KGlobal::config(), "DirSelect Dialog" );
}


KURL KDirSelectDialog::url() const
{
    return m_treeView->currentURL();
}

void KDirSelectDialog::slotCurrentChanged()
{
    if ( d->comboLocked )
        return;

    KFileTreeViewItem *current = view()->currentKFileTreeViewItem();
    KURL u = current ? current->url() : (d->branch ? d->branch->rootUrl() : KURL());

    if ( u.isValid() )
    {
        if ( u.isLocalFile() )
            d->urlCombo->setEditText( u.path() );

        else // remote url
            d->urlCombo->setEditText( u.prettyURL() );
    }
    else
        d->urlCombo->setEditText( QString::null );
}

void KDirSelectDialog::slotURLActivated( const QString& text )
{
    if ( text.isEmpty() )
        return;

    KURL url = KURL::fromPathOrURL( text );
    d->urlCombo->addToHistory( url.prettyURL() );

    if ( localOnly() && !url.isLocalFile() )
        return; // ### messagebox

    KURL oldURL = m_treeView->currentURL();
    if ( oldURL.isEmpty() )
        oldURL = KURL::fromPathOrURL( m_startDir );

    setCurrentURL( url );
}

KFileTreeBranch * KDirSelectDialog::createBranch( const KURL& url )
{
    QString title = url.isLocalFile() ? url.path() : url.prettyURL();
    KFileTreeBranch *branch = view()->addBranch( url, title, m_showHiddenFolders->isChecked() );
    branch->setChildRecurse( false );
    view()->setDirOnlyMode( branch, true );

    return branch;
}

void KDirSelectDialog::slotComboTextChanged( const QString& text )
{
    if ( d->branch )
    {
        KURL url = KURL::fromPathOrURL( text );
        KFileTreeViewItem *item = d->branch->findTVIByURL( url );
        if ( item )
        {
            view()->setCurrentItem( item );
            view()->setSelected( item, true );
            view()->ensureItemVisible( item );
            return;
        }
    }

    QListViewItem *item = view()->currentItem();
    if ( item )
    {
        item->setSelected( false );
        // 2002/12/27, deselected item is not repainted, so force it
        item->repaint();
    }
}

void KDirSelectDialog::slotContextMenu( KListView *, QListViewItem *, const QPoint& pos )
{
    m_contextMenu->popup( pos );
}

void KDirSelectDialog::slotMkdir()
{
    bool ok;
    QString where = url().prettyURL( +1, KURL::StripFileProtocol );
    QString directory = KIO::encodeFileName( KInputDialog::getText( i18n( "New Folder" ),
                                         i18n( "Create new folder in:\n%1" ).arg( where ),
                                         i18n("New Folder"), &ok, this));
    if (!ok)
      return;
      
    bool selectDirectory = true;
    bool writeOk = false;
    bool exists = false;
    KURL folderurl( url() );

    QStringList dirs = QStringList::split( QDir::separator(), directory );
    QStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        folderurl.addPath( *it );
        exists = KIO::NetAccess::exists( folderurl, false, 0 );
        writeOk = !exists && KIO::NetAccess::mkdir( folderurl, topLevelWidget() );
    }

    if ( exists ) // url was already existant
    {
        QString which = folderurl.isLocalFile() ? folderurl.path() : folderurl.prettyURL();
        KMessageBox::sorry(this, i18n("A file or folder named %1 already exists.").arg(which));
        selectDirectory = false;
    }
    else if ( !writeOk ) {
        KMessageBox::sorry(this, i18n("You do not have permission to create that folder." ));
    }
    else if ( selectDirectory ) {
        setCurrentURL( folderurl );
    }
}

void KDirSelectDialog::slotShowHiddenFoldersToggled()
{
    KURL currentURL = url();

    d->comboLocked = true;
    view()->removeBranch( d->branch );
    d->comboLocked = false;

    KURL root = rootUrl(d->startURL);
    d->branch = createBranch( root );

    setCurrentURL( currentURL );
}

// static
KURL KDirSelectDialog::selectDirectory( const QString& startDir,
                                        bool localOnly,
                                        QWidget *parent,
                                        const QString& caption)
{
    KDirSelectDialog myDialog( startDir, localOnly, parent,
                               "kdirselect dialog", true );

    if ( !caption.isNull() )
        myDialog.setCaption( caption );

    if ( myDialog.exec() == QDialog::Accepted )
        return myDialog.url();
    else
        return KURL();
}

void KDirSelectDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kdirselectdialog.moc"
