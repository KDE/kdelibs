/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Stephan Kulow <coolo@kde.org>
                  1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <unistd.h>

#include <qdir.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kdirlister.h>
#include <klineeditdlg.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprogress.h>
#include <kstdaction.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/previewjob.h>
#include <kpropertiesdialog.h>
#include <kservicetypefactory.h>
#include <kstdaccel.h>

#include "config-kfile.h"
#include "kcombiview.h"
#include "kdiroperator.h"
#include "kfiledetailview.h"
#include "kfileiconview.h"
#include "kfilepreview.h"
#include "kfileview.h"
#include "kfileitem.h"
#include "kimagefilepreview.h"


template class QPtrStack<KURL>;
template class QDict<KFileItem>;


class KDirOperator::KDirOperatorPrivate
{
public:
    KDirOperatorPrivate() {
        onlyDoubleClickSelectsFiles = false;
        progressDelayTimer = 0L;
        dirHighlighting = false;
    }

    ~KDirOperatorPrivate() {
        delete progressDelayTimer;
    }

    bool dirHighlighting;
    QString lastURL; // used for highlighting a directory on cdUp
    bool onlyDoubleClickSelectsFiles;
    QTimer *progressDelayTimer;
};

KDirOperator::KDirOperator(const KURL& _url,
                           QWidget *parent, const char* _name)
    : QWidget(parent, _name),
      dir(0),
      m_fileView(0),
      progress(0)
{
    myPreview = 0L;
    myMode = KFile::File;
    m_viewKind = KFile::Simple;
    mySorting = static_cast<QDir::SortSpec>(QDir::Name | QDir::DirsFirst);
    d = new KDirOperatorPrivate;

    if (_url.isEmpty()) { // no dir specified -> current dir
        QString strPath = QDir::currentDirPath();
        strPath.append('/');
        currUrl = KURL();
        currUrl.setProtocol(QString::fromLatin1("file"));
        currUrl.setPath(strPath);
    }
    else {
        currUrl = _url;
        if ( currUrl.protocol().isEmpty() )
            currUrl.setProtocol(QString::fromLatin1("file"));

        currUrl.addPath("/"); // make sure we have a trailing slash!
    }

    setDirLister( new KDirLister( true ) );

    connect(&myCompletion, SIGNAL(match(const QString&)),
            SLOT(slotCompletionMatch(const QString&)));

    progress = new KProgress(this, "progress");
    progress->adjustSize();
    progress->move(2, height() - progress->height() -2);

    d->progressDelayTimer = new QTimer( this, "progress delay timer" );
    connect( d->progressDelayTimer, SIGNAL( timeout() ),
	     SLOT( slotShowProgress() ));

    myCompleteListDirty = false;

    backStack.setAutoDelete( true );
    forwardStack.setAutoDelete( true );

    // action stuff
    setupActions();
    setupMenu();

    setFocusPolicy(QWidget::WheelFocus);
}

KDirOperator::~KDirOperator()
{
    resetCursor();
    delete m_fileView;
    m_fileView = 0L;
    delete myPreview;
    delete dir;
    delete d;
}


void KDirOperator::setSorting( QDir::SortSpec spec )
{
    if ( m_fileView )
        m_fileView->setSorting( spec );
    mySorting = spec;
    updateSortActions();
}

void KDirOperator::resetCursor()
{
    QApplication::restoreOverrideCursor();
    progress->hide();
}

void KDirOperator::activatedMenu( const KFileItem *, const QPoint& pos )
{
    // If we have a new view actionCollection(), insert its actions
    //  into viewActionMenu.
    if( m_fileView && viewActionCollection != m_fileView->actionCollection() ) {
        viewActionCollection = m_fileView->actionCollection();

        if ( !viewActionCollection->isEmpty() ) {
            viewActionMenu->insert( actionSeparator );
            for ( uint i = 0; i < viewActionCollection->count(); i++ )
                viewActionMenu->insert( viewActionCollection->action( i ));
        }

        connect( viewActionCollection, SIGNAL( inserted( KAction * )),
                 SLOT( slotViewActionAdded( KAction * )));
        connect( viewActionCollection, SIGNAL( removed( KAction * )),
                 SLOT( slotViewActionRemoved( KAction * )));
    }

    updateSelectionDependentActions();

    actionMenu->popup( pos );
}

void KDirOperator::updateSelectionDependentActions()
{
    bool hasSelection = m_fileView && m_fileView->selectedItems() &&
                        !m_fileView->selectedItems()->isEmpty();
    myActionCollection->action( "delete" )->setEnabled( hasSelection );
    myActionCollection->action( "properties" )->setEnabled( hasSelection );
}

void KDirOperator::setPreviewWidget(const QWidget *w)
{
    if(w != 0L)
        m_viewKind = (m_viewKind | KFile::PreviewContents);
    else
        m_viewKind = (m_viewKind & ~KFile::PreviewContents);

    delete myPreview;
    myPreview = w;

    KToggleAction *preview = static_cast<KToggleAction*>(myActionCollection->action("preview"));
    preview->setEnabled( w != 0L );
    preview->setChecked( w != 0L );
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

int KDirOperator::numDirs() const
{
    return m_fileView ? m_fileView->numDirs() : 0;
}

int KDirOperator::numFiles() const
{
    return m_fileView ? m_fileView->numFiles() : 0;
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Simple) | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (m_viewKind & ~KFile::Detail) | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    dir->setShowingDotFiles( show );
    updateDir();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotSeparateDirs()
{
    if (separateDirsAction->isChecked())
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind | KFile::SeparateDirs );
        setView( view );
    }
    else
    {
        KFile::FileView view = static_cast<KFile::FileView>( m_viewKind & ~KFile::SeparateDirs );
        setView( view );
    }
}

void KDirOperator::slotDefaultPreview()
{
    m_viewKind = m_viewKind | KFile::PreviewContents;
    if ( !myPreview ) {
        myPreview = new KImageFilePreview( this );
        (static_cast<KToggleAction*>( myActionCollection->action("preview") ))->setChecked(true);
    }

    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::slotSortByName()
{
    int sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Name ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( true );
}

void KDirOperator::slotSortBySize()
{
    int sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Size ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortByDate()
{
    int sorting = (m_fileView->sorting()) & ~QDir::SortByMask;
    m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Time ));
    mySorting = m_fileView->sorting();
    caseInsensitiveAction->setEnabled( false );
}

void KDirOperator::slotSortReversed()
{
    if ( m_fileView )
        m_fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    QDir::SortSpec sorting = m_fileView->sorting();
    if ( !KFile::isSortDirsFirst( sorting ) )
        m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::DirsFirst ));
    else
        m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::DirsFirst));
    mySorting = m_fileView->sorting();
}

void KDirOperator::slotToggleIgnoreCase()
{
    QDir::SortSpec sorting = m_fileView->sorting();
    if ( !KFile::isSortCaseInsensitive( sorting ) )
        m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::IgnoreCase ));
    else
        m_fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::IgnoreCase));
    mySorting = m_fileView->sorting();
}

void KDirOperator::mkdir()
{
    KLineEditDlg dlg(i18n("Create new directory in:") +
                     QString::fromLatin1( "\n" ) + /* don't break i18n now*/
                     url().prettyURL(), i18n("New Directory"), this);
    dlg.setCaption(i18n("New Directory"));
    if (dlg.exec()) {
      mkdir( dlg.text(), true );
    }
}

bool KDirOperator::mkdir( const QString& directory, bool enterDirectory )
{
    bool writeOk = false;
    KURL url( currUrl );
    url.addPath(directory);

    if ( url.isLocalFile() ) {
        // check if we are allowed to create local directories
        writeOk = checkAccess( url.path(), W_OK );
        if ( writeOk )
            writeOk = QDir().mkdir( url.path() );
    }
    else
        writeOk = KIO::NetAccess::mkdir( url );

    if ( !writeOk )
        KMessageBox::sorry(viewWidget(), i18n("You don't have permission to "
                                              "create that directory." ));
    else {
        if ( enterDirectory )
            setURL( url, true );
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
                                i18n("You didn't select a file to delete."),
                                i18n("Nothing to delete") );
        return 0L;
    }

    KURL::List urls;
    QStringList files;
    KFileItemListIterator it( items );

    for ( ; it; ++it ) {
        KURL url = (*it)->url();
        urls.append( url );
        if ( url.isLocalFile() )
            files.append( url.path() );
        else
            files.append( url.prettyURL() );
    }

    bool doIt = !ask;
    if ( ask ) {
        int ret;
        if ( items.count() == 1 ) {
            ret = KMessageBox::warningContinueCancel( parent,
                i18n( "<qt>Do you really want to delete\n <b>'%1'</b>?</qt>" )
                .arg( files.first() ),
                                                      i18n("Delete File"),
                                                      i18n("Delete") );
        }
        else
            ret = KMessageBox::warningContinueCancelList( parent,
                i18n("Do you really want to delete this item?", "Do you really want to delete these %n items?", items.count() ),
                                                    files,
                                                    i18n("Delete Files"),
                                                    i18n("Delete") );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt )
        return KIO::del( urls, false, showProgress );

    return 0L;
}

void KDirOperator::deleteSelected()
{
    if ( !m_fileView )
        return;

    const KFileItemList *list = m_fileView->selectedItems();
    if ( list )
        del( *list );
}

void KDirOperator::close()
{
    resetCursor();
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    dir->stop();
}

void KDirOperator::checkPath(const QString &, bool /*takeFiles*/) // SLOT
{
#if 0
    // copy the argument in a temporary string
    QString text = _txt;
    // it's unlikely to happen, that at the beginning are spaces, but
    // for the end, it happens quite often, I guess.
    text = text.stripWhiteSpace();
    // if the argument is no URL (the check is quite fragil) and it's
    // no absolute path, we add the current directory to get a correct url
    if (text.find(':') < 0 && text[0] != '/')
        text.insert(0, currUrl);

    // in case we have a selection defined and someone patched the file-
    // name, we check, if the end of the new name is changed.
    if (!selection.isNull()) {
        int position = text.findRev('/');
        ASSERT(position >= 0); // we already inserted the current dir in case
        QString filename = text.mid(position + 1, text.length());
        if (filename != selection)
            selection = QString::null;
    }

    KURL u(text); // I have to take care of entered URLs
    bool filenameEntered = false;

    if (u.isLocalFile()) {
        // the empty path is kind of a hack
        KFileItem i("", u.path());
        if (i.isDir())
            setURL(text, true);
        else {
            if (takeFiles)
                if (acceptOnlyExisting && !i.isFile())
                    warning("you entered an invalid URL");
                else
                    filenameEntered = true;
        }
    } else
        setURL(text, true);

    if (filenameEntered) {
        filename_ = u.url();
        emit fileSelected(filename_);

        QApplication::restoreOverrideCursor();

        accept();
    }
#endif
    kdDebug(kfile_area) << "TODO KDirOperator::checkPath()" << endl;
}

void KDirOperator::setURL(const KURL& _newurl, bool clearforward)
{
    KURL newurl;

    if ( _newurl.isMalformed() )
	newurl.setPath( QDir::homeDirPath() );
    else
	newurl = _newurl;

    QString pathstr = newurl.path(+1);
    newurl.setPath(pathstr);

    // already set
    if ( newurl.cmp( currUrl, true ) )
        return;

    if ( !isReadable( newurl ) ) {
        // maybe newurl is a file? check its parent directory
        newurl.cd(QString::fromLatin1(".."));
        if ( !isReadable( newurl ) ) {
            resetCursor();
            KMessageBox::error(viewWidget(),
                               i18n("The specified directory does not exist "
                                    "or was not readable."));
            return;
        }
    }

    if (clearforward) {
        // autodelete should remove this one
        backStack.push(new KURL(currUrl));
        forwardStack.clear();
    }

    d->lastURL = currUrl.url(-1);
    currUrl = newurl;

    pathChanged();
    emit urlEntered(newurl);

    // enable/disable actions
    forwardAction->setEnabled( !forwardStack.isEmpty() );
    backAction->setEnabled( !backStack.isEmpty() );
    upAction->setEnabled( !isRoot() );

    dir->openURL( newurl );
}

void KDirOperator::updateDir()
{
    dir->emitChanges();
    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::rereadDir()
{
    pathChanged();
    dir->openURL( currUrl, false, true );
}

// Protected
void KDirOperator::pathChanged()
{
    if (!m_fileView)
        return;

    pendingMimeTypes.clear();
    m_fileView->clear();
    myCompletion.clear();
    myDirCompletion.clear();

    // it may be, that we weren't ready at this time
    QApplication::restoreOverrideCursor();

    // when KIO::Job emits finished, the slot will restore the cursor
    QApplication::setOverrideCursor( waitCursor );

    if ( !isReadable( currUrl )) {
        KMessageBox::error(viewWidget(),
                           i18n("The specified directory does not exist "
                                "or was not readable."));
        if (backStack.isEmpty())
            home();
        else
            back();
    }
}

void KDirOperator::slotRedirected( const KURL& newURL )
{
    currUrl = newURL;
    pendingMimeTypes.clear();
    myCompletion.clear();
    myDirCompletion.clear();
    myCompleteListDirty = true;
    emit urlEntered( newURL );
}

// Code pinched from kfm then hacked
void KDirOperator::back()
{
    if ( backStack.isEmpty() )
        return;

    forwardStack.push( new KURL(currUrl) );

    KURL *s = backStack.pop();

    setURL(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( forwardStack.isEmpty() )
        return;

    backStack.push(new KURL(currUrl));

    KURL *s = forwardStack.pop();
    setURL(*s, false);
    delete s;
}

KURL KDirOperator::url() const
{
    return currUrl;
}

void KDirOperator::cdUp()
{
    KURL tmp( currUrl );
    tmp.cd(QString::fromLatin1(".."));
    setURL(tmp, true);
}

void KDirOperator::home()
{
    setURL(QDir::homeDirPath(), true);
}

void KDirOperator::clearFilter()
{
    dir->setNameFilter( QString::null );
    dir->clearMimeFilter();
    checkPreviewSupport();
}

void KDirOperator::setNameFilter(const QString& filter)
{
    dir->setNameFilter(filter);
    checkPreviewSupport();
}

void KDirOperator::setMimeFilter( const QStringList& mimetypes )
{
    dir->setMimeFilter( mimetypes );
    checkPreviewSupport();
}

bool KDirOperator::checkPreviewSupport()
{
    KToggleAction *previewAction = static_cast<KToggleAction*>( myActionCollection->action( "preview" ));

    bool hasPreviewSupport = false;
    KConfig *kc = KGlobal::config();
    KConfigGroupSaver cs( kc, ConfigGroup );
    if ( kc->readBoolEntry( "Show Default Preview", true ) )
        hasPreviewSupport = checkPreviewInternal();

    previewAction->setEnabled( hasPreviewSupport );
    return hasPreviewSupport;
}

bool KDirOperator::checkPreviewInternal() const
{
    QStringList supported = KIO::PreviewJob::supportedMimeTypes();
    // no preview support for directories?
    if ( dirOnlyMode() && supported.findIndex( "inode/directory" ) == -1 )
        return false;

    QStringList mimeTypes = dir->mimeFilters();
    QStringList nameFilter = QStringList::split( " ", dir->nameFilter() );

    if ( mimeTypes.isEmpty() && nameFilter.isEmpty() && !supported.isEmpty() )
        return true;
    else {
        QRegExp r;
        r.setWildcard( true ); // the "mimetype" can be "image/*"

        if ( !mimeTypes.isEmpty() ) {
            QStringList::Iterator it = supported.begin();

            for ( ; it != supported.end(); ++it ) {
                r.setPattern( *it );

                QStringList result = mimeTypes.grep( r );
                if ( !result.isEmpty() ) { // matches! -> we want previews
                    return true;
                }
            }
        }

        if ( !nameFilter.isEmpty() ) {
            // find the mimetypes of all the filter-patterns and
            KServiceTypeFactory *fac = KServiceTypeFactory::self();
            QStringList::Iterator it1 = nameFilter.begin();
            for ( ; it1 != nameFilter.end(); ++it1 ) {
                if ( (*it1) == "*" ) {
                    return true;
                }

                KMimeType *mt = fac->findFromPattern( *it1 );
                if ( !mt )
                    continue;
                QString mime = mt->name();
                delete mt;

                // the "mimetypes" we get from the PreviewJob can be "image/*"
                // so we need to check in wildcard mode
                QStringList::Iterator it2 = supported.begin();
                for ( ; it2 != supported.end(); ++it2 ) {
                    r.setPattern( *it2 );
                    if ( r.search( mime ) != -1 ) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

KFileView* KDirOperator::createView( QWidget* parent, KFile::FileView view ) {
    KFileView* new_view = 0L;
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview = ( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if ( separateDirs || preview ) {
        KCombiView *combi = 0L;
        if (separateDirs)
        {
            combi = new KCombiView( parent, "combi view" );
            combi->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
        }

        KFileView* v = 0L;
        if ( KFile::isSimpleView( view ) )
            v = createView( combi, KFile::Simple );
        else
            v = createView( combi, KFile::Detail );

        if (combi)
            combi->setRight( v );

        if (preview)
        {
            KFilePreview* pView = new KFilePreview( combi ? combi : v, parent, "preview" );
            pView->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
            new_view = pView;
        }
        else
            new_view = combi;
    }
    else if ( KFile::isDetailView( view ) && !preview ) {
        new_view = new KFileDetailView( parent, "detail view");
        new_view->setViewName( i18n("Detailed View") );
    }
    else /* if ( KFile::isSimpleView( view ) && !preview ) */ {
        new_view = new KFileIconView( parent, "simple view");
        new_view->setViewName( i18n("Short View") );
    }

    return new_view;
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview=( KFile::isPreviewInfo(view) || KFile::isPreviewContents( view ) );

    if (view == KFile::Default) {
        if ( KFile::isDetailView( (KFile::FileView) defaultView ) )
            view = KFile::Detail;
        else
            view = KFile::Simple;

        separateDirs = KFile::isSeparateDirs( static_cast<KFile::FileView>(defaultView) );
        preview = ( KFile::isPreviewInfo( static_cast<KFile::FileView>(defaultView) ) ||
                    KFile::isPreviewContents( static_cast<KFile::FileView>(defaultView) ) )
                  && myActionCollection->action("preview")->isEnabled();

        if ( preview ) { // instantiates KImageFilePreview and calls setView()
            m_viewKind = view;
            slotDefaultPreview();
            return;
        }
        else if ( !separateDirs )
            separateDirsAction->setChecked(true);
    }

    // if we don't have any files, we can't separate dirs from files :)
    if ( (mode() & KFile::File) == 0 &&
         (mode() & KFile::Files) == 0 ) {
        separateDirs = false;
        separateDirsAction->setEnabled( false );
    }

    m_viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);
    view = static_cast<KFile::FileView>(m_viewKind);

    KFileView *new_view = createView( this, view );
    if ( preview ) {
        // we keep the preview-_widget_ around, but not the KFilePreview.
        // KFilePreview::setPreviewWidget handles the reparenting for us
        static_cast<KFilePreview*>(new_view)->setPreviewWidget(myPreview, url());
    }

    setView( new_view );
}


void KDirOperator::connectView(KFileView *view)
{
    // TODO: do a real timer and restart it after that
    pendingMimeTypes.clear();
    bool listDir = true;

    if ( dirOnlyMode() )
         view->setViewMode(KFileView::Directories);
    else
        view->setViewMode(KFileView::All);

    if ( myMode & KFile::Files )
        view->setSelectionMode( KFile::Extended );
    else
        view->setSelectionMode( KFile::Single );

    if (m_fileView) {
        // transfer the state from old view to new view
        view->clear();
        view->addItemList( *m_fileView->items() );
        listDir = false;

        if ( m_fileView->widget()->hasFocus() )
            view->widget()->setFocus();

        KFileItem *oldCurrentItem = m_fileView->currentFileItem();
        if ( oldCurrentItem ) {
            view->setCurrentItem( oldCurrentItem );
            view->setSelected( oldCurrentItem, false );
            view->ensureItemVisible( oldCurrentItem );
        }

        const KFileItemList *oldSelected = m_fileView->selectedItems();
        if ( !oldSelected->isEmpty() ) {
            KFileItemListIterator it( *oldSelected );
            for ( ; it.current(); ++it )
                view->setSelected( it.current(), true );
        }

        m_fileView->widget()->hide();
        delete m_fileView;
    }

    m_fileView = view;
    viewActionCollection = 0;
    KFileViewSignaler *sig = view->signaler();

    connect(sig, SIGNAL( activatedMenu(const KFileItem *, const QPoint& ) ),
            this, SLOT( activatedMenu(const KFileItem *, const QPoint& )));
    connect(sig, SIGNAL( dirActivated(const KFileItem *) ),
            this, SLOT( selectDir(const KFileItem*) ) );
    connect(sig, SIGNAL( fileSelected(const KFileItem *) ),
            this, SLOT( selectFile(const KFileItem*) ) );
    connect(sig, SIGNAL( fileHighlighted(const KFileItem *) ),
            this, SLOT( highlightFile(const KFileItem*) ));
    connect(sig, SIGNAL( sortingChanged( QDir::SortSpec ) ),
            this, SLOT( slotViewSortingChanged( QDir::SortSpec )));

    if ( reverseAction->isChecked() != m_fileView->isReversed() )
        slotSortReversed();

    updateViewActions();
    m_fileView->widget()->resize(size());
    m_fileView->widget()->show();

    if ( listDir ) {
        QApplication::setOverrideCursor( waitCursor );
        dir->openURL( currUrl );
    }
    else
        view->listingCompleted();
}

KFile::Mode KDirOperator::mode() const
{
    return myMode;
}

void KDirOperator::setMode(KFile::Mode m)
{
    if (myMode == m)
        return;

    myMode = m;

    dir->setDirOnlyMode( dirOnlyMode() );

    // reset the view with the different mode
    setView( static_cast<KFile::FileView>(m_viewKind) );
}

void KDirOperator::setView(KFileView *view)
{
    if ( view == m_fileView ) {
        return;
    }

    setFocusProxy(view->widget());
    view->setSorting( mySorting );
    view->setOnlyDoubleClickSelectsFiles( d->onlyDoubleClickSelectsFiles );
    connectView(view); // also deletes the old view

    emit viewChanged( view );
}

void KDirOperator::setDirLister( KDirLister *lister )
{
    if ( lister == dir ) // sanity check
        return;

    delete dir;
    dir = lister;

    dir->setAutoUpdate( true );

    connect( dir, SIGNAL( percent( int )),
             SLOT( slotProgress( int ) ));
    connect( dir, SIGNAL(started( const KURL& )), SLOT(slotStarted()));
    connect( dir, SIGNAL(newItems(const KFileItemList &)),
             SLOT(insertNewFiles(const KFileItemList &)));
    connect( dir, SIGNAL(completed()), SLOT(slotIOFinished()));
    connect( dir, SIGNAL(canceled()), SLOT(slotCanceled()));
    connect( dir, SIGNAL(deleteItem(KFileItem *)),
             SLOT(itemDeleted(KFileItem *)));
    connect( dir, SIGNAL(redirection( const KURL& )),
	     SLOT( slotRedirected( const KURL& )));
    connect( dir, SIGNAL( clear() ), SLOT( slotClearView() ));
    connect( dir, SIGNAL( refreshItems( const KFileItemList& ) ),
             SLOT( slotRefreshItems( const KFileItemList& ) ) );
}

void KDirOperator::insertNewFiles(const KFileItemList &newone)
{
    if ( newone.isEmpty() || !m_fileView )
        return;

    myCompleteListDirty = true;
    m_fileView->addItemList( newone );
    emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());

    KFileItem *item;
    KFileItemListIterator it( newone );

    while ( (item = it.current()) ) {
	// highlight the dir we come from, if possible
	if ( d->dirHighlighting && item->isDir() &&
	     item->url().url(-1) == d->lastURL ) {
	    m_fileView->setCurrentItem( item );
	    m_fileView->ensureItemVisible( item );
	}

	++it;
    }

    QTimer::singleShot(200, this, SLOT(resetCursor()));
}

void KDirOperator::selectDir(const KFileItem *item)
{
    setURL(item->url(), true);
}

void KDirOperator::itemDeleted(KFileItem *item)
{
    pendingMimeTypes.removeRef( item );
    m_fileView->removeItem( static_cast<KFileItem *>( item ));
    emit updateInformation(m_fileView->numDirs(), m_fileView->numFiles());
}

void KDirOperator::selectFile(const KFileItem *item)
{
    QApplication::restoreOverrideCursor();

    emit fileSelected( item );
}

void KDirOperator::setCurrentItem( const QString& filename )
{
    if ( m_fileView ) {
        const KFileItem *item = 0L;

        if ( !filename.isNull() )
            item = static_cast<KFileItem *>(dir->findByName( filename ));

        m_fileView->clearSelection();
        if ( item ) {
            m_fileView->setCurrentItem( item );
            m_fileView->setSelected( item, true );
            m_fileView->ensureItemVisible( item );
        }
    }
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return QString::null;
    }

    prepareCompletionObjects();
    return myCompletion.makeCompletion( string );
}

QString KDirOperator::makeDirCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        m_fileView->clearSelection();
        return QString::null;
    }

    prepareCompletionObjects();
    return myDirCompletion.makeCompletion( string );
}

void KDirOperator::prepareCompletionObjects()
{
    if ( !m_fileView )
	return;

    if ( myCompleteListDirty ) { // create the list of all possible completions
        KFileItemListIterator it( *(m_fileView->items()) );
        for( ; it.current(); ++it ) {
            KFileItem *item = it.current();

            myCompletion.addItem( item->name() );
            if ( item->isDir() )
                myDirCompletion.addItem( item->name() );
        }
        myCompleteListDirty = false;
    }
}

void KDirOperator::slotCompletionMatch(const QString& match)
{
    setCurrentItem( match );
    emit completion( match );
}

void KDirOperator::setupActions()
{
    myActionCollection = new KActionCollection( this, "KDirOperator::myActionCollection" );
    actionMenu = new KActionMenu( i18n("Menu"), myActionCollection, "popupMenu" );

    upAction = KStdAction::up( this, SLOT( cdUp() ), myActionCollection, "up" );
    upAction->setText( i18n("Parent Directory") );
    backAction = KStdAction::back( this, SLOT( back() ), myActionCollection, "back" );
    forwardAction = KStdAction::forward( this, SLOT(forward()), myActionCollection, "forward" );
    homeAction = KStdAction::home( this, SLOT( home() ), myActionCollection, "home" );
    homeAction->setText(i18n("Home Directory"));
    reloadAction = KStdAction::redisplay( this, SLOT(rereadDir()), myActionCollection, "reload" );
    actionSeparator = new KActionSeparator( myActionCollection, "separator" );
    mkdirAction = new KAction( i18n("New Directory..."), 0,
                                 this, SLOT( mkdir() ), myActionCollection, "mkdir" );
    new KAction( i18n( "Delete" ), "editdelete", Key_Delete, this,
                  SLOT( deleteSelected() ), myActionCollection, "delete" );
    mkdirAction->setIcon( QString::fromLatin1("folder_new") );
    reloadAction->setText( i18n("Reload") );
    reloadAction->setShortcut( KStdAccel::shortcut( KStdAccel::Reload ));


    // the sort menu actions
    sortActionMenu = new KActionMenu( i18n("Sorting"), myActionCollection, "sorting menu");
    byNameAction = new KRadioAction( i18n("By Name"), 0,
                                     this, SLOT( slotSortByName() ),
                                     myActionCollection, "by name" );
    byDateAction = new KRadioAction( i18n("By Date"), 0,
                                     this, SLOT( slotSortByDate() ),
                                     myActionCollection, "by date" );
    bySizeAction = new KRadioAction( i18n("By Size"), 0,
                                     this, SLOT( slotSortBySize() ),
                                     myActionCollection, "by size" );
    reverseAction = new KToggleAction( i18n("Reverse"), 0,
                                       this, SLOT( slotSortReversed() ),
                                       myActionCollection, "reversed" );

    QString sortGroup = QString::fromLatin1("sort");
    byNameAction->setExclusiveGroup( sortGroup );
    byDateAction->setExclusiveGroup( sortGroup );
    bySizeAction->setExclusiveGroup( sortGroup );


    dirsFirstAction = new KToggleAction( i18n("Directories First"), 0,
                                         myActionCollection, "dirs first");
    caseInsensitiveAction = new KToggleAction(i18n("Case Insensitive"), 0,
                                              myActionCollection, "case insensitive" );

    connect( dirsFirstAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleDirsFirst() ));
    connect( caseInsensitiveAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleIgnoreCase() ));



    // the view menu actions
    viewActionMenu = new KActionMenu( i18n("View"), myActionCollection, "view menu" );
    shortAction = new KRadioAction( i18n("Short View"), "view_multicolumn",
                                    KShortcut(), myActionCollection, "short view" );
    detailedAction = new KRadioAction( i18n("Detailed View"), "view_detailed",
                                       KShortcut(), myActionCollection, "detailed view" );

    showHiddenAction = new KToggleAction( i18n("Show Hidden Files"), KShortcut(),
                                          myActionCollection, "show hidden" );
    separateDirsAction = new KToggleAction( i18n("Separate Directories"), KShortcut(),
                                            this,
                                            SLOT(slotSeparateDirs()),
                                            myActionCollection, "separate dirs" );
    KToggleAction *previewAction = new KToggleAction(i18n("Show Preview"),
                                                     "thumbnail", KShortcut(),
                                                     myActionCollection,
                                                     "preview" );
    connect( previewAction, SIGNAL( toggled( bool )),
             SLOT( togglePreview( bool )));


    QString viewGroup = QString::fromLatin1("view");
    shortAction->setExclusiveGroup( viewGroup );
    detailedAction->setExclusiveGroup( viewGroup );

    connect( shortAction, SIGNAL( toggled( bool ) ),
             SLOT( slotSimpleView() ));
    connect( detailedAction, SIGNAL( toggled( bool ) ),
             SLOT( slotDetailedView() ));
    connect( showHiddenAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleHidden( bool ) ));

    new KAction( i18n("Properties..."), KShortcut(ALT+Key_Return), this,
                 SLOT(slotProperties()), myActionCollection, "properties" );
}

void KDirOperator::setupMenu()
{
    setupMenu(AllActions);
}

void KDirOperator::setupMenu(int whichActions)
{
    // first fill the submenus (sort and view)
    sortActionMenu->popupMenu()->clear();
    sortActionMenu->insert( byNameAction );
    sortActionMenu->insert( byDateAction );
    sortActionMenu->insert( bySizeAction );
    sortActionMenu->insert( actionSeparator );
    sortActionMenu->insert( reverseAction );
    sortActionMenu->insert( dirsFirstAction );
    sortActionMenu->insert( caseInsensitiveAction );

    viewActionMenu->popupMenu()->clear();
//     viewActionMenu->insert( shortAction );
//     viewActionMenu->insert( detailedAction );
//     viewActionMenu->insert( actionSeparator );
    viewActionMenu->insert( myActionCollection->action( "short view" ) );
    viewActionMenu->insert( myActionCollection->action( "detailed view" ) );
    viewActionMenu->insert( actionSeparator );
    viewActionMenu->insert( showHiddenAction );
//    viewActionMenu->insert( myActionCollection->action( "single" ));
    viewActionMenu->insert( separateDirsAction );
    // Warning: adjust slotViewActionAdded() and slotViewActionRemoved()
    // when you add/remove actions here!

    // now plug everything into the popupmenu
    actionMenu->popupMenu()->clear();
    if (whichActions & NavActions)
    {
        actionMenu->insert( upAction );
        actionMenu->insert( backAction );
        actionMenu->insert( forwardAction );
        actionMenu->insert( homeAction );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->insert( mkdirAction );
        actionMenu->insert( myActionCollection->action( "delete" ) );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & SortActions)
    {
        actionMenu->insert( sortActionMenu );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & ViewActions)
    {
        actionMenu->insert( viewActionMenu );
        actionMenu->insert( actionSeparator );
    }

    if (whichActions & FileActions)
    {
        actionMenu->insert( myActionCollection->action( "properties" ) );
    }
}

void KDirOperator::updateSortActions()
{
    if ( KFile::isSortByName( mySorting ) )
        byNameAction->setChecked( true );
    else if ( KFile::isSortByDate( mySorting ) )
        byDateAction->setChecked( true );
    else if ( KFile::isSortBySize( mySorting ) )
        bySizeAction->setChecked( true );

    dirsFirstAction->setChecked( KFile::isSortDirsFirst( mySorting ) );
    caseInsensitiveAction->setChecked( KFile::isSortCaseInsensitive(mySorting) );
    caseInsensitiveAction->setEnabled( KFile::isSortByName( mySorting ) );

    if ( m_fileView )
        reverseAction->setChecked( m_fileView->isReversed() );
}

void KDirOperator::updateViewActions()
{
    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );

    separateDirsAction->setChecked( KFile::isSeparateDirs( fv ) &&
                                    separateDirsAction->isEnabled() );

    shortAction->setChecked( KFile::isSimpleView( fv ));
    detailedAction->setChecked( KFile::isDetailView( fv ));
}

void KDirOperator::readConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
        return;
    QString oldGroup = kc->group();
    if ( !group.isEmpty() )
        kc->setGroup( group );

    defaultView = 0;
    int sorting = 0;

    QString viewStyle = kc->readEntry( QString::fromLatin1("View Style"),
                                       QString::fromLatin1("Simple") );
    if ( viewStyle == QString::fromLatin1("Detail") )
        defaultView |= KFile::Detail;
    else
        defaultView |= KFile::Simple;
    if ( kc->readBoolEntry( QString::fromLatin1("Separate Directories"),
                            DefaultMixDirsAndFiles ) )
        defaultView |= KFile::SeparateDirs;
    else {
        if ( kc->readBoolEntry(QString::fromLatin1("Show Preview"), false))
            defaultView |= KFile::PreviewContents;
    }

    if ( kc->readBoolEntry( QString::fromLatin1("Sort case insensitively"),
                            DefaultCaseInsensitive ) )
        sorting |= QDir::IgnoreCase;
    if ( kc->readBoolEntry( QString::fromLatin1("Sort directories first"),
                            DefaultDirsFirst ) )
        sorting |= QDir::DirsFirst;


    QString name = QString::fromLatin1("Name");
    QString sortBy = kc->readEntry( QString::fromLatin1("Sort by"), name );
    if ( sortBy == name )
        sorting |= QDir::Name;
    else if ( sortBy == QString::fromLatin1("Size") )
        sorting |= QDir::Size;
    else if ( sortBy == QString::fromLatin1("Date") )
        sorting |= QDir::Time;

    mySorting = static_cast<QDir::SortSpec>( sorting );
    setSorting( mySorting );


    if ( kc->readBoolEntry( QString::fromLatin1("Show hidden files"),
                            DefaultShowHidden ) ) {
         showHiddenAction->setChecked( true );
         dir->setShowingDotFiles( true );
    }
    if ( kc->readBoolEntry( QString::fromLatin1("Sort reversed"),
                            DefaultSortReversed ) )
        reverseAction->setChecked( true );

    kc->setGroup( oldGroup );
}

void KDirOperator::writeConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
        return;

    const QString oldGroup = kc->group();

    if ( !group.isEmpty() )
        kc->setGroup( group );

    QString sortBy = QString::fromLatin1("Name");
    if ( KFile::isSortBySize( mySorting ) )
        sortBy = QString::fromLatin1("Size");
    else if ( KFile::isSortByDate( mySorting ) )
        sortBy = QString::fromLatin1("Date");
    kc->writeEntry( QString::fromLatin1("Sort by"), sortBy );

    kc->writeEntry( QString::fromLatin1("Sort reversed"),
                    reverseAction->isChecked() );
    kc->writeEntry( QString::fromLatin1("Sort case insensitively"),
                    caseInsensitiveAction->isChecked() );
    kc->writeEntry( QString::fromLatin1("Sort directories first"),
                    dirsFirstAction->isChecked() );

    // don't save the separate dirs or preview when an application specific
    // preview is in use.
    bool appSpecificPreview = false;
    if ( myPreview ) {
        QWidget *preview = const_cast<QWidget*>( myPreview ); // grmbl
        KImageFilePreview *tmp = dynamic_cast<KImageFilePreview*>( preview );
        appSpecificPreview = (tmp == 0L);
    }

    if ( !appSpecificPreview ) {
        if ( separateDirsAction->isEnabled() )
            kc->writeEntry( QString::fromLatin1("Separate Directories"),
                            separateDirsAction->isChecked() );

        KToggleAction *previewAction = static_cast<KToggleAction*>(myActionCollection->action("preview"));
        if ( previewAction->isEnabled() ) {
            bool hasPreview = previewAction->isChecked();
            kc->writeEntry( QString::fromLatin1("Show Preview"), hasPreview );
        }
    }

    kc->writeEntry( QString::fromLatin1("Show hidden files"),
                    showHiddenAction->isChecked() );

    KFile::FileView fv = static_cast<KFile::FileView>( m_viewKind );
    QString style;
    if ( KFile::isDetailView( fv ) )
        style = QString::fromLatin1("Detail");
    else if ( KFile::isSimpleView( fv ) )
        style = QString::fromLatin1("Simple");
    kc->writeEntry( QString::fromLatin1("View Style"), style );

    kc->setGroup( oldGroup );
}


void KDirOperator::resizeEvent( QResizeEvent * )
{
    if (m_fileView)
        m_fileView->widget()->resize( size() );

    if ( progress->parent() == this ) // might be reparented into a statusbar
	progress->move(2, height() - progress->height() -2);
}

void KDirOperator::setOnlyDoubleClickSelectsFiles( bool enable )
{
    d->onlyDoubleClickSelectsFiles = enable;
    if ( m_fileView )
        m_fileView->setOnlyDoubleClickSelectsFiles( enable );
}

bool KDirOperator::onlyDoubleClickSelectsFiles() const
{
    return d->onlyDoubleClickSelectsFiles;
}

void KDirOperator::slotStarted()
{
    progress->setProgress( 0 );
    // delay showing the progressbar for one second
    d->progressDelayTimer->start( 1000, true );
}

void KDirOperator::slotShowProgress()
{
    progress->raise();
    progress->show();
    QApplication::flushX();
}

void KDirOperator::slotProgress( int percent )
{
    progress->setProgress( percent );
    // we have to redraw this in as fast as possible
    if ( progress->isVisible() )
	QApplication::flushX();
}


void KDirOperator::slotIOFinished()
{
    d->progressDelayTimer->stop();
    slotProgress( 100 );
    progress->hide();
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

void KDirOperator::slotCanceled()
{
    emit finishedLoading();
    resetCursor();

    if ( m_fileView )
        m_fileView->listingCompleted();
}

KProgress * KDirOperator::progressBar() const
{
    return progress;
}

void KDirOperator::clearHistory()
{
    backStack.clear();
    backAction->setEnabled( false );
    forwardStack.clear();
    forwardAction->setEnabled( false );
}

void KDirOperator::slotViewActionAdded( KAction *action )
{
    if ( viewActionMenu->popupMenu()->count() == 5 ) // need to add a separator
	viewActionMenu->insert( actionSeparator );

    viewActionMenu->insert( action );
}

void KDirOperator::slotViewActionRemoved( KAction *action )
{
    viewActionMenu->remove( action );

    if ( viewActionMenu->popupMenu()->count() == 6 ) // remove the separator
	viewActionMenu->remove( actionSeparator );
}

void KDirOperator::slotViewSortingChanged( QDir::SortSpec sort )
{
    mySorting = sort;
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

void KDirOperator::slotProperties()
{
    if ( m_fileView ) {
        const KFileItemList *list = m_fileView->selectedItems();
        if ( !list->isEmpty() )
            (void) new KPropertiesDialog( *list, this, "props dlg", true);
    }
}

void KDirOperator::slotClearView()
{
    if ( m_fileView )
        m_fileView->clearView();
}

// ### temporary code
#include <dirent.h>
bool KDirOperator::isReadable( const KURL& url )
{
    if ( !url.isLocalFile() )
	return true; // what else can we say?

    struct stat buf;
    QString ts = url.path(+1);
    bool readable = ( ::stat( QFile::encodeName( ts ), &buf) == 0 );
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
        setView( (KFile::FileView) (m_viewKind & ~(KFile::PreviewContents|KFile::PreviewInfo)) );
}

void KDirOperator::slotRefreshItems( const KFileItemList& items )
{
    if ( !m_fileView )
        return;

    KFileItemListIterator it( items );
    for ( ; it.current(); ++it )
        m_fileView->updateView( it.current() );
}


void KDirOperator::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdiroperator.moc"
