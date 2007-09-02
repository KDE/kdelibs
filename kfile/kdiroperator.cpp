/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Stephan Kulow <coolo@kde.org>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kdiroperator.h"
#include "kcombiview.h"
#include "kdirmodel.h"
#include "kdirsortfilterproxymodel.h"
#include "kfilepreview.h"
#include "k3fileiconview.h"
#include "k3filedetailview.h"
#include "kfileview.h"
#include "kfileitem.h"
#include "kfilemetapreview.h"

#include <config-kfile.h>

#include <unistd.h>

#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QTimer>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QListView>
#include <QtGui/QTreeView>
#include <QtGui/QPushButton>
#include <QtGui/QProgressBar>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdirlister.h>
#include <kfileitemdelegate.h>
#include <kicon.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandardaction.h>
#include <kio/job.h>
#include <kio/deletejob.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/previewjob.h>
#include <kio/renamedialog.h>
#include <kpropertiesdialog.h>
#include <kmimetypefactory.h>
#include <kstandardshortcut.h>
#include <kde_file.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kconfiggroup.h>


template class QHash<QString,KFileItem*>;

/**
 * Default icon view for KDirOperator using
 * custom view options.
 */
class DirOperatorIconView : public QListView
{
public:
    DirOperatorIconView(QWidget *parent = 0);
    virtual ~DirOperatorIconView();

protected:
    virtual QStyleOptionViewItem viewOptions() const;

private:
    QStyleOptionViewItem m_viewOptions;
};

DirOperatorIconView::DirOperatorIconView(QWidget *parent) :
    QListView(parent)
{
    setViewMode(QListView::IconMode);
    setFlow(QListView::TopToBottom);
    setResizeMode(QListView::Adjust);
    setSpacing(KDialog::spacingHint());
    setMovement(QListView::Static);

    m_viewOptions = QListView::viewOptions();
    m_viewOptions.showDecorationSelected = true;
    m_viewOptions.decorationPosition = QStyleOptionViewItem::Left;
    m_viewOptions.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    m_viewOptions.decorationSize = QSize(K3Icon::SizeSmall, K3Icon::SizeSmall);

    QFontMetrics metrics(m_viewOptions.font);
    const int fontHeight = metrics.height();
    setGridSize(QSize(fontHeight * 10, fontHeight + 4));
}

DirOperatorIconView::~DirOperatorIconView()
{
}

QStyleOptionViewItem DirOperatorIconView::viewOptions() const
{
    return m_viewOptions;
}

/**
 * Default detail view for KDirOperator using
 * custom resizing options and columns.
 */
class DirOperatorDetailView : public QTreeView
{
public:
    DirOperatorDetailView(QWidget *parent = 0);
    virtual ~DirOperatorDetailView();

protected:
    virtual bool event(QEvent *event);
};

DirOperatorDetailView::DirOperatorDetailView(QWidget *parent) :
    QTreeView(parent)
{
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setUniformRowHeights(true);
}

DirOperatorDetailView::~DirOperatorDetailView()
{
}

bool DirOperatorDetailView::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        // Assure that by respecting the available width that:
        // - the 'Name' column is stretched as large as possible
        // - the remaining columns are as small as possible
        QHeaderView *headerView = header();
        headerView->setStretchLastSection(false);
        headerView->setResizeMode(QHeaderView::ResizeToContents);
        headerView->setResizeMode(0, QHeaderView::Stretch);

        // hide columns
        // TODO: provide a context menu for showing/hiding columns
        // and remember the setting
        hideColumn(KDirModel::Permissions);
        hideColumn(KDirModel::Owner);
        hideColumn(KDirModel::Group);
    }

    return QTreeView::event(event);
}

class KDirOperator::KDirOperatorPrivate
{
public:
    KDirOperatorPrivate();
    ~KDirOperatorPrivate();

    QStack<KUrl*> backStack;    ///< Contains all URLs you can reach with the back button.
    QStack<KUrl*> forwardStack; ///< Contains all URLs you can reach with the forward button.

    KDirLister *dirLister;
    KUrl currUrl;

    KCompletion completion;
    KCompletion dirCompletion;
    bool completeListDirty;
    QDir::SortFlags sorting;

    QAbstractItemView *itemView;
    KDirModel *dirModel;
    KDirSortFilterProxyModel *proxyModel;

    KFileItemList pendingMimeTypes;

    // the enum KFile::FileView as an int
    int viewKind;
    int defaultView;

    KFile::Modes mode;
    QProgressBar *progressBar;

    const QWidget *preview;    // temporary pointer for the preview widget

    bool leftButtonPressed;
    bool dirHighlighting;
    bool onlyDoubleClickSelectsFiles;
    QString lastURL; // used for highlighting a directory on cdUp
    QTimer *progressDelayTimer;
    QAction *viewActionSeparator;
    int dropOptions;

    KConfigGroup *configGroup;
};

KDirOperator::KDirOperatorPrivate::KDirOperatorPrivate() :
    dirLister(0),
    itemView(0),
    dirModel(0),
    proxyModel(0),
    progressBar(0),
    preview(0),
    leftButtonPressed(false),
    dirHighlighting(false),
    onlyDoubleClickSelectsFiles(false),
    progressDelayTimer(0),
    dropOptions(0),
    configGroup(0),
    viewActionSeparator(new QAction(0))
{
    viewActionSeparator->setSeparator(true);
}

KDirOperator::KDirOperatorPrivate::~KDirOperatorPrivate()
{
    delete itemView;

    qDeleteAll(backStack);
    qDeleteAll(forwardStack);
    delete preview;

    delete dirLister;
    delete configGroup;

    delete progressDelayTimer;
    delete viewActionSeparator;
}

KDirOperator::KDirOperator(const KUrl& _url, QWidget *parent)
    : QWidget(parent),
      d(new KDirOperatorPrivate)
{
    d->preview = 0L;
    d->mode = KFile::File;
    d->viewKind = KFile::Simple;
    d->sorting = QDir::Name | QDir::DirsFirst;

    if (_url.isEmpty()) { // no dir specified -> current dir
        QString strPath = QDir::currentPath();
        strPath.append(QChar('/'));
        d->currUrl = KUrl();
        d->currUrl.setProtocol(QLatin1String("file"));
        d->currUrl.setPath(strPath);
    }
    else {
        d->currUrl = _url;
        if ( d->currUrl.protocol().isEmpty() )
            d->currUrl.setProtocol(QLatin1String("file"));

        d->currUrl.addPath("/"); // make sure we have a trailing slash!
    }

    setDirLister( new KDirLister() );

    connect(&d->completion, SIGNAL(match(const QString&)),
            SLOT(slotCompletionMatch(const QString&)));

    d->progressBar = new QProgressBar(this);
    d->progressBar->setObjectName("d->progressBar");
    d->progressBar->adjustSize();
    d->progressBar->move(2, height() - d->progressBar->height() -2);

    d->progressDelayTimer = new QTimer( this );
    d->progressDelayTimer->setObjectName( QLatin1String( "d->progressBar delay timer" ) );
    connect( d->progressDelayTimer, SIGNAL( timeout() ),
        SLOT( slotShowProgress() ));

    d->completeListDirty = false;

    // action stuff
    setupActions();
    setupMenu();

    setFocusPolicy(Qt::WheelFocus);
}

KDirOperator::~KDirOperator()
{
    resetCursor();
    /*if ( d->fileView )
    {
        if ( d->configGroup )
            d->fileView->writeConfig( d->configGroup );

        delete d->fileView;
        d->fileView = 0L;
    }*/

    delete d;
}


void KDirOperator::setSorting( QDir::SortFlags spec )
{
    //if ( d->fileView )
    //    d->fileView->setSorting( spec );
    d->sorting = spec;
    updateSortActions();
}

QDir::SortFlags KDirOperator::sorting() const
{
    return d->sorting;
}

bool KDirOperator::isRoot() const
{
    return url().path() == QString(QLatin1Char('/'));
}

KDirLister *KDirOperator::dirLister() const
{
    return d->dirLister;
}

void KDirOperator::resetCursor()
{
   QApplication::restoreOverrideCursor();
   d->progressBar->hide();
}

void KDirOperator::insertViewDependentActions()
{
    return; // TODO

   // If we have a new view actionCollection(), insert its actions
   // into m_viewActionMenu.
    /*
    if( !d->fileView )
       return;

   if ( (m_viewActionMenu->menu()->actions().count() == 0) || 			// Not yet initialized or...
        (m_actionCollection != d->fileView->actionCollection()) )	// ...changed since.
   {
      if (m_actionCollection)
      {
         disconnect( m_actionCollection, SIGNAL( inserted( KAction * )),
               this, SLOT( slotViewActionAdded( KAction * )));
         disconnect( m_actionCollection, SIGNAL( removed( KAction * )),
               this, SLOT( slotViewActionRemoved( KAction * )));
      }

      m_viewActionMenu->menu()->clear();
//      m_viewActionMenu->addAction( m_shortAction );
//      m_viewActionMenu->addAction( m_detailedAction );
//      m_viewActionMenu->addAction( actionSeparator );
      m_viewActionMenu->addAction( m_actionCollection->action( "short view" ) );
      m_viewActionMenu->addAction( m_actionCollection->action( "detailed view" ) );
      m_viewActionMenu->addAction( actionSeparator );
      m_viewActionMenu->addAction( m_showHiddenAction );
//      m_viewActionMenu->addAction( m_actionCollection->action( "single" ));
      m_viewActionMenu->addAction( m_separateDirsAction );
      // Warning: adjust slotViewActionAdded() and slotViewActionRemoved()
      // when you add/remove actions here!

      m_actionCollection = d->fileView->actionCollection();
      if (!m_actionCollection)
         return;

      if ( !m_actionCollection->isEmpty() )
      {
         m_viewActionMenu->addAction( d->viewActionSeparator );

         // first insert the normal actions, then the grouped ones
         QList<QActionGroup*> groups = m_actionCollection->actionGroups();

         foreach (QAction* action, m_actionCollection->actionsWithoutGroup())
            m_viewActionMenu->addAction( action );

         foreach (QActionGroup* group, groups)
         {
            m_viewActionMenu->addSeparator();

            foreach (QAction* action, group->actions())
               m_viewActionMenu->addAction( action );
         }
      }

      connect( m_actionCollection, SIGNAL( inserted( KAction * )),
               SLOT( slotViewActionAdded( KAction * )));
      connect( m_actionCollection, SIGNAL( removed( KAction * )),
               SLOT( slotViewActionRemoved( KAction * )));
   }*/
}

void KDirOperator::activatedMenu( const KFileItem *, const QPoint& pos )
{
    setupMenu();
    updateSelectionDependentActions();

    m_actionMenu->menu()->exec( pos );
}

void KDirOperator::sortByName()
{
    m_byNameAction->setChecked( true );
}

void KDirOperator::sortBySize()
{
    m_bySizeAction->setChecked( true );
}

void KDirOperator::sortByDate()
{
    m_byDateAction->setChecked( true );
}

void KDirOperator::sortReversed()
{
    m_reverseAction->setChecked( !m_reverseAction->isChecked() );
}

void KDirOperator::toggleDirsFirst()
{
    m_dirsFirstAction->setChecked( !m_dirsFirstAction->isChecked() );
}

void KDirOperator::toggleIgnoreCase()
{
    // TODO: not available yet in KDirSortFilterProxyModel
}

void KDirOperator::updateSelectionDependentActions()
{
    const bool hasSelection = (d->itemView != 0) &&
                              d->itemView->selectionModel()->hasSelection();
    m_actionCollection->action("trash")->setEnabled(hasSelection);
    m_actionCollection->action("delete")->setEnabled(hasSelection);
    m_actionCollection->action("properties")->setEnabled(hasSelection);
}

void KDirOperator::setPreviewWidget(const QWidget *w)
{
    if(w != 0L)
        d->viewKind = (d->viewKind | KFile::PreviewContents);
    else
        d->viewKind = (d->viewKind & ~KFile::PreviewContents);

    delete d->preview;
    d->preview = w;

    KToggleAction *preview = static_cast<KToggleAction*>(m_actionCollection->action("preview"));
    preview->setEnabled( w != 0L );
    preview->setChecked( w != 0L );
    setView( static_cast<KFile::FileView>(d->viewKind) );
}

QList<KFileItem> KDirOperator::selectedItems() const
{
    QList<KFileItem> itemList;
    if (d->itemView == 0) {
        return itemList;
    }

    const QItemSelection selection = d->proxyModel->mapSelectionToSource(d->itemView->selectionModel()->selection());

    const QModelIndexList indexList = selection.indexes();
    foreach (QModelIndex index, indexList) {
        KFileItem item = d->dirModel->itemForIndex(index);
        if (!item.isNull()) {
            itemList.append(item);
        }
    }

    return itemList;
}

bool KDirOperator::isSelected(const KFileItem *item) const
{
    if ((item == 0) || (d->itemView == 0)) {
        return false;
    }

    const QModelIndex dirIndex = d->dirModel->indexForItem(*item);
    const QModelIndex proxyIndex = d->proxyModel->mapFromSource(dirIndex);
    return d->itemView->selectionModel()->isSelected(proxyIndex);
}

int KDirOperator::numDirs() const
{
    return 0; // TODO: d->fileView ? d->fileView->numDirs() : 0;
}

int KDirOperator::numFiles() const
{
    return 0; // TODO: d->fileView ? d->fileView->numFiles() : 0;
}

KCompletion * KDirOperator::completionObject() const
{
    return const_cast<KCompletion *>( &d->completion );
}

KCompletion *KDirOperator::dirCompletionObject() const
{
    return const_cast<KCompletion *>( &d->dirCompletion );
}

KActionCollection * KDirOperator::actionCollection() const
{
    return m_actionCollection;
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (d->viewKind & ~KFile::Simple) | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (d->viewKind & ~KFile::Detail) | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    d->dirLister->setShowingDotFiles( show );
    updateDir();
    //if ( d->fileView )
    //    d->fileView->listingCompleted();
}

void KDirOperator::slotSeparateDirs()
{
    if (m_separateDirsAction->isChecked())
    {
        KFile::FileView view = static_cast<KFile::FileView>( d->viewKind | KFile::SeparateDirs );
        setView( view );
    }
    else
    {
        KFile::FileView view = static_cast<KFile::FileView>( d->viewKind & ~KFile::SeparateDirs );
        setView( view );
    }
}

void KDirOperator::slotDefaultPreview()
{
    d->viewKind = d->viewKind | KFile::PreviewContents;
    if ( !d->preview ) {
        d->preview = new KFileMetaPreview( this );
        (static_cast<KToggleAction*>( m_actionCollection->action("preview") ))->setChecked(true);
    }

    setView( static_cast<KFile::FileView>(d->viewKind) );
}

void KDirOperator::slotSortByName()
{
    // TODO:
    //QDir::SortFlags sorting = (d->fileView->sorting()) & ~QDir::SortByMask;
    //d->fileView->setSorting( sorting | QDir::Name );
    //d->sorting = d->fileView->sorting();
}

void KDirOperator::slotSortBySize()
{
    // TODO:
    //QDir::SortFlags sorting = (d->fileView->sorting()) & ~QDir::SortByMask;
    //d->fileView->setSorting( sorting | QDir::Size );
    //d->sorting = d->fileView->sorting();
}

void KDirOperator::slotSortByDate()
{
    // TODO:
    //QDir::SoFlags sorting = (d->fileView->sorting()) & ~QDir::SortByMask;
    //d->fileView->setSorting( sorting | QDir::Time );
    //d->sorting = d->fileView->sorting();
}

void KDirOperator::slotSortReversed()
{
    // TODO:
    //if ( d->fileView )
    //    d->fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    // TODO:
    /*if ( !d->fileView )
      return;

    QDir::SortFlags sorting = d->fileView->sorting();
    if ( !KFile::isSortDirsFirst( sorting ) )
        d->fileView->setSorting( sorting | QDir::DirsFirst );
    else
        d->fileView->setSorting( sorting & ~QDir::DirsFirst );
    d->sorting = d->fileView->sorting();*/
}

void KDirOperator::slotToggleIgnoreCase()
{
    // TODO:
    /*if ( !d->fileView )
      return;

    QDir::SortFlags sorting = d->fileView->sorting();
    if ( !KFile::isSortCaseInsensitive( sorting ) )
        d->fileView->setSorting( sorting | QDir::IgnoreCase );
    else
        d->fileView->setSorting( sorting & ~QDir::IgnoreCase );
    d->sorting = d->fileView->sorting();*/
}

void KDirOperator::mkdir()
{
    bool ok;
    QString where = url().pathOrUrl();
    QString name = i18n( "New Folder" );
#ifdef Q_WS_WIN
    if ( url().isLocalFile() && QFileInfo( url().toLocalFile() + name ).exists() )
#else
    if ( url().isLocalFile() && QFileInfo( url().path(KUrl::AddTrailingSlash) + name ).exists() )
#endif
         name = KIO::RenameDialog::suggestName( url(), name );

    QString folder = KInputDialog::getText( i18n( "New Folder" ),
                                         i18n( "Create new folder in:\n%1" ,  where ),
                                         name, &ok, this);
    if (ok)
      mkdir( KIO::encodeFileName( folder ), true );
}

bool KDirOperator::mkdir( const QString& directory, bool enterDirectory )
{
    // Creates "directory", relative to the current directory (d->currUrl).
    // The given path may contain any number directories, existant or not.
    // They will all be created, if possible.

    bool writeOk = false;
    bool exists = false;
    KUrl url( d->currUrl );

    QStringList dirs = directory.split( QDir::separator(), QString::SkipEmptyParts );
    QStringList::ConstIterator it = dirs.begin();

    for ( ; it != dirs.end(); ++it )
    {
        url.addPath( *it );
        exists = KIO::NetAccess::exists( url, KIO::NetAccess::DestinationSide, 0 );
        writeOk = !exists && KIO::NetAccess::mkdir( url, topLevelWidget() );
    }

    if ( exists ) // url was already existant
    {
        KMessageBox::sorry(viewWidget(), i18n("A file or folder named %1 already exists.", url.pathOrUrl()));
        enterDirectory = false;
    }
    else if ( !writeOk ) {
        KMessageBox::sorry(viewWidget(), i18n("You do not have permission to "
                                              "create that folder." ));
    }
    else if ( enterDirectory ) {
        setUrl( url, true );
    }

    return writeOk;
}

KIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    bool ask, bool showProgress )
{
    return del( items, this, ask, showProgress );
}

KIO::DeleteJob * KDirOperator::del( const KFileItemList& items,
                                    QWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to delete."),
                                i18n("Nothing to Delete") );
        return 0L;
    }

    const KUrl::List urls = items.urlList();
    QStringList files;
    foreach( const KUrl& url, urls )
        files.append( url.pathOrUrl() );

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to delete\n <b>'%1'</b>?</qt>" ,
                  files.first() ),
                                                      i18n("Delete File"),
                                                      KStandardGuiItem::del(),
                                                      KStandardGuiItem::cancel(), "AskForDelete" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18np("Do you really want to delete this item?", "Do you really want to delete these %1 items?", items.count() ),
                                                    files,
                                                    i18n("Delete Files"),
                                                    KStandardGuiItem::del(),
                                                    KStandardGuiItem::cancel(), "AskForDelete" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        KIO::DeleteJob *job = KIO::del( urls, false, showProgress );
        job->ui()->setWindow (topLevelWidget());
        job->ui()->setAutoErrorHandlingEnabled( true );
        return job;
    }

    return 0L;
}

void KDirOperator::deleteSelected()
{
    // TODO:
    /*if ( !d->fileView )
        return;

    const KFileItemList *list = d->fileView->selectedItems();
    if ( list )
        del( *list );*/
}

KIO::CopyJob * KDirOperator::trash( const KFileItemList& items,
                                    QWidget *parent,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( parent,
                                i18n("You did not select a file to trash."),
                                i18n("Nothing to Trash") );
        return 0L;
    }

    const KUrl::List urls = items.urlList();
    QStringList files;
    foreach( const KUrl& url, urls )
        files.append( url.pathOrUrl() );

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to trash\n <b>'%1'</b>?</qt>" ,
                  files.first() ),
                                                      i18n("Trash File"),
                                                      KGuiItem(i18nc("to trash", "&Trash"),"edit-trash"),
                                                      KStandardGuiItem::cancel(), "AskForTrash" );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18np("translators: not called for n == 1", "Do you really want to trash these %1 items?", items.count() ),
                                                    files,
                                                    i18n("Trash Files"),
                                                    KGuiItem(i18nc("to trash", "&Trash"),"edit-trash"),
                                                    KStandardGuiItem::cancel(), "AskForTrash" );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt ) {
        KIO::CopyJob *job = KIO::trash( urls, showProgress );
        job->ui()->setWindow (topLevelWidget());
        job->ui()->setAutoErrorHandlingEnabled( true );
        return job;
    }

    return 0L;
}

void KDirOperator::trashSelected()
{
    // TODO:
    /*if ( !d->fileView )
        return;

    if ( reason == KAction::PopupMenuActivation && ( modifiers & Qt::ShiftModifier ) ) {
        deleteSelected();
        return;
    }

    const KFileItemList *list = d->fileView->selectedItems();
    if ( list )
        trash( *list, this );*/
}

void KDirOperator::close()
{
    resetCursor();
    d->pendingMimeTypes.clear();
    d->completion.clear();
    d->dirCompletion.clear();
    d->completeListDirty = true;
    d->dirLister->stop();
}

void KDirOperator::checkPath(const QString &, bool /*takeFiles*/) // SLOT
{
#if 0
    // copy the argument in a temporary string
    QString text = _txt;
    // it's unlikely to happen, that at the beginning are spaces, but
    // for the end, it happens quite often, I guess.
    text = text.trimmed();
    // if the argument is no URL (the check is quite fragil) and it's
    // no absolute path, we add the current directory to get a correct url
    if (text.find(':') < 0 && text[0] != '/')
        text.insert(0, d->currUrl);

    // in case we have a selection defined and someone patched the file-
    // name, we check, if the end of the new name is changed.
    if (!selection.isNull()) {
        int position = text.lastIndexOf('/');
        ASSERT(position >= 0); // we already inserted the current d->dirLister in case
        QString filename = text.mid(position + 1, text.length());
        if (filename != selection)
            selection = QString();
    }

    KUrl u(text); // I have to take care of entered URLs
    bool filenameEntered = false;

    if (u.isLocalFile()) {
        // the empty path is kind of a hack
        KFileItem i("", u.toLocalFile());
        if (i.isDir())
            setUrl(text, true);
        else {
            if (takeFiles)
                if (acceptOnlyExisting && !i.isFile())
                    warning("you entered an invalid URL");
                else
                    filenameEntered = true;
        }
    } else
        setUrl(text, true);

    if (filenameEntered) {
        filename_ = u.url();
        emit fileSelected(filename_);

        QApplication::restoreOverrideCursor();

        accept();
    }
#endif
    kDebug(kfile_area) << "TODO KDirOperator::checkPath()";
}

void KDirOperator::setUrl(const KUrl& _newurl, bool clearforward)
{
    KUrl newurl;

    if ( !_newurl.isValid() )
        newurl.setPath( QDir::homePath() );
    else
        newurl = _newurl;

#ifdef Q_WS_WIN
    QString pathstr = newurl.toLocalFile();
#else
    QString pathstr = newurl.path(KUrl::AddTrailingSlash);
#endif
    newurl.setPath(pathstr);

    // already set
    if ( newurl.equals( d->currUrl, KUrl::CompareWithoutTrailingSlash ) )
        return;

    if ( !isReadable( newurl ) ) {
        // maybe newurl is a file? check its parent directory
        newurl.cd(QLatin1String(".."));
        if ( !isReadable( newurl ) ) {
            resetCursor();
            KMessageBox::error(viewWidget(),
                               i18n("The specified folder does not exist "
                                    "or was not readable."));
            return;
        }
    }

    if (clearforward) {
        // autodelete should remove this one
        d->backStack.push(new KUrl(d->currUrl));
        qDeleteAll( d->forwardStack );
        d->forwardStack.clear();
    }

    d->lastURL = d->currUrl.url(KUrl::RemoveTrailingSlash);
    d->currUrl = newurl;

    pathChanged();
    emit urlEntered(newurl);

    // enable/disable actions
    QAction* forwardAction = m_actionCollection->action("forward");
    forwardAction->setEnabled( !d->forwardStack.isEmpty() );

    QAction* backAction = m_actionCollection->action("back");
    backAction->setEnabled(!d->backStack.isEmpty());

    QAction* upAction = m_actionCollection->action("up");
    upAction->setEnabled(!isRoot());

    openUrl(newurl);
}

void KDirOperator::updateDir()
{
    // TODO:
    /*d->dirLister->emitChanges();
    if ( d->fileView )
        d->fileView->listingCompleted();*/
}

void KDirOperator::rereadDir()
{
    pathChanged();
    openUrl( d->currUrl, false, true );
}


bool KDirOperator::openUrl( const KUrl& url, bool keep, bool reload )
{
    bool result = d->dirLister->openUrl( url, keep, reload );
    if ( !result ) // in that case, neither completed() nor canceled() will be emitted by KDL
        slotCanceled();

    return result;
}

// Protected
void KDirOperator::pathChanged()
{
    if (d->itemView == 0)
        return;

    d->pendingMimeTypes.clear();
    //d->fileView->clear(); TODO
    d->completion.clear();
    d->dirCompletion.clear();

    // it may be, that we weren't ready at this time
    QApplication::restoreOverrideCursor();

    // when KIO::Job emits finished, the slot will restore the cursor
    QApplication::setOverrideCursor( Qt::WaitCursor );

    if ( !isReadable( d->currUrl )) {
        KMessageBox::error(viewWidget(),
                           i18n("The specified folder does not exist "
                                "or was not readable."));
        if (d->backStack.isEmpty())
            home();
        else
            back();
    }
}

void KDirOperator::slotRedirected( const KUrl& newURL )
{
    d->currUrl = newURL;
    d->pendingMimeTypes.clear();
    d->completion.clear();
    d->dirCompletion.clear();
    d->completeListDirty = true;
    emit urlEntered( newURL );
}

// Code pinched from kfm then hacked
void KDirOperator::back()
{
    if ( d->backStack.isEmpty() )
        return;

    d->forwardStack.push( new KUrl(d->currUrl) );

    KUrl *s = d->backStack.pop();

    setUrl(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( d->forwardStack.isEmpty() )
        return;

    d->backStack.push(new KUrl(d->currUrl));

    KUrl *s = d->forwardStack.pop();
    setUrl(*s, false);
    delete s;
}

KUrl KDirOperator::url() const
{
    return d->currUrl;
}

void KDirOperator::cdUp()
{
    KUrl tmp(d->currUrl);
    tmp.cd(QLatin1String(".."));
    setUrl(tmp, true);
}

void KDirOperator::home()
{
    KUrl u;
    u.setPath( QDir::homePath() );
    setUrl(u, true);
}

void KDirOperator::clearFilter()
{
    d->dirLister->setNameFilter( QString() );
    d->dirLister->clearMimeFilter();
    checkPreviewSupport();
}

void KDirOperator::setNameFilter(const QString& filter)
{
    d->dirLister->setNameFilter(filter);
    checkPreviewSupport();
}

QString KDirOperator::nameFilter() const
{
    return d->dirLister->nameFilter();
}

void KDirOperator::setMimeFilter( const QStringList& mimetypes )
{
    d->dirLister->setMimeFilter( mimetypes );
    checkPreviewSupport();
}

QStringList KDirOperator::mimeFilter() const
{
    return d->dirLister->mimeFilters();
}

bool KDirOperator::checkPreviewSupport()
{
    KToggleAction *previewAction = static_cast<KToggleAction*>( m_actionCollection->action( "preview" ));

    bool hasPreviewSupport = false;
    KConfigGroup cg( KGlobal::config(), ConfigGroup );
    if ( cg.readEntry( "Show Default Preview", true ) )
        hasPreviewSupport = checkPreviewInternal();

    previewAction->setEnabled( hasPreviewSupport );
    return hasPreviewSupport;
}

bool KDirOperator::checkPreviewInternal() const
{
    QStringList supported = KIO::PreviewJob::supportedMimeTypes();
    // no preview support for directories?
    if ( dirOnlyMode() && supported.indexOf( "inode/directory" ) == -1 )
        return false;

    QStringList mimeTypes = d->dirLister->mimeFilters();
    QStringList nameFilter = d->dirLister->nameFilter().split( " ", QString::SkipEmptyParts );

    if ( mimeTypes.isEmpty() && nameFilter.isEmpty() && !supported.isEmpty() )
        return true;
    else {
        QRegExp r;
        r.setPatternSyntax( QRegExp::Wildcard ); // the "mimetype" can be "image/*"

        if ( !mimeTypes.isEmpty() ) {
            QStringList::Iterator it = supported.begin();

            for ( ; it != supported.end(); ++it ) {
                r.setPattern( *it );

                QStringList result = mimeTypes.filter( r );
                if ( !result.isEmpty() ) { // matches! -> we want previews
                    return true;
                }
            }
        }

        if ( !nameFilter.isEmpty() ) {
            // find the mimetypes of all the filter-patterns and
            KMimeTypeFactory *fac = KMimeTypeFactory::self();
            QStringList::Iterator it1 = nameFilter.begin();
            for ( ; it1 != nameFilter.end(); ++it1 ) {
                if ( (*it1) == "*" ) {
                    return true;
                }

                KMimeType::Ptr mt = fac->findFromPattern( *it1 );
                if ( !mt )
                    continue;
                QString mime = mt->name();

                // the "mimetypes" we get from the PreviewJob can be "image/*"
                // so we need to check in wildcard mode
                QStringList::Iterator it2 = supported.begin();
                for ( ; it2 != supported.end(); ++it2 ) {
                    r.setPattern( *it2 );
                    if ( r.indexIn( mime ) != -1 ) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

QAbstractItemView* KDirOperator::createView( QWidget* parent, KFile::FileView view )
{
    QAbstractItemView* newView = 0;
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview = ( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    /*if ( separateDirs || preview ) {
        KCombiView *combi = 0L;
        if (separateDirs)
        {
            combi = new KCombiView( parent );
            combi->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
        }

        QAbstractItemView* view = 0;
        if ( KFile::isSimpleView( view ) )
            view = createView( combi, KFile::Simple );
        else
            view = createView( combi, KFile::Detail );

        //view->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);

        //if (combi)
        //    combi->setRight( v );

        if (preview)
        {
            KFilePreview* pView = new KFilePreview( combi ? combi : view, parent );
            pView->setObjectName( QLatin1String( "preview" ) );
            pView->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
            newView = pView;
        }
        else
            newView = combi;
    }*/
    /*else if ( KFile::isDetailView( view ) && !preview ) {
        K3FileDetailView *ndw = new K3FileDetailView( parent );
        ndw->setObjectName( "detail view" );
        newView = ndw;
        newView->setViewName( i18n("Detailed View") );
    }
    else  if ( KFile::isSimpleView( view ) && !preview ) {
        //K3FileIconView* iconView = new K3FileIconView(parent, "simple view");

        newView = new QListView(parent);
        //newView->setViewName( i18n("Short View") );
    }*/

    if (KFile::isDetailView(view)) {
        newView = new DirOperatorDetailView(parent);
    } else {
        newView = new DirOperatorIconView(parent);
    }

    //newView->widget()->setAcceptDrops(acceptDrops());
    return newView;
}

void KDirOperator::setAcceptDrops(bool b)
{
    // TODO:
    //if (d->fileView)
    //   d->fileView->widget()->setAcceptDrops(b);
    QWidget::setAcceptDrops(b);
}

void KDirOperator::setDropOptions(int options)
{
    d->dropOptions = options;
    // TODO:
    //if (d->fileView)
    //   d->fileView->setDropOptions(options);
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview=( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if (view == KFile::Default) {
        if ( KFile::isDetailView( (KFile::FileView) d->defaultView ) )
            view = KFile::Detail;
        else
            view = KFile::Simple;

        separateDirs = KFile::isSeparateDirs( static_cast<KFile::FileView>(d->defaultView) );
        preview = ( KFile::isPreviewInfo( static_cast<KFile::FileView>(d->defaultView) ) ||
                    KFile::isPreviewContents( static_cast<KFile::FileView>(d->defaultView) ) )
                  && m_actionCollection->action("preview")->isEnabled();

        if ( preview ) { // instantiates KFileMetaPreview and calls setView()
            d->viewKind = d->defaultView;
            slotDefaultPreview();
            return;
        }
        else if ( !separateDirs )
            m_separateDirsAction->setChecked(true);
    }

    // if we don't have any files, we can't separate dirs from files :)
    if ( (mode() & KFile::File) == 0 &&
         (mode() & KFile::Files) == 0 ) {
        separateDirs = false;
        m_separateDirsAction->setEnabled( false );
    }

    d->viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);
    view = static_cast<KFile::FileView>(d->viewKind);

    QAbstractItemView *newView = createView( this, view );
    /*if ( preview ) {
        // we keep the preview-_widget_ around, but not the KFilePreview.
        // KFilePreview::setPreviewWidget handles the reparenting for us
        static_cast<KFilePreview*>(new_view)->setPreviewWidget(d->preview, url());
    }*/

    setView( newView );
}

QAbstractItemView * KDirOperator::view() const
{
    return d->itemView;
}

QWidget * KDirOperator::viewWidget() const
{
    return 0; // TODO: d->fileView ? d->fileView->widget() : 0L;
}


void KDirOperator::connectView(QAbstractItemView *view)
{
    // TODO: do a real timer and restart it after that
    d->pendingMimeTypes.clear();
    bool listDir = true;

    /*if ( dirOnlyMode() )
         view->setViewMode(KFileView::Directories);
    else
        view->setViewMode(KFileView::All);*/

    /*if ( d->mode & KFile::Files )
        view->setSelectionMode( KFile::Extended );
    else
        view->setSelectionMode( KFile::Single );*/

    /*    if (d->fileView)
    {
        if ( d->configGroup ) // save and restore the views' configuration
        {
            d->fileView->writeConfig(d->configGroup);
            view->readConfig(d->configGroup);
        }

        // transfer the state from old view to new view
        view->clear();
        view->addItemList( *d->fileView->items() );
        listDir = false;

        if ( d->fileView->widget()->hasFocus() )
            view->widget()->setFocus();

        KFileItem *oldCurrentItem = d->fileView->currentFileItem();
        if ( oldCurrentItem ) {
            view->setCurrentItem( oldCurrentItem );
            view->setSelected( oldCurrentItem, false );
            view->ensureItemVisible( oldCurrentItem );
        }

        const KFileItemList *oldSelected = d->fileView->selectedItems();
        if ( !oldSelected->isEmpty() ) {
            KFileItemList::const_iterator kit = oldSelected->begin();
            const KFileItemList::const_iterator kend = oldSelected->end();
            for ( ; kit != kend; ++kit )
                view->setSelected( *kit, true );
        }

        d->fileView->widget()->hide();
        delete d->fileView;
    }

    else
    {
        if ( d->configGroup )
            view->readConfig( d->configGroup );
    }*/

    delete d->itemView;
    d->itemView = view;
    d->itemView->setModel(d->proxyModel);

    KFileItemDelegate *delegate = new KFileItemDelegate(d->itemView);
    d->itemView->setItemDelegate(delegate);
    d->itemView->viewport()->setAttribute(Qt::WA_Hover);
    d->itemView->setContextMenuPolicy(Qt::CustomContextMenu);

    //d->fileView = view;
    //d->fileView->setDropOptions(d->dropOptions);

    connect(d->itemView, SIGNAL(pressed(const QModelIndex&)),
            this, SLOT(slotPressed(const QModelIndex&)));
    connect(d->itemView, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(slotClicked(const QModelIndex&)));
    connect(d->itemView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(slotDoubleClicked(const QModelIndex&)));
    connect(d->itemView, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(openContextMenu(const QPoint&)));


    /*KFileViewSignaler *sig = view->signaler();

    connect(sig, SIGNAL( activatedMenu(const KFileItem *, const QPoint& ) ),
            this, SLOT( activatedMenu(const KFileItem *, const QPoint& )));
    connect(sig, SIGNAL( dirActivated(const KFileItem *) ),
            this, SLOT( selectDir(const KFileItem*) ) );
    connect(sig, SIGNAL( fileSelected(const KFileItem *) ),
            this, SLOT( selectFile(const KFileItem*) ) );
    connect(sig, SIGNAL( fileHighlighted(const KFileItem *) ),
            this, SLOT( highlightFile(const KFileItem*) ));
    connect(sig, SIGNAL( sortingChanged( QDir::SortFlags ) ),
            this, SLOT( slotViewSortingChanged( QDir::SortFlags )));
    connect(sig, SIGNAL( dropped(const KFileItem *, QDropEvent*, const KUrl::List&) ),
            this, SIGNAL( dropped(const KFileItem *, QDropEvent*, const KUrl::List&)) );*/

    //if ( m_reverseAction->isChecked() != d->fileView->isReversed() )
    //    slotSortReversed();

    updateViewActions();
    d->itemView->resize(size());
    d->itemView->show();
    //d->fileView->widget()->resize(size());
    //d->fileView->widget()->show();

    if ( listDir ) {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        openUrl( d->currUrl );
    }
    //else
    //    view->listingCompleted();
}

KFile::Modes KDirOperator::mode() const
{
    return d->mode;
}

void KDirOperator::setMode(KFile::Modes mode)
{
    if (d->mode == mode)
        return;

    d->mode = mode;

    d->dirLister->setDirOnlyMode( dirOnlyMode() );

    // reset the view with the different mode
    setView( static_cast<KFile::FileView>(d->viewKind) );
}

void KDirOperator::setView(QAbstractItemView *view)
{
    if ( view == d->itemView ) {
        return;
    }

    //setFocusProxy(view->widget());
    //view->setSorting( d->sorting );
    //view->setOnlyDoubleClickSelectsFiles( d->onlyDoubleClickSelectsFiles );
    connectView(view); // also deletes the old view

    emit viewChanged( view );
}

void KDirOperator::setDirLister( KDirLister *lister )
{
    if ( lister == d->dirLister ) // sanity check
        return;

    delete d->dirModel;
    d->dirModel = 0;

    delete d->proxyModel;
    d->proxyModel = 0;

    delete d->dirLister;
    d->dirLister = lister;

    d->dirModel = new KDirModel();
    d->dirModel->setDirLister(d->dirLister);
    d->dirModel->setDropsAllowed(KDirModel::DropOnDirectory);

    d->proxyModel = new KDirSortFilterProxyModel(this);
    d->proxyModel->setSourceModel(d->dirModel);

    d->dirLister->setAutoUpdate( true );
    d->dirLister->setDelayedMimeTypes(true);

    QWidget* mainWidget = topLevelWidget();
    d->dirLister->setMainWindow (mainWidget);
    kDebug (kfile_area) << "mainWidget=" << mainWidget;

    connect( d->dirLister, SIGNAL( percent( int )),
             SLOT( slotProgress( int ) ));
    connect( d->dirLister, SIGNAL(started( const KUrl& )), SLOT(slotStarted()));
    connect( d->dirLister, SIGNAL(newItems(const KFileItemList &)),
             SLOT(insertNewFiles(const KFileItemList &)));
    connect( d->dirLister, SIGNAL(completed()), SLOT(slotIOFinished()));
    connect( d->dirLister, SIGNAL(canceled()), SLOT(slotCanceled()));
    connect( d->dirLister, SIGNAL(deleteItem(KFileItem *)),
             SLOT(itemDeleted(KFileItem *)));
    connect( d->dirLister, SIGNAL(redirection( const KUrl& )),
             SLOT( slotRedirected( const KUrl& )));
    connect( d->dirLister, SIGNAL( clear() ), SLOT( slotClearView() ));
    connect( d->dirLister, SIGNAL( refreshItems( const KFileItemList& ) ),
             SLOT( slotRefreshItems( const KFileItemList& ) ) );
}

void KDirOperator::insertNewFiles(const KFileItemList &newone)
{
    //if ( newone.isEmpty() || !d->fileView )
    //    return;

    d->completeListDirty = true;
    // TODO:
    //d->fileView->addItemList( newone );
    //emit updateInformation(d->fileView->numDirs(), d->fileView->numFiles());

    KFileItemList::const_iterator kit = newone.begin();
    const KFileItemList::const_iterator kend = newone.end();
    for ( ; kit != kend; ++kit ) {
        const KFileItem* item = *kit;
        // highlight the dir we come from, if possible
        if ( d->dirHighlighting && item->isDir() &&
                    item->url().url(KUrl::RemoveTrailingSlash) == d->lastURL ) {
            //d->fileView->setCurrentItem( item );
            //d->fileView->ensureItemVisible( item );
        }
    }

    QTimer::singleShot(200, this, SLOT(resetCursor()));
}

void KDirOperator::selectDir(const KFileItem *item)
{
    setUrl(item->url(), true);
}

void KDirOperator::itemDeleted(KFileItem *item)
{
    d->pendingMimeTypes.removeAll( item );
    // TODO:
    /*if ( d->fileView )
    {
        d->fileView->removeItem( item );
        emit updateInformation(d->fileView->numDirs(), d->fileView->numFiles());
    }*/
}

void KDirOperator::selectFile(const KFileItem *item)
{
    QApplication::restoreOverrideCursor();

    emit fileSelected( item );
}

void KDirOperator::highlightFile(const KFileItem *item)
{
    fileHighlighted( item );
}

void KDirOperator::setCurrentItem( const QString& filename )
{
    // TODO:
    /*if ( d->fileView ) {
        const KFileItem *item = 0;

        if ( !filename.isNull() )
            item = d->dirLister->findByName( filename );

        d->fileView->clearSelection();
        if ( item ) {
            d->fileView->setCurrentItem( item );
            d->fileView->setSelected( item, true );
            d->fileView->ensureItemVisible( item );
        }
    }*/
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        //d->fileView->clearSelection(); TODO
        return QString();
    }

    prepareCompletionObjects();
    return d->completion.makeCompletion( string );
}

QString KDirOperator::makeDirCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        //d->fileView->clearSelection(); TODO
        return QString();
    }

    prepareCompletionObjects();
    return d->dirCompletion.makeCompletion( string );
}

void KDirOperator::prepareCompletionObjects()
{
    // TODO
    //if ( !d->fileView )
    //    return;

    if ( d->completeListDirty ) { // create the list of all possible completions
        /*const KFileItemList* itemList = d->fileView->items();
        KFileItemList::const_iterator kit = itemList->begin();
        const KFileItemList::const_iterator kend = itemList->end();
        for ( ; kit != kend; ++kit ) {
            KFileItem *item = *kit;
            d->completion.addItem( item->name() );
            if ( item->isDir() )
                d->dirCompletion.addItem( item->name() );
        }*/
        d->completeListDirty = false;
    }
}

void KDirOperator::slotCompletionMatch(const QString& match)
{
    setCurrentItem( match );
    emit completion( match );
}

void KDirOperator::setupActions()
{
    m_actionCollection = new KActionCollection( this );
    m_actionCollection->setObjectName( "KDirOperator::actionCollection" );
    m_actionCollection->setAssociatedWidget( topLevelWidget() );

    m_actionMenu = new KActionMenu( i18n("Menu"), this );
    m_actionCollection->addAction( "popupMenu", m_actionMenu );

    QAction* upAction = m_actionCollection->addAction( KStandardAction::Up, "up", this, SLOT( cdUp() ) );
    upAction->setText( i18n("Parent Folder") );

    m_actionCollection->addAction( KStandardAction::Back, "back", this, SLOT( back() ) );

    m_actionCollection->addAction( KStandardAction::Forward, "forward", this, SLOT( forward() ) );

    QAction* homeAction = m_actionCollection->addAction( KStandardAction::Home, "home", this, SLOT( home() ) );
    homeAction->setText(i18n("Home Folder"));

    QAction* reloadAction = m_actionCollection->addAction( KStandardAction::Redisplay, "reload", this, SLOT( rereadDir() ) );
    reloadAction->setText( i18n("Reload") );
    reloadAction->setShortcuts( KStandardShortcut::shortcut( KStandardShortcut::Reload ));

    QAction* actionSeparator = new QAction( this );
    actionSeparator->setSeparator(true);
    m_actionCollection->addAction( "separator", actionSeparator );


    QAction* mkdirAction = new KAction( i18n("New Folder..."), this );
    m_actionCollection->addAction( "mkdir", mkdirAction );
    mkdirAction->setIcon( KIcon( QLatin1String("folder-new") ) );
    connect( mkdirAction, SIGNAL( triggered( bool ) ), this, SLOT( mkdir() ) );

    KAction* trash = new KAction( i18n( "Move to Trash" ), this );
    m_actionCollection->addAction( "trash", trash );
    trash->setIcon( KIcon( "edit-trash" ) );
    trash->setShortcuts( KShortcut(Qt::Key_Delete) );
    connect( trash, SIGNAL( triggered(bool) ), SLOT( trashSelected() ) );

    KAction* action = new KAction( i18n( "Delete" ), this );
    m_actionCollection->addAction( "delete", action );
    action->setIcon( KIcon( "edit-delete" ) );
    action->setShortcuts( KShortcut(Qt::SHIFT+Qt::Key_Delete) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT( deleteSelected() ) );

    // the sort menu actions
    m_sortActionMenu = new KActionMenu( i18n("Sorting"), this );
    m_actionCollection->addAction( "sorting menu",  m_sortActionMenu );

    m_byNameAction = new KToggleAction( i18n("By Name"), this );
    m_actionCollection->addAction( "by name", m_byNameAction );
    connect( m_byNameAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortByName() ) );

    m_byDateAction = new KToggleAction( i18n("By Date"), this );
    m_actionCollection->addAction( "by date", m_byDateAction );
    connect( m_byDateAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortByDate() ) );

    m_bySizeAction = new KToggleAction( i18n("By Size"), this );
    m_actionCollection->addAction( "by size", m_bySizeAction );
    connect( m_bySizeAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortBySize() ) );

    // TODO: add sort by type

    m_reverseAction = new KToggleAction( i18n("Reverse"), this );
    m_actionCollection->addAction( "reversed", m_reverseAction );
    connect( m_reverseAction, SIGNAL( triggered( bool ) ), this, SLOT( slotSortReversed() ) );

    QActionGroup* sortGroup = new QActionGroup(this);
    m_byNameAction->setActionGroup(sortGroup);
    m_byDateAction->setActionGroup(sortGroup);
    m_bySizeAction->setActionGroup(sortGroup);

    m_dirsFirstAction = new KToggleAction( i18n("Folders First"), this );
    m_actionCollection->addAction( "dirs first", m_dirsFirstAction );
    connect( m_dirsFirstAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleDirsFirst() ));

    // the view menu actions
    m_viewActionMenu = new KActionMenu( i18n("&View"), this );
    m_actionCollection->addAction( "view menu", m_viewActionMenu );
    connect( m_viewActionMenu->menu(), SIGNAL( aboutToShow() ), SLOT( insertViewDependentActions() ));

    m_shortAction = new KToggleAction(i18n("Short View"), this );
    m_actionCollection->addAction( "short view",  m_shortAction );
    m_shortAction->setIcon( KIcon( QLatin1String("fileview-multicolumn") ) );
    connect( m_shortAction, SIGNAL( activated() ), SLOT( slotSimpleView() ));

    m_detailedAction = new KToggleAction( i18n("Detailed View"), this );
    m_actionCollection->addAction( "detailed view", m_detailedAction );
    m_detailedAction->setIcon( KIcon( QLatin1String("fileview-detailed") ) );
    connect( m_detailedAction, SIGNAL( activated() ), SLOT( slotDetailedView() ));

    QActionGroup* viewGroup = new QActionGroup(this);
    m_shortAction->setActionGroup(viewGroup);
    m_detailedAction->setActionGroup(viewGroup);

    m_showHiddenAction = new KToggleAction( i18n("Show Hidden Files"), this );
    m_actionCollection->addAction( "show hidden", m_showHiddenAction );
    connect( m_showHiddenAction, SIGNAL( toggled( bool ) ), SLOT( slotToggleHidden( bool ) ));

    m_separateDirsAction = new KToggleAction( i18n("Separate Folders"), this );
    m_actionCollection->addAction( "separate dirs", m_separateDirsAction );
    connect( m_separateDirsAction, SIGNAL( triggered( bool ) ), this, SLOT(slotSeparateDirs()) );

    KToggleAction *previewAction = new KToggleAction(i18n("Show Preview"), this );
    m_actionCollection->addAction( "preview", previewAction );
    previewAction->setIcon( KIcon( "thumbnail-show" ) );
    connect( previewAction, SIGNAL( toggled( bool )),
             SLOT( togglePreview( bool )));


    QActionGroup* detailGroup = new QActionGroup(this);
    m_shortAction->setActionGroup( detailGroup );
    m_detailedAction->setActionGroup( detailGroup );

    action = new KAction( i18n("Properties"), this );
    m_actionCollection->addAction( "properties", action );
    action->setShortcut( KShortcut(Qt::ALT+Qt::Key_Return) );
    connect( action, SIGNAL( triggered( bool ) ), this, SLOT(slotProperties()) );
}

void KDirOperator::setupMenu()
{
    setupMenu(AllActions);
}

void KDirOperator::setupMenu(int whichActions)
{
    // first fill the submenus (sort and view)
    m_sortActionMenu->menu()->clear();
    m_sortActionMenu->addAction( m_byNameAction );
    m_sortActionMenu->addAction( m_byDateAction );
    m_sortActionMenu->addAction( m_bySizeAction );
    m_sortActionMenu->addAction( m_actionCollection->action("separator")); // TODO
    m_sortActionMenu->addAction( m_reverseAction );
    m_sortActionMenu->addAction( m_dirsFirstAction );

    // now plug everything into the popupmenu
    m_actionMenu->menu()->clear();
    if (whichActions & NavActions)
    {
        m_actionMenu->addAction( m_actionCollection->action("up") );
        m_actionMenu->addAction( m_actionCollection->action("back") );
        m_actionMenu->addAction( m_actionCollection->action("forward") );
        m_actionMenu->addAction( m_actionCollection->action("home") );
        m_actionMenu->addAction( m_actionCollection->action("separator") ); // TODO
    }

    if (whichActions & FileActions)
    {
        m_actionMenu->addAction( m_actionCollection->action("mkdir") );
        if (d->currUrl.isLocalFile() && !(QApplication::keyboardModifiers() & Qt::ShiftModifier))
            m_actionMenu->addAction( m_actionCollection->action( "trash" ) );
        KConfigGroup cg( KGlobal::config(), QLatin1String("KDE") );
        if (!d->currUrl.isLocalFile() || (QApplication::keyboardModifiers() & Qt::ShiftModifier) ||
            cg.readEntry("ShowDeleteCommand", false))
            m_actionMenu->addAction( m_actionCollection->action( "delete" ) );
        m_actionMenu->addAction( m_actionCollection->action("separator") ); // TODO
    }

    if (whichActions & SortActions)
    {
        m_actionMenu->addAction( m_sortActionMenu );
        m_actionMenu->addAction( m_actionCollection->action("separator") ); // TODO
    }

    if (whichActions & ViewActions)
    {
        m_actionMenu->addAction( m_viewActionMenu );
        m_actionMenu->addAction( m_actionCollection->action("separator") ); // TODO
    }

    if (whichActions & FileActions)
    {
        m_actionMenu->addAction( m_actionCollection->action( "properties" ) );
    }
}

void KDirOperator::updateSortActions()
{
    if ( KFile::isSortByName( d->sorting ) )
        m_byNameAction->setChecked( true );
    else if ( KFile::isSortByDate( d->sorting ) )
        m_byDateAction->setChecked( true );
    else if ( KFile::isSortBySize( d->sorting ) )
        m_bySizeAction->setChecked( true );

    m_dirsFirstAction->setChecked( KFile::isSortDirsFirst( d->sorting ) );

    // TODO
    //if ( d->fileView )
    //    m_reverseAction->setChecked( d->fileView->isReversed() );
}

void KDirOperator::updateViewActions()
{
    KFile::FileView fv = static_cast<KFile::FileView>( d->viewKind );

    m_separateDirsAction->setChecked( KFile::isSeparateDirs( fv ) &&
                                    m_separateDirsAction->isEnabled() );

    m_shortAction->setChecked( KFile::isSimpleView( fv ));
    m_detailedAction->setChecked( KFile::isDetailView( fv ));
}

void KDirOperator::readConfig( const KConfigGroup& configGroup)
{
    d->defaultView = 0;
    QDir::SortFlags sorting = QDir::Name;

    QString viewStyle = configGroup.readEntry( "View Style", "Simple" );
    if ( viewStyle == QLatin1String("Detail") )
        d->defaultView |= KFile::Detail;
    else
        d->defaultView |= KFile::Simple;
    if ( configGroup.readEntry( QLatin1String("Separate Directories"),
                        DefaultMixDirsAndFiles ) )
        d->defaultView |= KFile::SeparateDirs;
    if ( configGroup.readEntry(QLatin1String("Show Preview"), false) )
        d->defaultView |= KFile::PreviewContents;

    if ( configGroup.readEntry( QLatin1String("Sort directories first"),
                        DefaultDirsFirst ) )
        sorting |= QDir::DirsFirst;


    QString name = QLatin1String("Name");
    QString sortBy = configGroup.readEntry( QLatin1String("Sort by"), name );
    if ( sortBy == name )
        sorting |= QDir::Name;
    else if ( sortBy == QLatin1String("Size") )
        sorting |= QDir::Size;
    else if ( sortBy == QLatin1String("Date") )
        sorting |= QDir::Time;

    d->sorting = sorting;
    setSorting( d->sorting );


    if ( configGroup.readEntry( QLatin1String("Show hidden files"),
                        DefaultShowHidden ) ) {
         m_showHiddenAction->setChecked( true );
         d->dirLister->setShowingDotFiles( true );
    }
    if ( configGroup.readEntry( QLatin1String("Sort reversed"),
                        DefaultSortReversed ) )
        m_reverseAction->setChecked( true );

}

void KDirOperator::writeConfig( KConfigGroup& configGroup)
{
    QString sortBy = QLatin1String("Name");
    if ( KFile::isSortBySize( d->sorting ) )
        sortBy = QLatin1String("Size");
    else if ( KFile::isSortByDate( d->sorting ) )
        sortBy = QLatin1String("Date");
    configGroup.writeEntry( QLatin1String("Sort by"), sortBy );

    configGroup.writeEntry( QLatin1String("Sort reversed"),
                    m_reverseAction->isChecked() );
    configGroup.writeEntry( QLatin1String("Sort directories first"),
                    m_dirsFirstAction->isChecked() );

    // don't save the separate dirs or preview when an application specific
    // preview is in use.
    bool appSpecificPreview = false;
    if ( d->preview ) {
        QWidget *preview = const_cast<QWidget*>( d->preview ); // grmbl
        KFileMetaPreview *tmp = dynamic_cast<KFileMetaPreview*>( preview );
        appSpecificPreview = (tmp == 0L);
    }

    if ( !appSpecificPreview ) {
        if ( m_separateDirsAction->isEnabled() )
            configGroup.writeEntry( QLatin1String("Separate Directories"),
                            m_separateDirsAction->isChecked() );

       KToggleAction *previewAction = static_cast<KToggleAction*>(m_actionCollection->action("preview"));
        if ( previewAction->isEnabled() ) {
            bool hasPreview = previewAction->isChecked();
            configGroup.writeEntry( QLatin1String("Show Preview"), hasPreview );
        }
    }

    configGroup.writeEntry( QLatin1String("Show hidden files"),
                    m_showHiddenAction->isChecked() );

    KFile::FileView fv = static_cast<KFile::FileView>( d->viewKind );
    QString style;
    if ( KFile::isDetailView( fv ) )
        style = QLatin1String("Detail");
    else if ( KFile::isSimpleView( fv ) )
        style = QLatin1String("Simple");
    configGroup.writeEntry( QLatin1String("View Style"), style );

}


void KDirOperator::resizeEvent( QResizeEvent * )
{
    if (d->itemView) {
        d->itemView->resize(size());
    }

    if (d->progressBar->parent() == this) {
        // might be reparented into a statusbar
        d->progressBar->move(2, height() - d->progressBar->height() - 2);
    }
}

void KDirOperator::setOnlyDoubleClickSelectsFiles( bool enable )
{
    d->onlyDoubleClickSelectsFiles = enable;
    //if ( d->fileView )
    //    d->fileView->setOnlyDoubleClickSelectsFiles( enable );
}

bool KDirOperator::onlyDoubleClickSelectsFiles() const
{
    return d->onlyDoubleClickSelectsFiles;
}

void KDirOperator::slotStarted()
{
    d->progressBar->setValue( 0 );
    // delay showing the progressbar for one second
    d->progressDelayTimer->setSingleShot( true );
    d->progressDelayTimer->start( 1000 );
}

void KDirOperator::slotShowProgress()
{
    d->progressBar->raise();
    d->progressBar->show();
    QApplication::flush();
}

void KDirOperator::slotProgress( int percent )
{
    d->progressBar->setValue( percent );
    // we have to redraw this as fast as possible
    if ( d->progressBar->isVisible() )
        QApplication::flush();
}


void KDirOperator::slotIOFinished()
{
    d->progressDelayTimer->stop();
    slotProgress( 100 );
    d->progressBar->hide();
    emit finishedLoading();
    resetCursor();

    //if ( d->fileView )
    //    d->fileView->listingCompleted();
}

void KDirOperator::slotCanceled()
{
    emit finishedLoading();
    resetCursor();

    //if ( d->fileView )
    //    d->fileView->listingCompleted();
}

QProgressBar * KDirOperator::progressBar() const
{
    return d->progressBar;
}

void KDirOperator::clearHistory()
{
    qDeleteAll( d->backStack );
    d->backStack.clear();
    m_actionCollection->action("back")->setEnabled(false);

    qDeleteAll( d->forwardStack );
    d->forwardStack.clear();
    m_actionCollection->action("forward")->setEnabled( false );
}

void KDirOperator::slotViewActionAdded( KAction *action )
{
    if ( m_viewActionMenu->menu()->actions().count() == 5 ) // need to add a separator
        m_viewActionMenu->addAction( d->viewActionSeparator );

    m_viewActionMenu->addAction( action );
}

void KDirOperator::slotViewActionRemoved( KAction *action )
{
    m_viewActionMenu->removeAction( action );

    if ( m_viewActionMenu->menu()->actions().count() == 6 ) // remove the separator
        m_viewActionMenu->removeAction( d->viewActionSeparator );
}

void KDirOperator::slotViewSortingChanged( QDir::SortFlags sort )
{
    d->sorting = sort;
    updateSortActions();
}

void KDirOperator::setEnableDirHighlighting( bool enable )
{
    d->dirHighlighting = enable;
}

bool KDirOperator::dirHighlighting() const
{
    return d->dirHighlighting;
}

bool KDirOperator::dirOnlyMode() const
{
    return dirOnlyMode( d->mode );
}

bool KDirOperator::dirOnlyMode( uint mode )
{
    return ( (mode & KFile::Directory) &&
        (mode & (KFile::File | KFile::Files)) == 0 );
}

void KDirOperator::slotProperties()
{
    // TODO
    /*if ( d->fileView ) {
        const KFileItemList *list = d->fileView->selectedItems();
        if ( !list->isEmpty() ) {
            KPropertiesDialog::showDialog( *list, this );
        }
    }*/
}

void KDirOperator::slotClearView()
{
    // TODO
    //if ( d->fileView )
    //    d->fileView->clearView();
}

void KDirOperator::slotPressed(const QModelIndex& index)
{
    // Remember whether the left mouse button has been pressed, to prevent
    // that a right-click on an item opens an item (see slotClicked(),
    // slotDoubleClicked() and openContextMenu()).
    d->leftButtonPressed = (QApplication::mouseButtons() & Qt::LeftButton);
}

void KDirOperator::slotClicked(const QModelIndex& index)
{
    if (!d->leftButtonPressed) {
        return;
    }

    const QModelIndex dirIndex = d->proxyModel->mapToSource(index);
    KFileItem item = d->dirModel->itemForIndex(dirIndex);
    if (item.isDir()) {
        if (KGlobalSettings::singleClick()) {
            selectDir(&item);
        } else {
            highlightFile(&item);
        }
    } else {
        highlightFile(&item);
    }
}

void KDirOperator::slotDoubleClicked(const QModelIndex& index)
{
    if (!d->leftButtonPressed) {
        return;
    }

    const QModelIndex dirIndex = d->proxyModel->mapToSource(index);
    KFileItem item = d->dirModel->itemForIndex(dirIndex);
    if (item.isDir()) {
        selectDir(&item);
    } else {
        selectFile(&item);
    }
}

void KDirOperator::openContextMenu(const QPoint& pos)
{
    d->leftButtonPressed = false;

    const QModelIndex proxyIndex = d->itemView->indexAt(pos);
    const QModelIndex dirIndex = d->proxyModel->mapToSource(proxyIndex);
    KFileItem item = d->dirModel->itemForIndex(dirIndex);
    activatedMenu(item.isNull() ? 0 : &item, QCursor::pos());
}

// ### temporary code
#include <dirent.h>
bool KDirOperator::isReadable( const KUrl& url )
{
    if ( !url.isLocalFile() )
        return true; // what else can we say?

    KDE_struct_stat buf;
#ifdef Q_WS_WIN
    QString ts = url.toLocalFile();
#else
    QString ts = url.path(KUrl::AddTrailingSlash);
#endif
    bool readable = ( KDE_stat( QFile::encodeName( ts ), &buf) == 0 );
    if (readable) { // further checks
        DIR *test;
        test = opendir( QFile::encodeName( ts )); // we do it just to test here
        readable = (test != 0);
        if (test)
            closedir(test);
    }
    return readable;
}

void KDirOperator::togglePreview( bool on )
{
    if ( on )
        slotDefaultPreview();
    else
        setView( (KFile::FileView) (d->viewKind & ~(KFile::PreviewContents|KFile::PreviewInfo)) );
}

void KDirOperator::slotRefreshItems( const KFileItemList& items )
{
    // TODO:
    /*if ( !d->fileView )
        return;

    KFileItemList::const_iterator kit = items.begin();
    const KFileItemList::const_iterator kend = items.end();
    for ( ; kit != kend; ++kit )
        d->fileView->updateView( *kit );*/
}

void KDirOperator::setViewConfig( KConfigGroup& configGroup)
{
    delete d->configGroup;
    d->configGroup = new KConfigGroup(configGroup);
}

KConfigGroup* KDirOperator::viewConfigGroup() const
{
    return d->configGroup;
}

void KDirOperator::setShowHiddenFiles( bool s )
{
    m_showHiddenAction->setChecked( s );
}

bool KDirOperator::showHiddenFiles( ) const
{
    return m_showHiddenAction->isChecked();
}

#include "kdiroperator.moc"
