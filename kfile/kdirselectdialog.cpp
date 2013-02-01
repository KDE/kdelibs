/*
    Copyright (C) 2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>
    Copyright (C) 2009 Shaun Reich <shaun.reich@kdemail.net>

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

#include <QDialogButtonBox>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QLayout>
#include <qinputdialog.h>
#include <qpushbutton.h>
#include <qurlpathinfo.h>

#include <kactioncollection.h>
#include <kcoreauthorized.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <khistorycombobox.h>
#include <kfiledialog.h>
#include <kfiletreeview.h>
#include <kfileitemdelegate.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kio/netaccess.h>
#include <kio/renamedialog.h>
#include <jobuidelegate.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <krecentdirs.h>
#include <ktoggleaction.h>
#include <kurlcompletion.h>
#include <kurlpixmapprovider.h>
#include <kdebug.h>
#include <kpropertiesdialog.h>
#include <kmenu.h>

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

    void slotCurrentChanged();
    void slotExpand(const QModelIndex&);
    void slotUrlActivated(const QString&);
    void slotComboTextChanged(const QString&);
    void slotContextMenuRequested(const QPoint&);
    void slotNewFolder();
    void slotMoveToTrash();
    void slotDelete();
    void slotProperties();

    KDirSelectDialog *m_parent;
    bool m_localOnly : 1;
    bool m_comboLocked : 1;
    KUrl m_rootUrl;
    KUrl m_startDir;
    KFileTreeView *m_treeView;
    KMenu *m_contextMenu;
    KActionCollection *m_actions;
    KFilePlacesView *m_placesView;
    KHistoryComboBox *m_urlCombo;
    QString m_recentDirClass;
    KUrl m_startURL;
    KAction* moveToTrash;
    KAction* deleteAction;
    KAction* showHiddenFoldersAction;
};

void KDirSelectDialog::Private::readConfig(const KSharedConfig::Ptr &config, const QString& group)
{
    m_urlCombo->clear();

    KConfigGroup conf( config, group );
    m_urlCombo->setHistoryItems( conf.readPathEntry( "History Items", QStringList() ));

    const QSize size = conf.readEntry("DirSelectDialog Size", QSize());
    if (size.isValid()) {
        m_parent->resize(size);
    }
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
    QString where = m_parent->url().toDisplayString(QUrl::PreferLocalFile);
    QString name = i18nc("folder name", "New Folder" );
    if (m_parent->url().isLocalFile() && QFileInfo(m_parent->url().toLocalFile() + '/' + name).exists())
        name = KIO::RenameDialog::suggestName( m_parent->url(), name );

    QString directory = KIO::encodeFileName( QInputDialog::getText(m_parent, i18nc("@title:window", "New Folder" ),
                                             i18nc("@label:textbox", "Create new folder in:\n%1" ,  where ),
                                             QLineEdit::Normal, name, &ok));
    if (!ok)
      return;

    bool selectDirectory = true;
    bool writeOk = false;
    bool exists = false;
    KUrl folderurl( m_parent->url() );

    const QStringList dirs = directory.split( '/', QString::SkipEmptyParts );
    QStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        folderurl.addPath( *it );
        exists = KIO::NetAccess::exists( folderurl, KIO::NetAccess::DestinationSide, m_parent );
        writeOk = !exists && KIO::NetAccess::mkdir( folderurl, m_parent );
    }

    if ( exists ) // url was already existent
    {
        QString which = folderurl.toDisplayString(QUrl::PreferLocalFile);
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

void KDirSelectDialog::Private::slotCurrentChanged()
{
    if ( m_comboLocked )
        return;

    const QUrl u = m_treeView->currentUrl();

    if ( u.isValid() ) {
        m_urlCombo->setEditText( u.toDisplayString(QUrl::PreferLocalFile) );
    }
    else
        m_urlCombo->setEditText( QString() );
}

void KDirSelectDialog::Private::slotUrlActivated( const QString& text )
{
    if ( text.isEmpty() )
        return;

    const QUrl url = QUrl::fromUserInput(text);
    m_urlCombo->addToHistory(url.toDisplayString());

    if ( m_parent->localOnly() && !url.isLocalFile() )
        return; //FIXME: messagebox for the user

    KUrl oldUrl = m_treeView->currentUrl();
    if ( oldUrl.isEmpty() )
        oldUrl = m_startDir;

    m_parent->setCurrentUrl( oldUrl );
}

void KDirSelectDialog::Private::slotComboTextChanged( const QString& text )
{
    m_treeView->blockSignals(true);
    KUrl url = QUrl::fromUserInput(text);
#ifdef Q_OS_WIN
    QUrlPathInfo rootUrlInfo(m_treeView->rootUrl());
    if(url.isLocalFile() && !rootUrlInfo.isParentOfOrEqual(url)) {
        KUrl tmp = url.upUrl();
        while(tmp.path().length() > 1) {
            url = tmp;
            tmp = url.upUrl();
        }
        m_treeView->setRootUrl( url );
    }
#endif
    m_treeView->setCurrentUrl( url );
    m_treeView->blockSignals( false );
}

void KDirSelectDialog::Private::slotContextMenuRequested( const QPoint& pos )
{
    m_contextMenu->popup( m_treeView->viewport()->mapToGlobal(pos) );
}

void KDirSelectDialog::Private::slotExpand(const QModelIndex &index)
{
    m_treeView->setExpanded(index, !m_treeView->isExpanded(index));
}

void KDirSelectDialog::Private::slotNewFolder()
{
    slotMkdir();
}

void KDirSelectDialog::Private::slotMoveToTrash()
{
    const QUrl url = m_treeView->selectedUrl();
    KIO::JobUiDelegate job;
    if (job.askDeleteConfirmation(QList<QUrl>() << url, KIO::JobUiDelegate::Trash, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::CopyJob* copyJob = KIO::trash(url);
        copyJob->ui()->setWindow(m_parent);
        copyJob->ui()->setAutoErrorHandlingEnabled(true);
    }
}

void KDirSelectDialog::Private::slotDelete()
{
    const QUrl url = m_treeView->selectedUrl();
    KIO::JobUiDelegate job;
    if (job.askDeleteConfirmation(QList<QUrl>() << url, KIO::JobUiDelegate::Delete, KIO::JobUiDelegate::DefaultConfirmation)) {
        KIO::DeleteJob* deleteJob = KIO::del(url);
        deleteJob->ui()->setWindow(m_parent);
        deleteJob->ui()->setAutoErrorHandlingEnabled(true);
    }
}

void KDirSelectDialog::Private::slotProperties()
{
    KPropertiesDialog* dialog = 0;
    dialog = new KPropertiesDialog(m_treeView->selectedUrl(), this->m_parent);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}


KDirSelectDialog::KDirSelectDialog(const QUrl &startDir, bool localOnly,
                                   QWidget *parent)
#ifdef Q_OS_WIN
    : QDialog( parent , Qt::WindowMinMaxButtonsHint),
#else
    : QDialog( parent ),
#endif
      d( new Private( localOnly, this ) )
{
    setWindowTitle( i18nc("@title:window","Select Folder") );

    QVBoxLayout *topLayout = new QVBoxLayout;
    setLayout(topLayout);

    QFrame *page = new QFrame(this);
    topLayout->addWidget(page);

    QPushButton *folderButton = new QPushButton;
    KGuiItem::assign(folderButton, KGuiItem(i18nc("@action:button","New Folder..."), "folder-new"));
    connect(folderButton, SIGNAL(clicked()), this, SLOT(slotNewFolder()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(folderButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    topLayout->addWidget(buttonBox);


    QHBoxLayout *hlay = new QHBoxLayout( page);
    hlay->setMargin(0);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    d->m_actions=new KActionCollection(this);
    d->m_actions->addAssociatedWidget(this);
    d->m_placesView = new KFilePlacesView( page );
    d->m_placesView->setModel(new KFilePlacesModel(d->m_placesView));
    d->m_placesView->setObjectName( QLatin1String( "speedbar" ) );
    d->m_placesView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->m_placesView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect( d->m_placesView, SIGNAL(urlChanged(QUrl)),
             SLOT(setCurrentUrl(QUrl)) );
    hlay->addWidget( d->m_placesView );
    hlay->addLayout( mainLayout );

    d->m_treeView = new KFileTreeView(page);
    d->m_treeView->setDirOnlyMode(true);
    d->m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    for (int i = 1; i < d->m_treeView->model()->columnCount(); ++i)
        d->m_treeView->hideColumn(i);

    d->m_urlCombo = new KHistoryComboBox( page);
    d->m_urlCombo->setLayoutDirection( Qt::LeftToRight );
    d->m_urlCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    d->m_urlCombo->setTrapReturnKey( true );
    d->m_urlCombo->setPixmapProvider( new KUrlPixmapProvider() );
    KUrlCompletion *comp = new KUrlCompletion();
    comp->setMode( KUrlCompletion::DirCompletion );
    d->m_urlCombo->setCompletionObject( comp, true );
    d->m_urlCombo->setAutoDeleteCompletionObject( true );
    d->m_urlCombo->setDuplicatesEnabled( false );

    d->m_contextMenu = new KMenu( this );

    KAction* newFolder = new KAction( i18nc("@action:inmenu","New Folder..."), this);
    d->m_actions->addAction( newFolder->objectName(), newFolder );
    newFolder->setIcon( QIcon::fromTheme( "folder-new" ) );
    newFolder->setShortcut( Qt::Key_F10);
    connect( newFolder, SIGNAL(triggered(bool)), this, SLOT(slotNewFolder()) );
    d->m_contextMenu->addAction( newFolder );

    d->moveToTrash = new KAction( i18nc( "@action:inmenu","Move to Trash" ), this );
    d->m_actions->addAction( d->moveToTrash->objectName(), d->moveToTrash );
    d->moveToTrash->setIcon( QIcon::fromTheme( "user-trash" ) );
    d->moveToTrash->setShortcut(KShortcut(Qt::Key_Delete));
    connect( d->moveToTrash, SIGNAL(triggered(bool)), this, SLOT(slotMoveToTrash()) );
    d->m_contextMenu->addAction( d->moveToTrash );

    d->deleteAction = new KAction( i18nc("@action:inmenu","Delete"), this );
    d->m_actions->addAction( d->deleteAction->objectName(), d->deleteAction );
    d->deleteAction->setIcon( QIcon::fromTheme( "edit-delete" ) );
    d->deleteAction->setShortcut( KShortcut( Qt::SHIFT + Qt::Key_Delete ) );
    connect( d->deleteAction, SIGNAL(triggered(bool)), this, SLOT(slotDelete()) );
    d->m_contextMenu->addAction( d->deleteAction );

    d->m_contextMenu->addSeparator();

    d->showHiddenFoldersAction = new KToggleAction( i18nc("@option:check", "Show Hidden Folders"), this );
    d->m_actions->addAction( d->showHiddenFoldersAction->objectName(), d->showHiddenFoldersAction );
    d->showHiddenFoldersAction->setShortcut( Qt::Key_F8 );
    connect( d->showHiddenFoldersAction, SIGNAL(triggered(bool)), d->m_treeView, SLOT(setShowHiddenFiles(bool)) );
    d->m_contextMenu->addAction( d->showHiddenFoldersAction );
    d->m_contextMenu->addSeparator();

    KAction* propertiesAction = new KAction( i18nc("@action:inmenu","Properties"), this);
    d->m_actions->addAction(propertiesAction->objectName(), propertiesAction);
    propertiesAction->setIcon(QIcon::fromTheme("document-properties"));
    propertiesAction->setShortcut(KShortcut(Qt::ALT + Qt::Key_Return));
    connect( propertiesAction, SIGNAL(triggered(bool)), this, SLOT(slotProperties()) );
    d->m_contextMenu->addAction( propertiesAction );

    d->m_startURL = KFileDialog::getStartUrl( startDir, d->m_recentDirClass );
    if ( localOnly && !d->m_startURL.isLocalFile() )
    {
        d->m_startURL = QUrl();
        QString docPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (QDir(docPath).exists())
            d->m_startURL.setPath( docPath );
        else
            d->m_startURL.setPath( QDir::homePath() );
    }

    d->m_startDir = d->m_startURL;
    d->m_rootUrl = d->m_treeView->rootUrl();

    d->readConfig( KSharedConfig::openConfig(), "DirSelect Dialog" );

    mainLayout->addWidget( d->m_treeView, 1 );
    mainLayout->addWidget( d->m_urlCombo, 0 );

    connect( d->m_treeView, SIGNAL(currentChanged(QUrl)),
             SLOT(slotCurrentChanged()));
    connect( d->m_treeView, SIGNAL(activated(QModelIndex)),
             SLOT(slotExpand(QModelIndex)));
    connect( d->m_treeView, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenuRequested(QPoint)));

    connect( d->m_urlCombo, SIGNAL(editTextChanged(QString)),
             SLOT(slotComboTextChanged(QString)));
    connect( d->m_urlCombo, SIGNAL(activated(QString)),
             SLOT(slotUrlActivated(QString)));
    connect( d->m_urlCombo, SIGNAL(returnPressed(QString)),
             SLOT(slotUrlActivated(QString)));

    setCurrentUrl(d->m_startURL);
}


KDirSelectDialog::~KDirSelectDialog()
{
    delete d;
}

QUrl KDirSelectDialog::url() const
{
    QUrl comboUrl = QUrl::fromUserInput(d->m_urlCombo->currentText());

    if ( comboUrl.isValid() ) {
       KIO::StatJob *statJob = KIO::stat(comboUrl, KIO::HideProgressInfo);
       const bool ok = KIO::NetAccess::synchronousRun(statJob, d->m_parent);
       if (ok && statJob->statResult().isDir()) {
           return comboUrl;
       }
    }

    kDebug() << comboUrl.path() << " is not an accessible directory";
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

QUrl KDirSelectDialog::startDir() const
{
    return d->m_startDir;
}

void KDirSelectDialog::setCurrentUrl( const QUrl& url )
{
    if ( !url.isValid() )
        return;

    if (url.scheme() != d->m_rootUrl.scheme()) {
        QUrl u( url );
        u.setPath("/");//NOTE portability?
        d->m_treeView->setRootUrl( u );
        d->m_rootUrl = u;
    }

    //Check if url represents a hidden folder and enable showing them
    QString fileName = QUrlPathInfo(url).fileName();
    //TODO a better hidden file check?
    bool isHidden = fileName.length() > 1 && fileName[0] == '.' &&
                                                (fileName.length() > 2 ? fileName[1] != '.' : true);
    bool showHiddenFiles = isHidden && !d->m_treeView->showHiddenFiles();
    if (showHiddenFiles) {
        d->showHiddenFoldersAction->setChecked(true);
        d->m_treeView->setShowHiddenFiles(true);
    }

    d->m_treeView->setCurrentUrl( url );
}

void KDirSelectDialog::accept()
{
    QUrl selectedUrl = url();
    if (!selectedUrl.isValid()) {
        return;
    }

    if (!d->m_recentDirClass.isEmpty()) {
        KRecentDirs::add(d->m_recentDirClass, selectedUrl.toString());
    }

    d->m_urlCombo->addToHistory(selectedUrl.toDisplayString());
    KFileDialog::setStartDir( url() );

    QDialog::accept();
}

void KDirSelectDialog::hideEvent( QHideEvent *event )
{
    d->saveConfig( KSharedConfig::openConfig(), "DirSelect Dialog" );

    QDialog::hideEvent(event);
}

// static
QUrl KDirSelectDialog::selectDirectory( const QUrl& startDir,
                                        bool localOnly,
                                        QWidget *parent,
                                        const QString& caption)
{
    KDirSelectDialog myDialog( startDir, localOnly, parent);

    if ( !caption.isNull() )
        myDialog.setWindowTitle( caption );

    if ( myDialog.exec() == QDialog::Accepted )
        return KIO::NetAccess::mostLocalUrl(myDialog.url(),parent);
    else
        return QUrl();
}

#include "moc_kdirselectdialog.cpp"
