/* This file is part of the KDE libraries
    Copyright (C) 1999,2000 Stephan Kulow <coolo@kde.org>
                  1999,2000 Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <qtimer.h>
#include <qvbox.h>

#include <kaction.h>
#include <kapp.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdialogbase.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kprogress.h>
#include <kstdaction.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/previewjob.h>
#include <kpropsdlg.h>
#include <kservicetypefactory.h>

#include "config-kfile.h"
#include "kcombiview.h"
#include "kdiroperator.h"
#include "kfiledetailview.h"
#include "kfileiconview.h"
#include "kfilepreview.h"
#include "kfilereader.h"
#include "kfileview.h"
#include "kfileviewitem.h"
#include "kimagefilepreview.h"


template class QStack<KURL>;
template class QDict<KFileViewItem>;

KURL *KDirOperator::lastDirectory = 0; // to set the start path

class KDirOperator::KDirOperatorPrivate
{
public:
    KDirOperatorPrivate() {
        onlyDoubleClickSelectsFiles = false;
        progressDelayTimer = 0L;
	viewActionSeparator = new KActionSeparator;
	dirHighlighting = false;
    }

    ~KDirOperatorPrivate() {
        delete progressDelayTimer;
	delete viewActionSeparator;
    }

    bool dirHighlighting;
    QString lastURL; // used for highlighting a directory on cdUp
    bool onlyDoubleClickSelectsFiles;
    QTimer *progressDelayTimer;
    KActionSeparator *viewActionSeparator;
};

KDirOperator::KDirOperator(const KURL& url,
                           QWidget *parent, const char* _name)
    : QWidget(parent, _name), fileView(0), oldView(0), progress(0)
{
    myPreview = 0L;
    myMode = KFile::File;
    viewKind = KFile::Simple;
    mySorting = static_cast<QDir::SortSpec>(QDir::Name | QDir::DirsFirst);
    d = new KDirOperatorPrivate;

    if (url.isEmpty()) { // no dir specified -> current dir
        QString strPath = QDir::currentDirPath();
        strPath.append('/');
        lastDirectory = new KURL;
        lastDirectory->setProtocol(QString::fromLatin1("file"));
        lastDirectory->setPath(strPath);
    }
    else {
        lastDirectory = new KURL(url);
        if ( lastDirectory->protocol().isEmpty() )
            lastDirectory->setProtocol(QString::fromLatin1("file"));
    }

    dir = new KFileReader(*lastDirectory);
    dir->setAutoUpdate( true );

    connect(dir, SIGNAL(started( const QString& )), SLOT(slotStarted()));
    connect(dir, SIGNAL(newItems(const KFileItemList &)),
            SLOT(insertNewFiles(const KFileItemList &)));
    connect(dir, SIGNAL(completed()), SLOT(slotIOFinished()));
    connect(dir, SIGNAL(canceled()), SLOT(resetCursor()));
    connect(dir, SIGNAL(deleteItem(KFileItem *)),
            SLOT(itemDeleted(KFileItem *)));
    connect(dir, SIGNAL(redirection( const KURL& )),
	    SLOT( slotRedirected( const KURL& )));

    connect(&myCompletion, SIGNAL(match(const QString&)),
            SLOT(slotCompletionMatch(const QString&)));

    progress = new KProgress(this, "progress");
    progress->adjustSize();
    progress->setRange(0, 100);
    progress->move(2, height() - progress->height() -2);

    d->progressDelayTimer = new QTimer( this, "progress delay timer" );
    connect( d->progressDelayTimer, SIGNAL( timeout() ),
	     SLOT( slotShowProgress() ));

    finished = false;
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
    delete fileView;
    delete myPreview;
    delete dir;
    delete d;
}


void KDirOperator::setSorting( QDir::SortSpec spec )
{
    if ( fileView )
        fileView->setSorting( spec );
    mySorting = spec;
    updateSortActions();
}

void KDirOperator::readNextMimeType()
{
    if (pendingMimeTypes.isEmpty()) {
        return;
    }

    KFileViewItem *item = pendingMimeTypes.first();
    int oldSerial = item->pixmap().serialNumber();
    (void) item->determineMimeType();

    if ( item->pixmap().serialNumber() != oldSerial )
        fileView->updateView(item);
    pendingMimeTypes.removeFirst();
    QTimer::singleShot(0, this, SLOT(readNextMimeType()));
}

void KDirOperator::resetCursor()
{
    if (!finished)
        QApplication::restoreOverrideCursor();
    finished = false;
    progress->hide();
}

void KDirOperator::activatedMenu( const KFileViewItem * )
{
    // ### this should be invoked from a KFileView signal
    // when the view changed the sorting.
    slotViewSortingChanged();

    updateSelectionDependentActions();

    actionMenu->popup( QCursor::pos() );
}

void KDirOperator::updateSelectionDependentActions()
{
    bool hasSelection = fileView && fileView->selectedItems() &&
                        !fileView->selectedItems()->isEmpty();
    myActionCollection->action( "delete" )->setEnabled( hasSelection );
    myActionCollection->action( "properties")->setEnabled( hasSelection );
}

void KDirOperator::setPreviewWidget(const QWidget *w)
{
    if(w != 0L)
        viewKind = (viewKind | KFile::PreviewContents) & ~KFile::SeparateDirs;
    else
        viewKind = (viewKind & ~KFile::PreviewContents);

    delete myPreview;
    myPreview = w;

    KToggleAction *preview = static_cast<KToggleAction*>(myActionCollection->action("preview"));
    preview->setEnabled( w != 0L );
    preview->setChecked( w != 0L );
    setView( static_cast<KFile::FileView>(viewKind) );
}

int KDirOperator::numDirs() const
{
    return fileView ? fileView->numDirs() : 0;
}

int KDirOperator::numFiles() const
{
    return fileView ? fileView->numFiles() : 0;
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (viewKind & ~KFile::Simple) | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( (viewKind & ~KFile::Detail) | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    dir->setShowingDotFiles( show );
    rereadDir();
}

void KDirOperator::slotSingleView()
{
    KFile::FileView view = static_cast<KFile::FileView>(
        viewKind & ~(KFile::PreviewContents |
                     KFile::PreviewInfo |
                     KFile::SeparateDirs ));

    setView( view );
}

void KDirOperator::slotSeparateDirs()
{
    KFile::FileView view = static_cast<KFile::FileView>(
        (viewKind & ~(KFile::PreviewContents |
                      KFile::PreviewInfo)
         | KFile::SeparateDirs ));

    setView( view );
}

void KDirOperator::slotDefaultPreview()
{
    viewKind = (viewKind | KFile::PreviewContents) & ~KFile::SeparateDirs;
    if ( !myPreview ) {
        myPreview = new KImageFilePreview( this );
        (static_cast<KRadioAction*>( myActionCollection->action("preview") ))->setChecked(true);
    }

    setView( static_cast<KFile::FileView>(viewKind) );
}

void KDirOperator::slotSortByName()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Name ));
    mySorting = fileView->sorting();
}

void KDirOperator::slotSortBySize()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Size ));
    mySorting = fileView->sorting();
}

void KDirOperator::slotSortByDate()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Time ));
    mySorting = fileView->sorting();
}

void KDirOperator::slotSortReversed()
{
    if ( fileView )
        fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    QDir::SortSpec sorting = fileView->sorting();
    if ( !KFile::isSortDirsFirst( sorting ) )
        fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::DirsFirst ));
    else
        fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::DirsFirst));
    mySorting = fileView->sorting();
}

void KDirOperator::slotToggleIgnoreCase()
{
    QDir::SortSpec sorting = fileView->sorting();
    if ( !KFile::isSortCaseInsensitive( sorting ) )
        fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::IgnoreCase ));
    else
        fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::IgnoreCase));
    mySorting = fileView->sorting();
}

void KDirOperator::mkdir()
{
    // Modal widget asking the user the name of a new directory
    //
    KDialogBase *lMakeDir;
    QLabel *label;
    KLineEdit *ed;
    QVBox *vbox;

    // Create widgets, and display using geometry management
    //
    lMakeDir = new KDialogBase( viewWidget(),
				"MakeDir Dialog", true, i18n("New Directory"),
				KDialogBase::Ok | KDialogBase::Cancel );
    vbox = new QVBox( lMakeDir );
    vbox->setSpacing( KDialog::spacingHint() );
    lMakeDir->setMainWidget( vbox );
    label = new QLabel( vbox );
    label->setAlignment( AlignLeft | AlignVCenter );
    label->setText(i18n("Create new directory in: ") +
		   QString::fromLatin1( "\n" ) + /* don't break i18n now*/
		   url().prettyURL() );
    ed= new KLineEdit( vbox );
    ed->setText( i18n("New Directory") );
    ed->selectAll();
    connect(ed, SIGNAL(returnPressed()), lMakeDir, SLOT(accept()) );

    connect( lMakeDir->actionButton( KDialogBase::Ok ), SIGNAL(clicked()),
	     lMakeDir, SLOT(accept()) );
    connect( lMakeDir->actionButton( KDialogBase::Cancel ), SIGNAL(clicked()),
	     lMakeDir, SLOT(reject()) );


    // If the users presses enter (not escape) then create the dir
    // and insert it into the ListBox
    lMakeDir->setMinimumSize( 300, 120); // default size
    ed->grabKeyboard();
    if ( lMakeDir->exec() == QDialog::Accepted && !ed->text().isEmpty() )
        mkdir( ed->text(), true );

    delete lMakeDir;
}

bool KDirOperator::mkdir( const QString& directory, bool enterDirectory )
{
    bool writeOk = false;
    KURL url( dir->url());
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

KIO::DeleteJob * KDirOperator::del( const KFileViewItemList& items,
                                    bool ask, bool showProgress )
{
    if ( items.isEmpty() ) {
        KMessageBox::information( this,
                                  i18n("You didn't select a file to delete."),
                                  i18n("Nothing to delete") );
        return 0L;
    }

    KURL::List urls;
    QStringList files;
    KFileViewItemListIterator it( items );

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
            ret = KMessageBox::warningContinueCancel( viewWidget(),
                i18n( "<qt>Do you really want to delete\n <b>'%1'</b>?</qt>" )
                .arg( files.first() ),
                                                      i18n("Delete file"),
                                                      i18n("Delete") );
        }
        else
            ret = KMessageBox::warningContinueCancelList( viewWidget(),
                i18n("Do you really want to delete these %1 items?"),
                                                    files,
                                                    i18n("Delete files"),
                                                    i18n("Delete") );
        doIt = (ret == KMessageBox::Continue);
    }

    if ( doIt )
        return KIO::del( urls, false, showProgress );

    return 0L;
}

void KDirOperator::deleteSelected()
{
    if ( !fileView )
        return;

    const KFileViewItemList *list = fileView->selectedItems();
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
        text.insert(0, dir->url());

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
        KFileViewItem i("", u.path());
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

        if (!finished)
            QApplication::restoreOverrideCursor();
        finished = false;

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

    if (finished && newurl == dir->url()) // already set
        return;

    pendingMimeTypes.clear();

    if (clearforward) {
        // autodelete should remove this one
	backStack.push(new KURL(dir->url()));
        forwardStack.clear();
    }
    /* // FIXME: (pfeiffer) we should have a flag "onlyLocal", I guess
    KURL testURL(pathstr);
    if ( !testURL.isLocalFile() ) {
        KMessageBox::sorry(0,
                           i18n("The specified directory is not a "
                                "local directory\n"
                                "But the application accepts just "
                                "local files."));
        return;
    }
    */

    KURL backup(dir->url());
    dir->setURL(newurl);

    bool ok = false;
    if (!dir->isReadable()) {
        // maybe newurl is a file? check its parent directory
        newurl.cd(QString::fromLatin1(".."));
        dir->setURL(newurl);

        if ( !dir->isReadable() ) {
            KMessageBox::error(viewWidget(),
                               i18n("The specified directory does not exist\n"
                                    "or was not readable."));
            dir->setURL(backup);
            backStack.pop();
        }
        else
            // don't reread when the url stayed the same
            ok = (dir->url().url(-1) != newurl.url(-1));
    }
    else
        ok = true;

    if ( ok ) {
	d->lastURL = backup.url(-1);
        myCompletion.clear();
        myDirCompletion.clear();
        emit urlEntered(dir->url());
        pathChanged();
    }


    // enable/disable actions
    forwardAction->setEnabled( !forwardStack.isEmpty() );
    backAction->setEnabled( !backStack.isEmpty() );
    upAction->setEnabled( !isRoot() );
}

void KDirOperator::rereadDir()
{
    dir->setURLDirty( true );
    pathChanged();
}

// Protected
void KDirOperator::pathChanged()
{
    if (!fileView)
        return;

    pendingMimeTypes.clear();
    fileView->clear();
    myCompletion.clear();
    myDirCompletion.clear();

    // it may be, that we weren't ready at this time
    if (!finished)
        QApplication::restoreOverrideCursor();

    finished = false;

    // when KIO::Job emits finished, the slot will restore the cursor
    QApplication::setOverrideCursor( waitCursor );

    // lastDirectory is used to set the start path next time
    // we select a file
    *lastDirectory = dir->url();

    if (!dir->isReadable()) {
        KMessageBox::error(viewWidget(),
                           i18n("The specified directory does not exist "
                                "or was not readable."));
        if (backStack.isEmpty())
            home();
        else
            back();
    }

    dir->listDirectory();
}

void KDirOperator::slotRedirected( const KURL& newURL )
{
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

    forwardStack.push( new KURL(dir->url()) );

    KURL *s = backStack.pop();

    setURL(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( forwardStack.isEmpty() )
        return;

    backStack.push(new KURL(dir->url()));

    KURL *s = forwardStack.pop();
    setURL(*s, false);
    delete s;
}

KURL KDirOperator::url() const
{
    return dir->url();
}

void KDirOperator::cdUp()
{
    KURL tmp( dir->url() );
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
    KRadioAction *previewAction = static_cast<KRadioAction*>( myActionCollection->action( "preview" ));

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
                    if ( r.match( mime ) != -1 ) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool KDirOperator::isRoot() const
{
    return dir->isRoot();
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = KFile::isSeparateDirs( view );
    bool preview=( (view & KFile::PreviewInfo) == KFile::PreviewInfo ||
                   (view & KFile::PreviewContents) == KFile::PreviewContents );

    if (view == KFile::Default) {
        if ( KFile::isDetailView( (KFile::FileView) defaultView ) )
            view = KFile::Detail;
        else
            view = KFile::Simple;

        separateDirs = KFile::isSeparateDirs( (KFile::FileView) defaultView );
        preview = ( (defaultView & KFile::PreviewInfo) == KFile::PreviewInfo ||
                    (defaultView & KFile::PreviewContents) ==
                    KFile::PreviewContents )
                  && myActionCollection->action("preview")->isEnabled();

        if ( preview ) { // instantiates KImageFilePreview and calls setView()
            viewKind = view;
            slotDefaultPreview();
            return;
        }
        else if ( !separateDirs )
            (static_cast<KRadioAction*>( myActionCollection->action("single") ))->setChecked(true);

    }

    // if we don't have any files, we can't separate dirs from files :)
    if ( (mode() & KFile::File) == 0 && mode() & KFile::Files == 0 ) {
        separateDirs = false;
 	separateDirsAction->setEnabled( false );
    }

    viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);

    KFileView *new_view = 0L;

    if (separateDirs) {
        KCombiView *combi = new KCombiView( this, "combi view" );
        combi->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);
        new_view = combi;

        if ( (view & KFile::Simple) == KFile::Simple ) {
            KFileIconView *view = new KFileIconView( combi, "simple view" );
            view->setViewName( i18n("Short View") );
            combi->setRight( view );
        }
        else
            combi->setRight(new KFileDetailView( combi, "detail view" ));

    } else {
        if ( (view & KFile::Simple) == KFile::Simple && !preview ) {
            new_view = new KFileIconView( this, "simple view");
            new_view->setViewName( i18n("Short View") );
        }
        else if ( (view & KFile::Detail) == KFile::Detail && !preview ) {
            new_view = new KFileDetailView( this,"detail view" );
        }

        else { // preview
	    KFileView *v; // will get reparented by KFilePreview
	    if ( (view & KFile::Simple ) == KFile::Simple )
	        v = new KFileIconView( 0L, "simple view" );
	    else
	        v = new KFileDetailView( 0L, "detail view" );
	
            KFilePreview *tmp = new KFilePreview( v, this, "preview" );
           tmp->setOnlyDoubleClickSelectsFiles(d->onlyDoubleClickSelectsFiles);

           // we keep the preview-_widget_ around, but not the KFilePreview.
           // So in order to reuse the widget, we have to prevent it from
           // being deleted by ~KFilePreview().
           if ( myPreview && myPreview->parent() ) {
               myPreview->parent()->removeChild( (QWidget*) myPreview );
           }

            tmp->setPreviewWidget(myPreview, url());
            new_view=tmp;
        }
    }

    setView( new_view );
}

void KDirOperator::deleteOldView()
{
    if ( oldView ) {
        oldView->widget()->hide();
        delete oldView;
        oldView = 0;
    }
}


void KDirOperator::connectView(KFileView *view)
{
    // TODO: do a real timer and restart it after that
    pendingMimeTypes.clear();

    if (fileView) {
        QApplication::setOverrideCursor( waitCursor );
        finished = false;
        fileView->setOperator(0);
        // it's _very_ unlikly that oldView still has a value
        oldView = fileView;
        // the timer didn't work (KFileItems being deleted, then the view
        // being deleted, the view deleting the view items, which tried to
        // access the (dead) KFileItems. What was it good for anyway?
        // QTimer::singleShot(0, this, SLOT(deleteOldView()));
        deleteOldView();
    }

    fileView = view;
    fileView->setOperator(this);
    KActionCollection *coll = fileView->actionCollection();
    if ( !coll->isEmpty() ) {
 	viewActionMenu->insert( d->viewActionSeparator );
 	for ( uint i = 0; i < coll->count(); i++ )
 	    viewActionMenu->insert( coll->action( i ));
    }

    connect( coll, SIGNAL( inserted( KAction * )),
 	     SLOT( slotViewActionAdded( KAction * )));
    connect( coll, SIGNAL( removed( KAction * )),
 	     SLOT( slotViewActionRemoved( KAction * )));

    if ( reverseAction->isChecked() != fileView->isReversed() )
        fileView->sortReversed();

    if ( dirOnlyMode() )
         fileView->setViewMode(KFileView::Directories);
    else {
        fileView->setViewMode(KFileView::All);
    }
    if ( myMode & KFile::Files )
        fileView->setSelectionMode( KFile::Extended );
    else
        fileView->setSelectionMode( KFile::Single );

    dir->listDirectory();

    updateViewActions();
    fileView->widget()->show();
    fileView->widget()->resize(size());
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
    setView( static_cast<KFile::FileView>(viewKind) );
}

void KDirOperator::setView(KFileView *view)
{
    if ( view == fileView ) {
        return;
    }

    setFocusProxy(view->widget());
    view->setSorting( mySorting );
    view->setOnlyDoubleClickSelectsFiles( d->onlyDoubleClickSelectsFiles );
    connectView(view); // also deletes the old view

    emit viewChanged( view );
}

void KDirOperator::setFileReader( KFileReader *reader )
{
    delete dir;
    dir = reader;
}

void KDirOperator::insertNewFiles(const KFileItemList &newone)
{
    if (newone.isEmpty())
        return;

    myCompleteListDirty = true;
    fileView->addItemList( newone );
    emit updateInformation(fileView->numDirs(), fileView->numFiles());

    KFileItem *item;
    KFileItemListIterator it( newone );
    while ( (item = it.current()) ) {
	// highlight the dir we come from, if possible
	if ( d->dirHighlighting && item->isDir() &&
	     item->url().url(-1) == d->lastURL ) {
	    fileView->setCurrentItem( QString::null, (KFileViewItem*) item );
	    fileView->ensureItemVisible( (KFileViewItem*) item );
	}
	
	if ( !item->isMimeTypeKnown() )
	    pendingMimeTypes.append(static_cast<KFileViewItem*>(item));
	++it;
    }

    if ( !pendingMimeTypes.isEmpty() )
        QTimer::singleShot(0, this, SLOT(readNextMimeType()));
    QTimer::singleShot(200, this, SLOT(resetCursor()));
}

void KDirOperator::selectDir(const KFileViewItem *item)
{
    KURL tmp( dir->url() );
    tmp.cd(item->name());
    setURL(tmp, true);
}

void KDirOperator::itemDeleted(KFileItem *item)
{
    fileView->removeItem( static_cast<KFileViewItem *>( item ));
    emit updateInformation(fileView->numDirs(), fileView->numFiles());
}

void KDirOperator::selectFile(const KFileViewItem *item)
{
    if (!finished)
        QApplication::restoreOverrideCursor();
    finished = false;

    emit fileSelected( item );
}


void KDirOperator::filterChanged()
{
    pendingMimeTypes.clear();
    fileView->clear();
    myCompletion.clear();
    myDirCompletion.clear();
    dir->listDirectory();
}

void KDirOperator::setCurrentItem( const QString& filename )
{
    const KFileViewItem *item = 0L;

    if ( !filename.isNull() )
        item = static_cast<KFileViewItem *>(dir->findByName( filename ));

    fileView->clearSelection();
    if ( item ) {
        fileView->setCurrentItem( QString::null, item );
	fileView->ensureItemVisible( item );
    }
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        fileView->clearSelection();
        return QString::null;
    }

    prepareCompletionObjects();
    return myCompletion.makeCompletion( string );
}

QString KDirOperator::makeDirCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        fileView->clearSelection();
        return QString::null;
    }

    prepareCompletionObjects();
    return myDirCompletion.makeCompletion( string );
}

void KDirOperator::prepareCompletionObjects()
{
    if ( !fileView )
	return;

    if ( myCompleteListDirty ) { // create the list of all possible completions
        KFileViewItemListIterator it( *(fileView->items()) );
        for( ; it.current(); ++it ) {
            KFileViewItem *item = it.current();

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
    actionMenu = new KActionMenu( i18n("Menu"), this, "popupMenu" );

    upAction = KStdAction::up( this, SLOT( cdUp() ), this, "up" );
    upAction->setText( i18n("Parent directory") );
    backAction = KStdAction::back( this, SLOT( back() ), this, "back" );
    forwardAction = KStdAction::forward(this, SLOT(forward()), this,"forward");
    homeAction = KStdAction::home( this, SLOT( home() ), this, "home" );
    homeAction->setText(i18n("Home directory"));
    reloadAction =KStdAction::redisplay(this,SLOT(rereadDir()),this, "reload");
    actionSeparator = new KActionSeparator( this, "separator" );
    mkdirAction = new KAction( i18n("New Directory..."), 0,
                                 this, SLOT( mkdir() ), this, "mkdir");
    KAction * deleteAction = new KAction( i18n( "Delete" ), "editdelete",
                                          Key_Delete, this,
                                          SLOT( deleteSelected() ),
                                          this, "delete" );
    mkdirAction->setIcon( QString::fromLatin1("folder_new") );
    reloadAction->setText( i18n("Reload") );


    // the sort menu actions
    sortActionMenu = new KActionMenu( i18n("Sorting"), this, "sorting menu");
    byNameAction = new KRadioAction( i18n("By Name"), 0,
                                     this, SLOT( slotSortByName() ),
                                     this, "by name" );
    byDateAction = new KRadioAction( i18n("By Date"), 0,
                                     this, SLOT( slotSortByDate() ),
                                     this, "by date" );
    bySizeAction = new KRadioAction( i18n("By Size"), 0,
                                     this, SLOT( slotSortBySize() ),
                                     this, "by size" );
    reverseAction = new KToggleAction( i18n("Reverse"), 0,
                                       this, SLOT( slotSortReversed() ),
                                       this, "reversed" );

    QString sortGroup = QString::fromLatin1("sort");
    byNameAction->setExclusiveGroup( sortGroup );
    byDateAction->setExclusiveGroup( sortGroup );
    bySizeAction->setExclusiveGroup( sortGroup );


    dirsFirstAction = new KToggleAction( i18n("Directories first"), 0,
                                         this, "dirs first");
    caseInsensitiveAction = new KToggleAction(i18n("Case insensitive"), 0,
                                              this, "case insensitive" );

    connect( dirsFirstAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleDirsFirst() ));
    connect( caseInsensitiveAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleIgnoreCase() ));



    // the view menu actions
    viewActionMenu = new KActionMenu( i18n("View"), this, "view menu" );
    shortAction = new KRadioAction( i18n("Short View"), "view_multicolumn",
                                    0, this, "short view" );
    detailedAction = new KRadioAction( i18n("Detailed View"), "view_detailed",
                                       0, this, "detailed view" );

    showHiddenAction = new KToggleAction( i18n("Show Hidden Files"), 0,
                                          this, "show hidden" );
    KRadioAction *singleAction = new KRadioAction( i18n("Single view"), 0,
                                                   this,
                                                   SLOT( slotSingleView() ),
                                                   this, "single" );
    separateDirsAction = new KRadioAction( i18n("Separate Directories"), 0,
                                            this,
                                            SLOT(slotSeparateDirs()),
                                            this, "separate dirs" );
    KRadioAction *previewAction = new KRadioAction(i18n("Preview"), 0,
                                                   this,
                                                   SLOT(slotDefaultPreview()),
                                                   this, "preview" );

    QString combiView = QString::fromLatin1("combiview");
    singleAction->setExclusiveGroup( combiView );
    separateDirsAction->setExclusiveGroup( combiView );
    previewAction->setExclusiveGroup( combiView );

    QString viewGroup = QString::fromLatin1("view");
    shortAction->setExclusiveGroup( viewGroup );
    detailedAction->setExclusiveGroup( viewGroup );

    connect( shortAction, SIGNAL( toggled( bool ) ),
             SLOT( slotSimpleView() ));
    connect( detailedAction, SIGNAL( toggled( bool ) ),
             SLOT( slotDetailedView() ));
    connect( showHiddenAction, SIGNAL( toggled( bool ) ),
             SLOT( slotToggleHidden( bool ) ));

    KAction *props = new KAction( i18n("Properties..."), ALT+Key_Return, this,
                                  SLOT(slotProperties()), this, "properties" );

    // insert them into the actionCollection
    myActionCollection = new KActionCollection( this, "action collection" );
    myActionCollection->insert( actionMenu );
    myActionCollection->insert( backAction );
    myActionCollection->insert( forwardAction );
    myActionCollection->insert( homeAction );
    myActionCollection->insert( upAction );
    myActionCollection->insert( reloadAction );
    myActionCollection->insert( actionSeparator );
    myActionCollection->insert( mkdirAction );
    myActionCollection->insert( deleteAction );
    myActionCollection->insert( sortActionMenu );
    myActionCollection->insert( byNameAction );
    myActionCollection->insert( byDateAction );
    myActionCollection->insert( bySizeAction );
    myActionCollection->insert( reverseAction );
    myActionCollection->insert( dirsFirstAction );
    myActionCollection->insert( caseInsensitiveAction );
    myActionCollection->insert( viewActionMenu );
    myActionCollection->insert( shortAction );
    myActionCollection->insert( detailedAction );
    myActionCollection->insert( showHiddenAction );
    myActionCollection->insert( singleAction );
    myActionCollection->insert( previewAction );
    myActionCollection->insert( separateDirsAction );
    myActionCollection->insert( props );
}


void KDirOperator::setupMenu()
{
    // first fill the submenus (sort and view)
    sortActionMenu->insert( byNameAction );
    sortActionMenu->insert( byDateAction );
    sortActionMenu->insert( bySizeAction );
    sortActionMenu->insert( reverseAction );
    sortActionMenu->insert( actionSeparator );
    sortActionMenu->insert( dirsFirstAction );
    sortActionMenu->insert( caseInsensitiveAction );

    viewActionMenu->insert( shortAction );
    viewActionMenu->insert( detailedAction );
    viewActionMenu->insert( actionSeparator );
    viewActionMenu->insert( showHiddenAction );
    viewActionMenu->insert( actionSeparator );
    viewActionMenu->insert( myActionCollection->action( "single" ));
    viewActionMenu->insert( separateDirsAction );
    viewActionMenu->insert( myActionCollection->action( "preview" ));
    // Warning: adjust slotViewActionAdded() and slotViewActionRemoved()
    // when you add/remove actions here!


    // now plug everything into the popupmenu
    actionMenu->insert( upAction );
    actionMenu->insert( backAction );
    actionMenu->insert( forwardAction );
    actionMenu->insert( homeAction );
    actionMenu->insert( actionSeparator );

    actionMenu->insert( mkdirAction );
    actionMenu->insert( myActionCollection->action( "delete" ) );
    actionMenu->insert( actionSeparator );

    actionMenu->insert( sortActionMenu );
    actionMenu->insert( actionSeparator );
    actionMenu->insert( viewActionMenu );
    actionMenu->insert( actionSeparator );
    actionMenu->insert( myActionCollection->action( "properties" ) );
}


void KDirOperator::updateSortActions()
{
    if ( KFile::isSortByName( mySorting ) )
        byNameAction->setChecked( true );
    else if ( KFile::isSortByDate( mySorting ) )
        byDateAction->setChecked( true );
    else if ( KFile::isSortBySize( mySorting ) )
        bySizeAction->setChecked( true );

    dirsFirstAction->setChecked( KFile::isSortDirsFirst( mySorting ));
    caseInsensitiveAction->setChecked(KFile::isSortCaseInsensitive(mySorting));

    if ( fileView )
	reverseAction->setChecked( fileView->isReversed() );
}

void KDirOperator::updateViewActions()
{
    KFile::FileView fv = static_cast<KFile::FileView>( viewKind );

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
        // we want KFileDialog to have a preview by default. Other apps
        // using KDirOperator shouldn't get it by default, but an extra
        // method for that would be overkill. So we have this nice lil hacklet.
        KFileDialog *kfd = dynamic_cast<KFileDialog*>( topLevelWidget() );

        if ( kc->readBoolEntry(QString::fromLatin1("Show Preview"), kfd != 0L))
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

void KDirOperator::saveConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
        return;

    const QString oldGroup = kc->group();

    if ( !group.isEmpty() )
        kc->setGroup( group );

    // ### remove this when KFileView emits a sortingChanged() signal.
    slotViewSortingChanged();

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

        KRadioAction *previewAction = static_cast<KRadioAction*>(myActionCollection->action("preview"));
        if ( previewAction->isEnabled() ) {
            bool hasPreview = previewAction->isChecked();
            kc->writeEntry( QString::fromLatin1("Show Preview"), hasPreview );
        }
    }

    kc->writeEntry( QString::fromLatin1("Show hidden files"),
                    showHiddenAction->isChecked() );

    KFile::FileView fv = static_cast<KFile::FileView>( viewKind );
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
    if (fileView)
        fileView->widget()->resize( size() );

    if ( progress->parent() == this ) // might be reparented into a statusbar
	progress->move(2, height() - progress->height() -2);
}

void KDirOperator::setOnlyDoubleClickSelectsFiles( bool enable )
{
    d->onlyDoubleClickSelectsFiles = enable;
}

bool KDirOperator::onlyDoubleClickSelectsFiles() const
{
    return d->onlyDoubleClickSelectsFiles;
}

void KDirOperator::slotStarted()
{
    if ( !dir->job() )
	return;

    dir->job()->disconnect( this );

    progress->setValue( 0 );
    // delay showing the progressbar for one second
    d->progressDelayTimer->start( 1000, true );

    connect( dir->job(), SIGNAL( percent( KIO::Job *, unsigned long )),
	     this, SLOT( slotProgress( KIO::Job *, unsigned long ) ));
}

void KDirOperator::slotShowProgress()
{
    progress->raise();
    progress->show();
    QApplication::flushX();
}

void KDirOperator::slotProgress( KIO::Job * job, unsigned long percent )
{
    if ( dir->job() != job )
	return;

    progress->setValue( percent );
    // we have to redraw this in as fast as possible
    if ( progress->isVisible() )
	QApplication::flushX();
}


void KDirOperator::slotIOFinished()
{
    d->progressDelayTimer->stop();
    slotProgress( dir->job(), 100 );
    progress->hide();
    emit finishedLoading();
    resetCursor();
}

KProgress * KDirOperator::progressBar() const {
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
    if ( viewActionMenu->popupMenu()->count() == 8 ) // need to add a separator
	viewActionMenu->insert( d->viewActionSeparator );
	
    viewActionMenu->remove( action );
}

void KDirOperator::slotViewActionRemoved( KAction *action )
{
    viewActionMenu->remove( action );

    if ( viewActionMenu->popupMenu()->count() == 9 ) // remove the separator
	viewActionMenu->remove( d->viewActionSeparator );
}

void KDirOperator::slotViewSortingChanged()
{
    mySorting = fileView->sorting();
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
    if ( fileView ) {
        KFileItemList list; // now this sucks :(
        KFileViewItemListIterator it( *fileView->selectedItems() );
        for ( ; it.current(); ++it )
            list.append( it.current() );

        if ( !list.isEmpty() )
            (void) new KPropertiesDialog( list, this, "props dlg", true );
    }
}

#include "kdiroperator.moc"
