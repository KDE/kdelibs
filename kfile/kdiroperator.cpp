#include "kdiroperator.h"
#include "kfileviewitem.h"
#include "kfilereader.h"
#include "kfileview.h"
#include "config-kfile.h"
#include <qapplication.h>
#include <qtimer.h>
#include <kmessagebox.h>
#include <qpopupmenu.h>
#include <klocale.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <kcombiview.h>
#include <kfilepreview.h>
#include <kfileiconview.h>
#include <kfiledetailview.h>
#include <kprogress.h>
#include <kdebug.h>

// for public enums
#include <kfiledialog.h>

template class QStack<KURL>;
template class QDict<KFileViewItem>;

QString *KDirOperator::lastDirectory = 0; // to set the start path

KDirOperator::KDirOperator(const QString& dirName,
			   QWidget *parent, const char* _name)
    : QWidget(parent, _name), fileList(0), oldList(0), progress(0)
{


    _mode=KFileDialog::Files;

    if (dirName.isEmpty()) // no dir specified -> current dir
	lastDirectory = new QString(QDir::currentDirPath());
    else
	lastDirectory = new QString(dirName);

    dir = new KFileReader(*lastDirectory);
    dir->setAutoUpdate( true );

    connect(dir, SIGNAL(contents(const KFileViewItemList *, bool)),
	    SLOT(insertNewFiles(const KFileViewItemList *, bool)));
    connect(dir, SIGNAL(itemsDeleted(const KFileViewItemList *)),
	    SLOT(itemsDeleted(const KFileViewItemList *)));
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
    setShowHiddenFiles(c->readBoolEntry(QString::fromLatin1("ShowHidden"), DefaultShowHidden));
    setFocusPolicy(QWidget::WheelFocus);
}

KDirOperator::~KDirOperator()
{
    delete fileList;
}

void KDirOperator::readNextMimeType()
{
    if (pendingMimeTypes.isEmpty()) {
	debugC("all mimetypes checked %ld", time(0));
	return;
    }

    KFileViewItem *item = pendingMimeTypes.first();
    const QPixmap p = item->pixmap();
    (void) item->mimeType();

    if ( item->pixmap().serialNumber() != p.serialNumber() ) // reloads the pixmap in case
        fileList->updateView(item);
    pendingMimeTypes.removeFirst();
    QTimer::singleShot(0, this, SLOT(readNextMimeType()));
}

void KDirOperator::slotKIOError(int, const QString& )
{
    debugC("slotKIOError");
    resetCursor();
}

void KDirOperator::resetCursor()
{
    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = false;
    if (progress)
	progress->hide();
    debugC("\ndone %ld\n", time(0));
}

void KDirOperator::activatedMenu( const KFileViewItem *item )
{
    if ( !item ) {
	QPopupMenu *p = new QPopupMenu();
	int id;
	id = p->insertItem(BarIcon(QString::fromLatin1("up")),
			   i18n("Up"), this, SLOT(cdUp()));
	p->setItemEnabled(id, !isRoot());

	id = p->insertItem(BarIcon(QString::fromLatin1("back")),
			   i18n("Back"), this, SLOT(back()));
	p->setItemEnabled(id, !backStack.isEmpty());

	id = p->insertItem(BarIcon(QString::fromLatin1("forward")),
			   i18n("Forward"), this, SLOT(forward()));
	p->setItemEnabled(id, !forwardStack.isEmpty());

	id = p->insertItem(BarIcon(QString::fromLatin1("home")),
			   i18n("Home"), this, SLOT(home()));

	p->insertSeparator();

	id = p->insertItem(i18n("New Folder..."),
		      this, SLOT(mkdir()));
	p->setItemEnabled(id, dir->isLocalFile());

	if (viewKind) {
	    p->insertSeparator();

	    QPopupMenu *views = new QPopupMenu(p);
	    views->insertItem(i18n("Short View"),
			      this, SLOT(simpleView()));
	    views->insertItem(i18n("Detailed View"),
			      this, SLOT(detailedView()));
	    views->insertSeparator();
	    id = views->insertItem(i18n("Show Hidden Files"),
			       this, SLOT(toggleHidden()));
	    views->setItemChecked(id, showHiddenFiles());

	    id = views->insertItem(i18n("Separate Directories"),
				   this, SLOT(toggleMixDirsAndFiles()));
	    views->setItemChecked(id, (viewKind & SeparateDirs) == SeparateDirs);

	    id = p->insertItem(i18n("View"), views);
	}

	p->exec( QCursor::pos() );
	delete p;
    }
}

void KDirOperator::detailedView()
{
    setView(Detail, (viewKind & SeparateDirs) == SeparateDirs);
}

void KDirOperator::simpleView()
{
    setView(Simple, (viewKind & SeparateDirs) == SeparateDirs);
}

void KDirOperator::setPreviewWidget(const QWidget *w) {
    _mode=static_cast<KFileDialog::Mode>(_mode | KFileDialog::Preview);
    preview=const_cast<QWidget*>(w);
    setView(Simple, false);
}

int KDirOperator::numDirs() const
{
    return fileList->numDirs();
}

int KDirOperator::numFiles() const
{
    return fileList->numFiles();
}

void KDirOperator::toggleHidden()
{
    setShowHiddenFiles(!showHidden);
}

void KDirOperator::toggleMixDirsAndFiles()
{
    setView(static_cast<FileView>(viewKind & ~SeparateDirs), (viewKind & SeparateDirs) != SeparateDirs);
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
    debug("TODO checkPath");
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

    debugC("setURL %s %ld (%s)", debugString(newurl.url()), time(0), debugString(dir->url()));

    /*
       what is the sense of this? If it's set, it's set, not?
       this was for blending out/in completion matches (pfeiffer)
       if (dir->url() == pathstr) { // already set
       fileList->clear();
       fileList->addItemList(dir->currentContents());
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
    } else {

        myCompletion.clear();
	emit urlEntered(*dir);
	pathChanged();
    }
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
    if (!fileList)
	return;

    debugC("changed %ld", time(0));

    fileList->clear();
    myCompletion.clear();

    // it may be, that we weren't ready at this time
    if (!finished)
	QApplication::restoreOverrideCursor();

    finished = false;

    // when KIOJob emits finished, the slot will restore the cursor
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

void KDirOperator::setShowHiddenFiles(bool b)
{
    showHidden = b;
    dir->setShowHiddenFiles(b);
}

bool KDirOperator::showHiddenFiles() const
{
    return showHidden;
}

void KDirOperator::setView(FileView view, bool separateDirs)
{
    // TODO write the config
    if (view == Default) {
	KConfig *c= KGlobal::config();
	KConfigGroupSaver sc(c, ConfigGroup);

	if ( c->readEntry( QString::fromLatin1("ViewStyle"),
			   DefaultViewStyle) ==
	     QString::fromLatin1("DetailView") ) {
	    view = Detail;
	} else {
	    view = Simple;
	}

	separateDirs = ! c->readBoolEntry( QString::fromLatin1("MixDirsAndFiles"), DefaultMixDirsAndFiles );
    }

    // if only directories separating makes only limited sense :)
    if ( (mode() & KFileDialog::Directory) == KFileDialog::Directory ||
         (mode() & KFileDialog::Preview) == KFileDialog::Preview )
        separateDirs = false;

    viewKind = static_cast<int>(view) | (separateDirs ? SeparateDirs : 0);

    KFileView *myFileView = 0;

    if (separateDirs) {
        KCombiView *combi = new KCombiView(this, "combi view");
        if (view == Simple)
            combi->setRight(new KFileIconView( combi, "simple view" ));
        else
            combi->setRight(new KFileDetailView( combi, "detail view" ));
        myFileView = combi;
    } else {
        if (view == Simple)
            myFileView = new KFileIconView( this, "simple view" );
        else if (view == Detail)
            myFileView = new KFileDetailView( this, "detail view" );
        else {
            KFilePreview *tmp=new KFilePreview(this, "preview");
            tmp->setPreviewWidget(preview);
            myFileView=tmp;
        }
    }

    if ( (mode() & KFileDialog::Directory) )
        myFileView->setViewMode(KFileView::Directories);
    else
        myFileView->setViewMode(KFileView::All);

    setFocusProxy(myFileView->widget());
    connectView(myFileView);
}

void KDirOperator::deleteOldView()
{
    oldList->widget()->hide();
    delete oldList;
    oldList = 0;
}

void KDirOperator::connectView(KFileView *view)
{
    // TODO: do a real timer and restart it after that
    pendingMimeTypes.clear();

    if (fileList) {
        QApplication::setOverrideCursor( waitCursor );
	finished = false;
	fileList->setOperator(0);
	// it's _very_ unlikly that oldList still has a value
	oldList = fileList;
	QTimer::singleShot(0, this, SLOT(deleteOldView()));
    }

    fileList = view;
    fileList->setOperator(this);

    if ( _mode == KFileDialog::File ) {
	fileList->setViewMode( KFileView::All );
	fileList->setSelectMode( KFileView::Single );
    } else if ( _mode == KFileDialog::Directory ) {
	fileList->setViewMode( KFileView::Directories );
	fileList->setSelectMode( KFileView::Single );
    } else if ( _mode == KFileDialog::Files ) {
	fileList->setViewMode( KFileView::All );
	fileList->setSelectMode( KFileView::Multi );
    }
    else if( _mode == KFileDialog::Preview ) {
    fileList->setViewMode( KFileView::All );
    fileList->setSelectMode( KFileView::Single );
    }

    dir->listContents();
    fileList->widget()->show();
    fileList->widget()->resize(size());
}

KFileDialog::Mode KDirOperator::mode() const
{
    return _mode;
}

void KDirOperator::setMode(KFileDialog::Mode m)
{
    if (_mode == m)
	return;

    _mode = m;

    // reset the view with the different mode
    setView(static_cast<FileView>(viewKind & ~SeparateDirs), (viewKind & SeparateDirs) == SeparateDirs);
}

void KDirOperator::setView(KFileView *view)
{
    connectView(view);
    // TODO: this is a hack! It should be 0
    viewKind = Simple | SeparateDirs;
}

void KDirOperator::setFileReader( KFileReader *reader )
{
    delete dir;
    dir = reader;
}

void KDirOperator::insertNewFiles(const KFileViewItemList *newone, bool ready)
{
    if (!newone)
	return;

    debug("insertNewFiles %d", newone->count());
    myCompleteListDirty = true;

    bool isLocal = dir->isLocalFile();
    if (!isLocal)
	fileList->addItemList(newone);

    KFileViewItemListIterator it(*newone);
    for( ; it.current(); ++it ) {
        KFileViewItem *item = it.current();

	if ( isLocal && !item->isHidden() )
	    pendingMimeTypes.append( item );
    }

    if (ready) {
	if (progress) // it may not be visible even if not 0
	    progress->setValue(100);

	if (isLocal)
	    fileList->addItemList(dir->currentContents());

	QTimer::singleShot(0, this, SLOT(readNextMimeType()));
	QTimer::singleShot(200, this, SLOT(resetCursor()));
    } else {
	if (!progress) {
	    progress = new KProgress(this, "progress");
	    progress->setRange(0, 100);
	    progress->move(0, 0);
	}
	progress->setValue(ulong(dir->count() * 100)/ dir->dirCount());
	progress->raise();
	progress->show();

	// we have to redraw this in as fast as possible
	QApplication::flushX();
    }
    emit updateInformation(fileList->numDirs(), fileList->numFiles());
}

void KDirOperator::selectDir(const KFileViewItem *item)
{
    KURL tmp( dir->url() );
    tmp.cd(item->name());
    setURL(tmp.url(), true);
}

void KDirOperator::itemsDeleted(const KFileViewItemList *list)
{
    if ( !list || list->count() == 0 )
        return;

    KFileViewItemListIterator it(*list);
    for( ; it.current(); ++it )
        fileList->updateView( it.current() );

    emit updateInformation(fileList->numDirs(), fileList->numFiles());
}

void KDirOperator::selectFile(const KFileViewItem *item)
{
    KURL tmp ( dir->url() );
    tmp.setFileName(item->name());

    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = false;
}

void KDirOperator::highlightFile(const KFileViewItem *i)
{
    emit fileHighlighted(i);
}

void KDirOperator::filterChanged()
{
    fileList->clear();
    myCompletion.clear();
    dir->listContents();
    emit updateInformation(fileList->numDirs(), fileList->numFiles());
}

QString KDirOperator::makeCompletion(const QString& string)
{
    if ( string.isEmpty() ) {
        fileList->clearSelection();
        return QString::null;
    }

    if ( myCompleteListDirty ) { // create the list of all possible completions
        KFileViewItemListIterator it( *(dir->currentContents()));
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
        item = dir->currentContents()->findByName( match );
    else
        fileList->clearSelection();

    if ( item )
        fileList->setCurrentItem( QString::null, item );
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

	fileList->clear();
	if ( !list.isEmpty() )
	    fileList->addItemList( &list );
    }

    emit updateInformation(fileList->numDirs(), fileList->numFiles());
  */
}


void KDirOperator::resizeEvent( QResizeEvent * )
{
    if (fileList)
	fileList->widget()->resize( size() );
}

#include "kdiroperator.moc"

