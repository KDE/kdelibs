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

#include <qdir.h>
#include <qapplication.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kaction.h>
#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kstdaction.h>

#include "config-kfile.h"
#include "kcombiview.h"
#include "kdiroperator.h"
#include "kfiledetailview.h"
#include "kfileiconview.h"
#include "kfilepreview.h"
#include "kfilereader.h"
#include "kfileview.h"
#include "kfileviewitem.h"


template class QStack<KURL>;
template class QDict<KFileViewItem>;

QString *KDirOperator::lastDirectory = 0; // to set the start path

KDirOperator::KDirOperator(const QString& dirName,
			   QWidget *parent, const char* _name)
    : QWidget(parent, _name), fileView(0), oldView(0), progress(0)
{
    myPreview = 0L;
    myMode=KFile::File;
    viewKind = KFile::Simple;
    mySorting = static_cast<QDir::SortSpec>(QDir::Name | QDir::DirsFirst);

    if (dirName.isEmpty()) // no dir specified -> current dir
	lastDirectory = new QString(QDir::currentDirPath());
    else
	lastDirectory = new QString(dirName);

    dir = new KFileReader(*lastDirectory);
    dir->setAutoUpdate( true );

    connect(dir, SIGNAL(contents(const KFileViewItemList &, bool)),
	    SLOT(insertNewFiles(const KFileViewItemList &, bool)));
    connect(dir, SIGNAL(itemsDeleted(const KFileViewItemList &)),
	    SLOT(itemsDeleted(const KFileViewItemList &)));
    connect(dir, SIGNAL(error(int, const QString& )),
	    SLOT(slotKIOError(int, const QString& )));
    connect(dir, SIGNAL(filterChanged()),
	    SLOT(filterChanged()));

    connect(&myCompletion, SIGNAL(match(const QString&)),
	    SLOT(slotCompletionMatch(const QString&)));

    finished = true;
    myCompleteListDirty = false;

    backStack.setAutoDelete( true );
    forwardStack.setAutoDelete( true );

    KConfig *c = KGlobal::config();
    KConfigGroupSaver sa(c, ConfigGroup);
    bool showHidden = c->readBoolEntry(QString::fromLatin1("ShowHidden"),
				       DefaultShowHidden);

    // action stuff
    setupActions();
    setupMenu();

    setShowHiddenFiles( showHidden );
    setFocusPolicy(QWidget::WheelFocus);
}

KDirOperator::~KDirOperator()
{
    delete fileView;
}


void KDirOperator::setSorting( QDir::SortSpec spec )
{
    fileView->setSorting( spec );
    mySorting = fileView->sorting();
}

void KDirOperator::readNextMimeType()
{
    if (pendingMimeTypes.isEmpty()) {
	kDebugInfo(kfile_area, "all mimetypes checked %ld", time(0));
	return;
    }

    KFileViewItem *item = pendingMimeTypes.first();
    const QPixmap p = item->pixmap();
    (void) item->mimeType();

    if ( item->pixmap().serialNumber() != p.serialNumber() ) // reloads the pixmap in case
        fileView->updateView(item);
    pendingMimeTypes.removeFirst();
    QTimer::singleShot(0, this, SLOT(readNextMimeType()));
}

void KDirOperator::slotKIOError(int, const QString& )
{
    kDebugInfo(kfile_area, "slotKIOError");
    resetCursor();
}

void KDirOperator::resetCursor()
{
    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = false;
    if (progress)
	progress->hide();
    kDebugInfo(kfile_area, "done %ld", time(0));
}

void KDirOperator::activatedMenu( const KFileViewItem *item )
{
    if ( !item ) {
	actionMenu->popup( QCursor::pos() );
    }
}

void KDirOperator::setPreviewWidget(const QWidget *w) {

    if(w != 0L)
        viewKind = (viewKind | KFile::PreviewContents & ~KFile::SeparateDirs);
    else
        viewKind = (viewKind & ~KFile::PreviewContents);
    myPreview=const_cast<QWidget*>(w);
    setView( static_cast<KFile::FileView>(viewKind) );
}

int KDirOperator::numDirs() const
{
    return fileView->numDirs();
}

int KDirOperator::numFiles() const
{
    return fileView->numFiles();
}

void KDirOperator::slotDetailedView()
{
    KFile::FileView view = static_cast<KFile::FileView>( viewKind & ~KFile::Simple | KFile::Detail );
    setView( view );
}

void KDirOperator::slotSimpleView()
{
    KFile::FileView view = static_cast<KFile::FileView>( viewKind & ~KFile::Detail | KFile::Simple );
    setView( view );
}

void KDirOperator::slotToggleHidden( bool show )
{
    dir->setShowHiddenFiles( show );
}

void KDirOperator::slotToggleMixDirsAndFiles()
{
    int flag = ((viewKind & KFile::SeparateDirs) == KFile::SeparateDirs) ? 0 : KFile::SeparateDirs;
    KFile::FileView view = static_cast<KFile::FileView>( viewKind & ~KFile::SeparateDirs | flag );
    setView( view );
}

void KDirOperator::slotSortByName()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Name ));
}

void KDirOperator::slotSortBySize()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Size ));
}

void KDirOperator::slotSortByDate()
{
    int sorting = (fileView->sorting()) & ~QDir::SortByMask;
    fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::Time ));
}

void KDirOperator::slotSortReversed()
{
    fileView->sortReversed();
}

void KDirOperator::slotToggleDirsFirst()
{
    QDir::SortSpec sorting = fileView->sorting();
    if ( (sorting & QDir::DirsFirst) == 0 )
	fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::DirsFirst ));
    else
	fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::DirsFirst));
}

void KDirOperator::slotToggleIgnoreCase()
{
    QDir::SortSpec sorting = fileView->sorting();
    if ( (sorting & QDir::IgnoreCase) == 0 )
	fileView->setSorting( static_cast<QDir::SortSpec>( sorting | QDir::IgnoreCase ));
    else
	fileView->setSorting( static_cast<QDir::SortSpec>( sorting & ~QDir::IgnoreCase));
}

void KDirOperator::mkdir()
{
    if (!dir->isLocalFile())
	return;

    // Modal widget asking the user the name of a new directory
    //
    QDialog *lMakeDir;
    QLabel *label;
    QLineEdit *ed;
    QVBoxLayout *lLayout;
    QPushButton *okButton, *cancelButton;
    QHBoxLayout *lLayout2;

    // Create widgets, and display using geometry management
    //
    lMakeDir = new QDialog(0,"MakeDir Dialog",true);
    lLayout = new QVBoxLayout( lMakeDir, 5 );
    label = new QLabel(lMakeDir);
    label->setAlignment( AlignLeft | AlignVCenter );
    label->setText(i18n("Create new directory in: ") + url().url() );
    label->setMinimumSize( label->sizeHint() );
    ed= new QLineEdit(lMakeDir);
    ed->setText( i18n("New Directory") );
    ed->adjustSize();
    ed->setFixedHeight( ed->height());
    ed->selectAll();
    connect(ed, SIGNAL(returnPressed()), lMakeDir, SLOT(accept()) );

    lLayout->addWidget( label, 0);
    lLayout->addSpacing( 5 );
    lLayout->addWidget( ed, 0);

    okButton = new QPushButton( lMakeDir, "okButton" );
    okButton->adjustSize();
    okButton->setText( i18n("&OK") );
    okButton->setFixedHeight( okButton->height() );
    okButton-> setMinimumWidth( okButton->width() );

    cancelButton = new QPushButton( lMakeDir, "cancelButton" );
    cancelButton->setText( i18n("&Cancel") );
    cancelButton->adjustSize();
    cancelButton->setFixedHeight( cancelButton->height() );
    cancelButton->setMinimumWidth( cancelButton->width() );

    connect( okButton, SIGNAL(clicked()), lMakeDir, SLOT(accept()) );
    connect( cancelButton, SIGNAL(clicked()), lMakeDir, SLOT(reject()) );

    lLayout2 = new QHBoxLayout( 5 );
    lLayout->addSpacing( 10 );
    lLayout->addLayout( lLayout2 , 0);
    lLayout2->addWidget( okButton, 3);
    lLayout2->addWidget( cancelButton, 3);
    lLayout->activate();

    // If the users presses enter (not escape) then create the dir
    // and insert it into the ListBox
    lMakeDir->resize( 10, 10);
    ed->grabKeyboard();
    if ( lMakeDir->exec() == QDialog::Accepted ) {

        // check if we are allowed to create directories
        bool writeOk = false;
	QString tmp = ed->text();
	int idx = tmp.findRev(QString::fromLatin1("/"));
	if ( idx != -1 ) {
	    QDir dir( url().path() );
	    if ( dir.cd( tmp.left( idx )) )
	        writeOk = checkAccess( dir.absPath(), W_OK );
	}
	else
	    writeOk = checkAccess( url().path(), W_OK );

	
	if ( !writeOk ) {
	    KMessageBox::sorry(0,
			       i18n("You don't have permissions to create "
				    "that directory." ));
	    return;
	}
	if ( QDir(url().path()).mkdir(ed->text()) == true ) {  // !! don't like this move it into KFileReader ??
	    setURL( KURL(url(), ed->text()), true );
	}
    }

    delete lMakeDir;
}

void KDirOperator::close()
{
    resetCursor();
    pendingMimeTypes.clear();
    myCompletion.clear();
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
    // no absolut path, we add the current directory to get a correct
    // url
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
    kDebugInfo(kfile_area, "TODO checkPath");
}

void KDirOperator::setURL(const KURL& _newurl, bool clearforward)
{
    KURL newurl = _newurl;

    QString pathstr = newurl.path(+1);

    if (pathstr.isEmpty() || pathstr.at(pathstr.length() - 1) != '/')
	pathstr += '/';
    newurl.setPath(pathstr);

    if (newurl == *dir) // already set
	return;

    kDebugInfo(kfile_area, "setURL %s %ld (%s)", debugString(newurl.url()), time(0), debugString(dir->url()));

    /*
       what is the sense of this? If it's set, it's set, not?
       this was for blending out/in completion matches (pfeiffer)
       if (dir->url() == pathstr) { // already set
       fileView->clear();
       fileView->addItemList(dir->currentContents());
       return;
       }
    */

    pendingMimeTypes.clear();

    if (clearforward) {
	// autodelete should remove this one
	backStack.push(new KURL(*dir));
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

    KURL backup(*dir);
    dir->setURL(newurl);

    if (!dir->isReadable()) {
	KMessageBox::error(0,
			   i18n("The specified directory does not exist\n"
				"or was not readable."));
	dir->setURL(backup);
	backStack.pop();
    } else {

        myCompletion.clear();
	emit urlEntered(*dir);
	pathChanged();
    }


    // enable/disable actions
    forwardAction->setEnabled( !forwardStack.isEmpty() );
    backAction->setEnabled( !backStack.isEmpty() );
    upAction->setEnabled( !isRoot() );
}

void KDirOperator::rereadDir()
{
    // some would call this dirty. I don't ;)
    dir->setURL(dir->url());
    pathChanged();
}

// Protected
void KDirOperator::pathChanged()
{
    if (!fileView)
	return;

    kDebugInfo(kfile_area, "changed %ld", time(0));

    fileView->clear();
    myCompletion.clear();

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
	KMessageBox::error(0,
			   i18n("The specified directory does not exist "
				"or was not readable."));
	if (backStack.isEmpty())
	    home();
	else
	    back();
    }

    dir->listContents();
}

// Code pinched from kfm then hacked
void KDirOperator::back()
{
    if ( backStack.isEmpty() )
	return;

    forwardStack.push( new KURL(*dir) );

    KURL *s = backStack.pop();

    setURL(*s, false);
    delete s;
}

// Code pinched from kfm then hacked
void KDirOperator::forward()
{
    if ( forwardStack.isEmpty() )
	return;

    backStack.push(new KURL(*dir));

    KURL *s = forwardStack.pop();
    setURL(*s, false);
    delete s;
}

KURL KDirOperator::url() const
{
    return *dir;
}

void KDirOperator::cdUp()
{
    KURL tmp( dir->url() );
    tmp.cd(QString::fromLatin1(".."));
    setURL(tmp.url(), true);
}

void KDirOperator::home()
{
    setURL(QDir::homeDirPath(), true);
}

void KDirOperator::setNameFilter(const QString& filter)
{
    dir->setNameFilter(filter);
}

bool KDirOperator::isRoot() const
{
    return dir->isRoot();
}

void KDirOperator::setView( KFile::FileView view )
{
    bool separateDirs = (view & KFile::SeparateDirs) == KFile::SeparateDirs;

    // TODO write the config
    if (view == KFile::Default) {
	KConfig *c= KGlobal::config();
	KConfigGroupSaver sc(c, ConfigGroup);

	if (c->readEntry(QString::fromLatin1("ViewStyle"), DefaultViewStyle)
	    == QString::fromLatin1("DetailView"))
	    view = KFile::Detail;
	else
	    view = KFile::Simple;

	separateDirs = ! c->readBoolEntry( QString::fromLatin1("MixDirsAndFiles"), DefaultMixDirsAndFiles );
    }

    bool preview=( (view & KFile::PreviewInfo) == KFile::PreviewInfo ||
		   (view & KFile::PreviewContents) == KFile::PreviewContents );

    // we only have a dual combi view, not a triple one. So in Directory and
    // preview mode, we don't allow separating dirs & files
    if ( (mode() & KFile::Directory) == KFile::Directory || preview )
	separateDirs = false;

    viewKind = static_cast<int>(view) | (separateDirs ? KFile::SeparateDirs : 0);

    KFileView *new_view = 0L;

    if (separateDirs) {
        KCombiView *combi = new KCombiView(this, "combi view");
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
            new_view = new KFileIconView( this, "simple view" );
	    new_view->setViewName( i18n("Short View") );
	}
        else if ( (view & KFile::Detail) == KFile::Detail && !preview )
            new_view = new KFileDetailView( this, "detail view" );

        else { // FIXME: make Preview and Detail possible?
            KFilePreview *tmp = new KFilePreview(this, "preview");
            tmp->setPreviewWidget(myPreview, url());
            new_view=tmp;
        }
    }

    if ( (mode() & KFile::Directory) )
        new_view->setViewMode(KFileView::Directories);
    else
        new_view->setViewMode(KFileView::All);

    setFocusProxy(new_view->widget());
    new_view->setSorting( mySorting );
    connectView(new_view);
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
	QTimer::singleShot(0, this, SLOT(deleteOldView()));
    }

    fileView = view;
    fileView->setOperator(this);

    if ( myMode == KFile::File ) {
	fileView->setViewMode( KFileView::All );
	fileView->setSelectionMode( KFile::Single );
    } else if ( myMode == KFile::Directory ) {
	fileView->setViewMode( KFileView::Directories );
	fileView->setSelectionMode( KFile::Single );
    } else if ( myMode == KFile::Files ) {
	fileView->setViewMode( KFileView::All );
	fileView->setSelectionMode( KFile::Extended );
    }
    else if ( (viewKind & KFile::PreviewContents) == KFile::PreviewContents ||
	      (viewKind & KFile::PreviewInfo) == KFile::PreviewInfo ) {
	fileView->setViewMode( KFileView::All );
	fileView->setSelectionMode( KFile::Single );
    }

    dir->listContents();

    // set the actions (without calling the slots)
    byNameAction->blockSignals( true );
    byDateAction->blockSignals( true );
    bySizeAction->blockSignals( true );
    reverseAction->blockSignals( true );
    dirsFirstAction->blockSignals( true );
    caseInsensitiveAction->blockSignals( true );
    showHiddenAction->blockSignals( true );
    separateDirsAction->blockSignals( true );

    shortAction->blockSignals( true );
    detailedAction->blockSignals( true );

    QDir::SortSpec sorting = fileView->sorting();
	
    // grr, who had the idea to set QDir::Name to 0x0?
    bool byName = (sorting & QDir::Time) != QDir::Time && (sorting & QDir::Size) != QDir::Size;
    byNameAction->setChecked( byName );
    byDateAction->setChecked((sorting & QDir::Time) == QDir::Time);
    bySizeAction->setChecked((sorting & QDir::Size) == QDir::Size);
    reverseAction->setChecked( fileView->isReversed() );
    dirsFirstAction->setChecked((sorting & QDir::DirsFirst)== QDir::DirsFirst);
    caseInsensitiveAction->setChecked( (sorting & QDir::IgnoreCase ) == QDir::IgnoreCase );
    // showHiddenAction->setChecked( showHiddenFiles() );
    separateDirsAction->setChecked( (viewKind & KFile::SeparateDirs) == KFile::SeparateDirs);

    shortAction->setChecked( (fileView->viewName() == i18n("Short View")));
    detailedAction->setChecked((fileView->viewName() ==i18n("Detailed View")));

    byNameAction->blockSignals( false );
    byDateAction->blockSignals( false );
    bySizeAction->blockSignals( false );
    reverseAction->blockSignals( false );
    dirsFirstAction->blockSignals( false );
    caseInsensitiveAction->blockSignals( false );
    showHiddenAction->blockSignals( false );
    separateDirsAction->blockSignals( false );

    shortAction->blockSignals( false );
    detailedAction->blockSignals( false );
    //////////

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

    // reset the view with the different mode
    setView( static_cast<KFile::FileView>(viewKind) );
}

void KDirOperator::setView(KFileView *view)
{
    connectView(view);
    // TODO: this is a hack! It should be 0
    // viewKind = KFile::Simple | KFile::SeparateDirs;
}

void KDirOperator::setFileReader( KFileReader *reader )
{
    delete dir;
    dir = reader;
}

void KDirOperator::insertNewFiles(const KFileViewItemList &newone, bool ready)
{
    if (newone.isEmpty() && !ready)
	return;

    kDebugInfo(kfile_area, "insertNewFiles %d", newone.count());
    myCompleteListDirty = true;

    bool isLocal = dir->isLocalFile();
    if (!isLocal)
	fileView->addItemList(newone);

    KFileViewItemListIterator it(newone);
    for( ; it.current(); ++it ) {
        KFileViewItem *item = it.current();

	if ( isLocal && !item->isHidden() )
	    pendingMimeTypes.append( item );
    }

    if (ready) {
	if (progress) // it may not be visible even if not 0
	    progress->setValue(100);

	if (isLocal) {
		fileView->clear();
		fileView->addItemList(dir->currentContents());
	}

	QTimer::singleShot(0, this, SLOT(readNextMimeType()));
	QTimer::singleShot(200, this, SLOT(resetCursor()));
    } else {
	if (!progress) {
	    progress = new KProgress(this, "progress");
	    progress->adjustSize();
	    progress->setRange(0, 100);
	    progress->move(2, height() - progress->height() -2);
	}
	progress->setValue(ulong(dir->count() * 100)/ dir->dirCount());
	progress->raise();
	progress->show();

	// we have to redraw this in as fast as possible
	QApplication::flushX();
    }
    emit updateInformation(fileView->numDirs(), fileView->numFiles());
}

void KDirOperator::selectDir(const KFileViewItem *item)
{
    KURL tmp( dir->url() );
    tmp.cd(item->name());
    setURL(tmp.url(), true);
}

void KDirOperator::itemsDeleted(const KFileViewItemList &list)
{
    if ( list.count() == 0 )
        return;

    KFileViewItemListIterator it(list);
    for( ; it.current(); ++it )
        fileView->updateView( it.current() );

    emit updateInformation(fileView->numDirs(), fileView->numFiles());
}

void KDirOperator::selectFile(const KFileViewItem *item)
{
    KURL tmp ( dir->url() );
    tmp.setFileName(item->name());

    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = false;
}


void KDirOperator::filterChanged()
{
    fileView->clear();
    myCompletion.clear();
    dir->listContents();
    emit updateInformation(fileView->numDirs(), fileView->numFiles());
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        fileView->clearSelection();
        return QString::null;
    }

    if ( myCompleteListDirty ) { // create the list of all possible completions
        KFileViewItemListIterator it( dir->currentContents());
	for( ; it.current(); ++it ) {
            KFileViewItem *item = it.current();

	    if ( !item->isHidden() )
	        myCompletion.addItem( item->name() );
	}
	myCompleteListDirty = false;
    }

    return myCompletion.makeCompletion( string );
}

void KDirOperator::slotCompletionMatch(const QString& match)
{
    const KFileViewItem *item = 0L;

    if ( !match.isNull() )
        item = dir->currentContents().findByName( match );
    else
        fileView->clearSelection();

    if ( item )
        fileView->setCurrentItem( QString::null, item );
    emit completion( match );
}

void KDirOperator::slotCompletionMatches(const QStringList& /*matches*/)
{
  /*
    KFileViewItemList list;
    KFileViewItem *item = 0L;
    const KFileViewItemList *contents = dir->currentContents();

    if ( !matches.isEmpty() ) {
        QStringList::ConstIterator it;
	for( it = matches.begin(); it != matches.end(); ++it ) {
	    item = contents->findByName( *it );
	    if ( item && !item->isHidden() )
	        list.append( item );
	}

	fileView->clear();
	if ( !list.isEmpty() )
	    fileView->addItemList( &list );
    }

    emit updateInformation(fileView->numDirs(), fileView->numFiles());
  */
}


void KDirOperator::setupActions()
{
    actionMenu = new KActionMenu( i18n("Menu"), this, "popupMenu" );

    upAction = KStdAction::up( this, SLOT( cdUp() ), this, "up" );
    backAction = KStdAction::back( this, SLOT( back() ), this, "back" );
    forwardAction = KStdAction::forward(this, SLOT(forward()), this,"forward");
    homeAction = KStdAction::home( this, SLOT( home() ), this, "home" );
    reloadAction =KStdAction::redisplay(this,SLOT(rereadDir()),this, "reload");
    actionSeparator = new KActionSeparator( this, "separator" );
    mkdirAction = new KAction( i18n("New Folder..."), 0,
				 this, SLOT( mkdir() ), this, "mkdir");
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
    shortAction = new KRadioAction( i18n("Short View"), 0,
				     this, "short view" );
    detailedAction = new KRadioAction( i18n("Detailed View"), 0,
					this, "detailed view" );

    showHiddenAction = new KToggleAction( i18n("Show Hidden Files"), 0,
					  this, "show hidden" );
    separateDirsAction = new KToggleAction( i18n("Separate Directories"), 0,
					    this,
					    SLOT(slotToggleMixDirsAndFiles()),
					    this, "separate dirs" );

    QString viewGroup = QString::fromLatin1("view");
    shortAction->setExclusiveGroup( viewGroup );
    detailedAction->setExclusiveGroup( viewGroup );

    connect( shortAction, SIGNAL( toggled( bool ) ),
	     SLOT( slotSimpleView() ));
    connect( detailedAction, SIGNAL( toggled( bool ) ),
	     SLOT( slotDetailedView() ));
    connect( showHiddenAction, SIGNAL( toggled( bool ) ),
	     SLOT( slotToggleHidden( bool ) ));


    // insert them into the actionCollection
    myActionCollection = new QActionCollection( this, "action collection" );
    myActionCollection->insert( actionMenu );
    myActionCollection->insert( backAction );
    myActionCollection->insert( forwardAction );
    myActionCollection->insert( homeAction );
    myActionCollection->insert( upAction );
    myActionCollection->insert( reloadAction );
    myActionCollection->insert( actionSeparator );
    myActionCollection->insert( mkdirAction );
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
    myActionCollection->insert( separateDirsAction );
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
    viewActionMenu->insert( separateDirsAction );


    // now plug everything into the popupmenu
    actionMenu->insert( upAction );
    actionMenu->insert( backAction );
    actionMenu->insert( forwardAction );
    actionMenu->insert( homeAction );
    actionMenu->insert( actionSeparator );

    actionMenu->insert( mkdirAction );
    actionMenu->insert( actionSeparator );

    actionMenu->insert( sortActionMenu );
    actionMenu->insert( actionSeparator );
    actionMenu->insert( viewActionMenu );
}



void KDirOperator::resizeEvent( QResizeEvent * )
{
    if (fileView)
	fileView->widget()->resize( size() );
    if ( progress )
	progress->move(2, height() - progress->height() -2);
}

#include "kdiroperator.moc"
