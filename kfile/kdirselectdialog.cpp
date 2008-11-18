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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdirselectdialog.h"

#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtGui/QLayout>
#include <QtGui/QMenu>

#include <kactioncollection.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <khistorycombobox.h>
#include <kfiledialog.h>
#include <kfiletreeview.h>
#include <kfileitemdelegate.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kinputdialog.h>
#include <kio/netaccess.h>
#include <kio/renamedialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <krecentdirs.h>
#include <ktoggleaction.h>
#include <kurlcompletion.h>
#include <kurlpixmapprovider.h>

#include <kdebug.h>

#include "kfileplacesview.h"
#include "kfileplacesmodel.h"
// ### add mutator for treeview!

class KDirSelectDialog::Private
{
public:
    Private( bool localOnly, KDirSelectDialog *parent )
        : m_parent( parent ),
          m_localOnly( localOnly ),
          m_comboLocked( false ),
          m_urlCombo(0)
    {
    }

    void readConfig(const KSharedConfigPtr &config, const QString& group);
    void saveConfig(KSharedConfigPtr config, const QString& group);
    void slotMkdir();

    void _k_slotCurrentChanged();
    void _k_slotExpand(const QModelIndex&);
    void _k_slotUrlActivated(const QString&);
    void _k_slotComboTextChanged(const QString&);
    void _k_slotContextMenu(const QPoint&);
    void _k_slotUser1();

    KDirSelectDialog *m_parent;
    bool m_localOnly : 1;
    bool m_comboLocked : 1;
    KUrl m_rootUrl;
    KUrl m_startDir;
    KFileTreeView *m_treeView;
    QMenu *m_contextMenu;
    KActionCollection *m_actions;
    KFilePlacesView *m_placesView;
    KHistoryComboBox *m_urlCombo;
    QString m_recentDirClass;
    KUrl m_startURL;

};

void KDirSelectDialog::Private::readConfig(const KSharedConfig::Ptr &config, const QString& group)
{
    m_urlCombo->clear();

    KConfigGroup conf( config, group );
    m_urlCombo->setHistoryItems( conf.readPathEntry( "History Items", QStringList() ));

    m_parent->resize( conf.readEntry( "DirSelectDialog Size", QSize( 400, 450 ) ) );
}

void KDirSelectDialog::Private::saveConfig(KSharedConfig::Ptr config, const QString& group)
{
    KConfigGroup conf( config, group );
    KConfigGroup::WriteConfigFlags flags(KConfigGroup::Persistent|KConfigGroup::Global);
    conf.writePathEntry( "History Items", m_urlCombo->historyItems(), flags );
    conf.writeEntry( "DirSelectDialog Size", m_parent->size(), flags );

    config->sync();
}

void KDirSelectDialog::Private::slotMkdir()
{
    bool ok;
    QString where = m_parent->url().pathOrUrl();
    QString name = i18n( "New Folder" );
    if ( m_parent->url().isLocalFile() && QFileInfo( m_parent->url().path(KUrl::AddTrailingSlash) + name ).exists() )
        name = KIO::RenameDialog::suggestName( m_parent->url(), name );

    QString directory = KIO::encodeFileName( KInputDialog::getText( i18nc("@title:window", "New Folder" ),
                                         i18nc("@label:textbox", "Create new folder in:\n%1" ,  where ),
                                         name, &ok, m_parent));
    if (!ok)
      return;

    bool selectDirectory = true;
    bool writeOk = false;
    bool exists = false;
    KUrl folderurl( m_parent->url() );

    const QStringList dirs = directory.split( QDir::separator(), QString::SkipEmptyParts );
    QStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        folderurl.addPath( *it );
        exists = KIO::NetAccess::exists( folderurl, KIO::NetAccess::DestinationSide, 0 );
        writeOk = !exists && KIO::NetAccess::mkdir( folderurl, m_parent->topLevelWidget() );
    }

    if ( exists ) // url was already existent
    {
        QString which = folderurl.isLocalFile() ? folderurl.path() : folderurl.prettyUrl();
        KMessageBox::sorry(m_parent, i18n("A file or folder named %1 already exists.", which));
        selectDirectory = false;
    }
    else if ( !writeOk ) {
        KMessageBox::sorry(m_parent, i18n("You do not have permission to create that folder." ));
    }
    else if ( selectDirectory ) {
        m_parent->setCurrentUrl( folderurl );
    }
}

void KDirSelectDialog::Private::_k_slotCurrentChanged()
{
    if ( m_comboLocked )
        return;

    const KUrl u = m_treeView->currentUrl();

    if ( u.isValid() )
    {
        if ( u.isLocalFile() )
            m_urlCombo->setEditText( u.path() );

        else // remote url
            m_urlCombo->setEditText( u.prettyUrl() );
    }
    else
        m_urlCombo->setEditText( QString() );
}

void KDirSelectDialog::Private::_k_slotUrlActivated( const QString& text )
{
    if ( text.isEmpty() )
        return;

    KUrl url( text );
    m_urlCombo->addToHistory( url.prettyUrl() );

    if ( m_parent->localOnly() && !url.isLocalFile() )
        return; // ### messagebox

    KUrl oldUrl = m_treeView->currentUrl();
    if ( oldUrl.isEmpty() )
        oldUrl = m_startDir;

    m_parent->setCurrentUrl( oldUrl );
}

void KDirSelectDialog::Private::_k_slotComboTextChanged( const QString& text )
{
    m_treeView->blockSignals(true);
    m_treeView->setCurrentUrl( KUrl( text ) );
    m_treeView->blockSignals(false);
}

void KDirSelectDialog::Private::_k_slotContextMenu( const QPoint& pos )
{
    m_contextMenu->popup( pos );
}

void KDirSelectDialog::Private::_k_slotExpand(const QModelIndex &index)
{
    m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
}

void KDirSelectDialog::Private::_k_slotUser1()
{
    slotMkdir();
}




KDirSelectDialog::KDirSelectDialog(const KUrl &startDir, bool localOnly,
                                   QWidget *parent)
#ifdef Q_WS_WIN
    : KDialog( parent , Qt::WindowMinMaxButtonsHint),
#else
    : KDialog( parent ),
#endif
      d( new Private( localOnly, this ) )
{
    setCaption( i18nc("@title:window","Select Folder") );
    setButtons( Ok | Cancel | User1 );
    setButtonGuiItem( User1, KGuiItem( i18nc("@action:button","New Folder..."), "folder-new" ) );
    showButtonSeparator(false);
    setDefaultButton(Ok);

    QFrame *page = new QFrame(this);
    setMainWidget(page);
    QHBoxLayout *hlay = new QHBoxLayout( page);
    hlay->setMargin(0);
    hlay->setSpacing(spacingHint());
    QVBoxLayout *mainLayout = new QVBoxLayout();
    d->m_actions=new KActionCollection(this);
    d->m_placesView = new KFilePlacesView( page );
    d->m_placesView->setModel(new KFilePlacesModel(d->m_placesView));
    d->m_placesView->setObjectName( QLatin1String( "speedbar" ) );
    d->m_placesView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->m_placesView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect( d->m_placesView, SIGNAL( urlChanged( const KUrl& )),
             SLOT( setCurrentUrl( const KUrl& )) );
    hlay->addWidget( d->m_placesView );
    hlay->addLayout( mainLayout );

    d->m_treeView = new KFileTreeView(page);
    d->m_treeView->setDirOnlyMode(true);

    for (int i = 1; i < d->m_treeView->model()->columnCount(); ++i)
        d->m_treeView->hideColumn(i);

    d->m_urlCombo = new KHistoryComboBox( page);
    d->m_urlCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    d->m_urlCombo->setTrapReturnKey( true );
    d->m_urlCombo->setPixmapProvider( new KUrlPixmapProvider() );
    KUrlCompletion *comp = new KUrlCompletion();
    comp->setMode( KUrlCompletion::DirCompletion );
    d->m_urlCombo->setCompletionObject( comp, true );
    d->m_urlCombo->setAutoDeleteCompletionObject( true );
    d->m_urlCombo->setDuplicatesEnabled( false );

    d->m_contextMenu = new QMenu( this );
    KAction* newFolder = new KAction( i18nc("@action:inmenu","New Folder..."), this);
    d->m_actions->addAction(newFolder->objectName(), newFolder);
    newFolder->setIcon( KIcon( "folder-new" ) );
    connect( newFolder, SIGNAL( triggered( bool ) ), this, SLOT( _k_slotUser1() ) );
    d->m_contextMenu->addAction( newFolder );
    d->m_contextMenu->addSeparator();

    KToggleAction *action = new KToggleAction( i18nc("@option:check", "Show Hidden Folders" ), this );
    d->m_actions->addAction( action->objectName(), action );
    connect( action, SIGNAL( triggered( bool ) ), d->m_treeView, SLOT( setShowHiddenFiles( bool ) ) );
    d->m_contextMenu->addAction( action );

    d->m_startURL = KFileDialog::getStartUrl( startDir, d->m_recentDirClass );
    if ( localOnly && !d->m_startURL.isLocalFile() )
    {
        d->m_startURL = KUrl();
        QString docPath = KGlobalSettings::documentPath();
        if (QDir(docPath).exists())
            d->m_startURL.setPath( docPath );
        else
            d->m_startURL.setPath( QDir::homePath() );
    }

    d->m_startDir = d->m_startURL;
    d->m_rootUrl = d->m_treeView->rootUrl();

    d->readConfig( KGlobal::config(), "DirSelect Dialog" );

    mainLayout->addWidget( d->m_treeView, 1 );
    mainLayout->addWidget( d->m_urlCombo, 0 );

    connect( d->m_treeView, SIGNAL( currentChanged(const KUrl&)),
             SLOT( _k_slotCurrentChanged() ));
    connect( d->m_treeView, SIGNAL( activated(const QModelIndex&)),
             SLOT( _k_slotExpand(const QModelIndex&) ));
    connect( d->m_treeView, SIGNAL( customContextMenuRequested( const QPoint & )),
             SLOT( _k_slotContextMenu( const QPoint & )));

    connect( d->m_urlCombo, SIGNAL( editTextChanged( const QString& ) ),
             SLOT( _k_slotComboTextChanged( const QString& ) ));
    connect( d->m_urlCombo, SIGNAL( activated( const QString& )),
             SLOT( _k_slotUrlActivated( const QString& )));
    connect( d->m_urlCombo, SIGNAL( returnPressed( const QString& )),
             SLOT( _k_slotUrlActivated( const QString& )));

    connect(this, SIGNAL(user1Clicked()), this, SLOT(_k_slotUser1()));

    setCurrentUrl(d->m_startURL);
}


KDirSelectDialog::~KDirSelectDialog()
{
    delete d;
}

KUrl KDirSelectDialog::url() const
{
    KUrl comboUrl(d->m_urlCombo->currentText());
    if (comboUrl.isValid()) {
        return comboUrl;
    }
    kDebug() << comboUrl.path() << " is not valid";
    return d->m_treeView->currentUrl();
}

QAbstractItemView* KDirSelectDialog::view() const
{
    return d->m_treeView;
}

bool KDirSelectDialog::localOnly() const
{
    return d->m_localOnly;
}

KUrl KDirSelectDialog::startDir() const
{
    return d->m_startDir;
}

void KDirSelectDialog::setCurrentUrl( const KUrl& url )
{
    if ( !url.isValid() )
        return;

    if (url.protocol() != d->m_rootUrl.protocol()) {
        KUrl u( url );
        u.cd("/");//NOTE portability?
        d->m_treeView->setRootUrl( u );
        d->m_rootUrl = u;
    }

    d->m_treeView->setCurrentUrl( url );
}

void KDirSelectDialog::accept()
{
    const KUrl selectedUrl = d->m_treeView->selectedUrl();
    if (!selectedUrl.isValid())
        return;

    if ( !d->m_recentDirClass.isEmpty() )
    {
        KRecentDirs::add(d->m_recentDirClass, selectedUrl.url());
    }

    d->m_urlCombo->addToHistory( selectedUrl.prettyUrl() );
    KFileDialog::setStartDir( url() );

    KDialog::accept();
}

void KDirSelectDialog::hideEvent( QHideEvent *event )
{
    d->saveConfig( KGlobal::config(), "DirSelect Dialog" );

    KDialog::hideEvent(event);
}

// static
KUrl KDirSelectDialog::selectDirectory( const KUrl& startDir,
                                        bool localOnly,
                                        QWidget *parent,
                                        const QString& caption)
{
    KDirSelectDialog myDialog( startDir, localOnly, parent);

    if ( !caption.isNull() )
        myDialog.setCaption( caption );

    if ( myDialog.exec() == QDialog::Accepted )
        return KIO::NetAccess::mostLocalUrl(myDialog.url(),parent);
    else
        return KUrl();
}

#include "kdirselectdialog.moc"
