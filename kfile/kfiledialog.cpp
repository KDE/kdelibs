#include "kfileview.h"
#include "krecentdocument.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <qtimer.h>
#include <qtabdialog.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qstack.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qpixmap.h>
#include <qcollection.h>
#include <kmessagebox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcolor.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qaccel.h>
#include <kurl.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include "kfiledialogconf.h"
#include "kfiledialog.h"
#include "kfileiconview.h"
#include "kfiledetailview.h"
#include "kcombiview.h"
#include "config-kfile.h"
#include "kfilefilter.h"
#include "kfilereader.h"
#include "kfilebookmark.h"
#include <kprocess.h>
#include <kapp.h>
#include <klocale.h>
#include <kio_job.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include "kdiroperator.h"
#include <kdebug.h>

enum Buttons { HOTLIST_BUTTON,
	       PATH_COMBO, CONFIGURE_BUTTON };

const int idStart = 1;

struct KFileDialogPrivate
{
    // the last selected filename
    QString filename_;
    // the name of the filename set by setSelection
    QString selection;

    // we need this to determine what has changed in the location bar
    QString completionHack;

    /*
     * indicates, if the status bar should be shown.
     * Initialized by "ShowStatusLine"
     */
    bool showStatusLine;

    // indicates, if the file selector accepts just existing
    // files or not. If set to true, it will check for local
    // files, if they exist
    bool acceptOnlyExisting;

    // now following all kind of widgets, that I need to rebuild
    // the geometry managment
    QVBoxLayout *boxLayout;
    QGridLayout *lafBox;
    QHBoxLayout *btngroup;

    QWidget *mainWidget;

    QLabel *myStatusLine;

    QLabel *locationLabel;
    QLabel *filterLabel;
};

QString *KFileDialog::lastDirectory; // to set the start path

KFileDialog::KFileDialog(const QString& dirName, const QString& filter,
			 QWidget *parent, const char* name, bool modal)
    : KDialogBase( parent, name, modal, QString::null, KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok )
{
    d = new KFileDialogPrivate();
    d->boxLayout = 0;
    d->mainWidget = new QWidget( this, "KFileDialog::mainWidget");
    setMainWidget( d->mainWidget );

    d->myStatusLine = 0;

    // I hard code this for now
    d->acceptOnlyExisting = false;
    bookmarksMenu= 0;

    if (!lastDirectory)
    {
	qAddPostRoutine( cleanup );
	if (dirName.isEmpty()) // no dir specified -> current dir
	    lastDirectory = new QString(QDir::currentDirPath());
        else
	    lastDirectory = new QString(dirName);
    }

    // we remember the selected name for init()
    d->filename_ = *lastDirectory;

    ops = new KDirOperator(*lastDirectory, d->mainWidget, "KFileDialog::ops");
    connect(ops, SIGNAL(updateInformation(int, int)),
	    SLOT(updateStatusLine(int, int)));
    connect(ops, SIGNAL(urlEntered(const KURL&)),
	    SLOT(urlEntered(const KURL&)));

    visitedDirs = new QStringList();

    // Get the config object
    KConfig *c= KGlobal::config();
    KConfigGroupSaver tmp(c, ConfigGroup);

    //
    // Read configuration from the config file
    //
    int w, h;
    QWidget *desk = QApplication::desktop();
    w = QMIN( 530, desk->width() * 5 / 10 ); // maximum default width = 530
    h = desk->height() * 2 / 6;
    w = c->readNumEntry(QString::fromLatin1("Width%1").arg( desk->width()), w);
    h = c->readNumEntry(QString::fromLatin1("Height%1").arg( desk->height()), h);

    d->showStatusLine = c->readBoolEntry(QString::fromLatin1("ShowStatusLine"), DefaultShowStatusLine);

    toolbar= new KToolBar( d->mainWidget, "KFileDialog::toolbar");

    /*
      toolbar->insertButton(BarIcon("reload"),
      RELOAD_BUTTON, true,
      i18n("Reload"));
    */

    bookmarks= new KFileBookmarkManager();
    CHECK_PTR( bookmarks );
    connect( bookmarks, SIGNAL( changed() ),
	     this, SLOT( bookmarksChanged() ) );

    QString bmFile = locate("data", QString::fromLatin1("kdeui/bookmarks.html"));
    if (!bmFile.isNull())
	bookmarks->read(bmFile);

    toolbar->insertButton(BarIcon(QString::fromLatin1("flag")),
			  HOTLIST_BUTTON, true,
			  i18n("Bookmarks"), -1);

    toolbar->insertButton(BarIcon(QString::fromLatin1("configure")),
			  CONFIGURE_BUTTON, true,
			  i18n("Configure this dialog"));

    connect(toolbar, SIGNAL(clicked(int)),
	    SLOT(toolbarCallback(int)));
    // try this again
    connect(toolbar, SIGNAL(pressed(int)),
    	    this, SLOT(toolbarPressedCallback(int)));

    toolbar->insertCombo(i18n("The path"), PATH_COMBO, false,
			 SIGNAL(activated(int)),
			 this, SLOT(comboActivated(int)),
			 true, i18n("Current Path"));

    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->enableMoving(false);
    toolbar->adjustSize();

    locationEdit = new KFileComboBox(true, d->mainWidget, "KFileDialog::locationEdit");
    locationEdit->setInsertionPolicy(QComboBox::NoInsertion);
    locationEdit->setFocus();

    d->locationLabel = new QLabel(locationEdit, i18n("&Location:"), d->mainWidget);
    d->locationLabel->adjustSize();
    locationEdit->adjustSize();
    d->locationLabel->setMinimumSize(d->locationLabel->width(),
				     locationEdit->height());
    locationEdit->setFixedHeight(d->locationLabel->height());

    //    connect(locationEdit, SIGNAL(activated(const QString&)),
    //    	    SLOT(locationChanged(const QString&)));
    connect(locationEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(locationChanged(const QString&)));
    connect(locationEdit, SIGNAL(completion()), SLOT(completion()));


    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    d->filterLabel->adjustSize();
    d->filterLabel->setMinimumWidth(d->filterLabel->width());
    d->filterLabel->resize(d->locationLabel->width(), d->filterLabel->height());

    filterWidget = new KFileFilter(d->mainWidget, "KFileDialog::filterwidget");
    filterWidget->setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    filterWidget->adjustSize();
    filterWidget->setMinimumWidth(100);
    filterWidget->setFixedHeight(filterWidget->height());
    connect(filterWidget, SIGNAL(filterChanged()),
	    SLOT(filterChanged()));

    connect( this, SIGNAL( okClicked() ), SLOT( okPressed() ) );

    connect( this, SIGNAL( cancelClicked() ), SLOT( reject() ) );

    connect( locationEdit, SIGNAL( returnPressed() ),
	     SLOT( returnPressed()));

    if (d->filename_ == QString::fromLatin1(".") || d->filename_.isEmpty())
	d->filename_ = QString::fromLatin1("file:") + QDir::currentDirPath();

    // filename_ is remembered as the dirName argument for the constructor

    // FIXME:
    // set the view _after_ calling setSelection(), otherwise we would read
    // the startdirectory twice. This must be fixed somehow else, tho.
    setSelection(d->filename_);
    ops->setView( KDirOperator::Default);

    initGUI(); // activate GM

    if (!d->filename_.isNull()) {
	debugC("edit %s", debugString(locationEdit->text(0)));
	checkPath(d->filename_);
	locationEdit->setEditText(d->filename_);
    }
    adjustSize();
    int w1 = minimumSize().width();
    int w2 = toolbar->sizeHint().width() + 10;
    if (w1 < w2)
	setMinimumWidth(w2);
    resize(w, h);
}

void KFileDialog::cleanup() {
    delete lastDirectory;
    lastDirectory = 0;
}

void KFileDialog::setFilter(const QString& filter)
{
    filterWidget->setFilter(filter);
    ops->setNameFilter(filterWidget->currentFilter());
}

void KFileDialog::okPressed()
{
    d->filename_ = locationEdit->currentText();

    KURL u( d->filename_ );
    if ( u.isLocalFile() ) {
	if ( QFileInfo( u.path() ).isDir() ) {
	    setURL( QDir::cleanDirPath( u.path() ) );
	    return;
	}
    }

    debugC("filename %s", debugString(d->filename_));

    accept();
}


void KFileDialog::returnPressed()
{
  if ( locationEdit->currentText().stripWhiteSpace().isEmpty() )
    return;

  debug("** return pressed **");
  if ( mode() == Directory ) {
    //    KFileRea

  }
}


void KFileDialog::initGUI()
{
    if (d->boxLayout)
	delete d->boxLayout; // deletes all sub layouts

    d->boxLayout = new QVBoxLayout( d->mainWidget, 0, 2);
    d->boxLayout->addWidget(toolbar, AlignTop);
    d->boxLayout->addWidget(ops, 4);
    d->boxLayout->addSpacing(3);

    d->lafBox= new QGridLayout(2, 2, 7);
    d->boxLayout->addLayout(d->lafBox, 0);
    d->lafBox->addWidget(d->locationLabel, 0, 0);
    d->lafBox->addWidget(locationEdit, 0, 1);

    d->lafBox->addWidget(d->filterLabel, 1, 0);
    d->lafBox->addWidget(filterWidget, 1, 1);

    d->lafBox->setColStretch(0, 1);
    d->lafBox->setColStretch(1, 4);
    d->lafBox->setColStretch(2, 1);

    delete d->myStatusLine;
    d->myStatusLine = 0L;

    // Add the status line
    if ( d->showStatusLine ) {

	d->myStatusLine = new QLabel( d->mainWidget, "StatusBar" );
	updateStatusLine(ops->numDirs(), ops->numFiles());
        d->myStatusLine->adjustSize();
	d->myStatusLine->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	d->myStatusLine->setAlignment( AlignHCenter | AlignVCenter );
	d->myStatusLine->setMinimumSize( d->myStatusLine->sizeHint() );
	d->boxLayout->addWidget( d->myStatusLine, 0 );
	d->myStatusLine->show();
    }

    d->boxLayout->addSpacing(3);
}

KFileDialog::~KFileDialog()
{
    KFileViewItem::cleanup();
    delete bookmarks;
    delete visitedDirs;
    delete ops;
    delete d;
    KConfig *c = KGlobal::config();
    KConfigGroupSaver(c, ConfigGroup);
    QWidget *desk = KApplication::desktop();
    c->writeEntry(QString::fromLatin1("Width%1").arg(desk->width()), width(), true, true);
    c->writeEntry(QString::fromLatin1("Height%1").arg(desk->height()), height(), true, true);
    c->sync();
}

void KFileDialog::filterChanged() // SLOT
{
    ops->setNameFilter(filterWidget->currentFilter());
}

void KFileDialog::locationChanged(const QString& txt)
{
    QString text = txt.stripWhiteSpace();
    QString newText = text.left(locationEdit->cursorPosition() -1);

    KURL url( text );
    // debug("Proto: %s, host: %s, dir: %s", debugString(url.protocol()),
    //       debugString(url.host()), debugString(url.directory());

    // don't mess with malformed urls or remote urls without directory or host
    if ( url.isMalformed() ||
	 ( text.find(QString::fromLatin1("file:/")) != 0 &&
	   ( url.directory().isNull() || url.host().isNull()) ) ) {
        d->completionHack = newText;
        return;
    }


    // the user is backspacing -> don't annoy him with completions
    if ( d->completionHack.find( newText ) == 0 ) {
        // but we can follow the directories...

	// find out the current directory according to locationEdit and cd into
	int l = text.length() - 1;
	while (!text.isEmpty() && text[l] != '/')
	    l--;

	QString newLocation = text.left(l+1);

	if ( newLocation != ops->url() ) {
	    setURL(text.left(l), true);
	    locationEdit->setEditText(text);
	}
	d->filename_ = text; // FIXME: dunno about this one...
    }

    // typing forward, ending with a / -> cd into the directory
    else if ( text.at(text.length()-1) == '/' && ops->url() != text ) {
	d->selection = QString::null;
        setURL( text, false );
    }

    // typing forward - do completion
    else {
        if ( KGlobal::completionMode() == KGlobal::CompletionAuto )
	    completion();
    }

    d->completionHack = newText;
}

void KFileDialog::checkPath(const QString&_txt, bool takeFiles) // SLOT
{
    // copy the argument in a temporary string
    // it's unlikely to happen, that at the beginning are spaces, but
    // for the end, it happens quite often, I guess.
    QString text = _txt.stripWhiteSpace();
    // if the argument is no URL (the check is quite fragil) and it's
    // no absolute path, we add the current directory to get a correct
    // url
    if (text.find(':') < 0 && text[0] != '/')
	text.insert(0, ops->url());

    // in case we have a selection defined and someone patched the file-
    // name, we check, if the end of the new name is changed.
    if (!d->selection.isNull()) {
	int position = text.findRev('/');
	ASSERT(position >= 0); // we already inserted the current dir in case
	QString filename = text.mid(position + 1, text.length());
	if (filename != d->selection)
	    d->selection = QString::null;
    }

    KURL u(text); // I have to take care of entered URLs
    bool filenameEntered = false;

    if (u.isLocalFile()) {
	
	// KFileViewItem i(QString::fromLatin1(""), u.url());
	KFileViewItem i(QString::fromLatin1("file:"), u.path());
	if (i.isDir())
	    setURL(text, true);
	else {
	    if (takeFiles)
		if (d->acceptOnlyExisting && !i.isFile())
		    warning("you entered an invalid URL");
		else
		    filenameEntered = true;
	}
    } else
	setURL(text, true);

    if (filenameEntered) {
	d->filename_ = u.url();
	emit fileSelected(d->filename_);

	accept();
    }
}

void KFileDialog::setURL(const QString& _pathstr, bool clearforward)
{
    ops->setURL(_pathstr, clearforward);
}

// Protected
void KFileDialog::urlEntered(const KURL& url)
{
    debugC("urlEntered %s", debugString(url.url()));
    toolbar->getCombo(PATH_COMBO)->clear();

    QStringList list;
    list.append(i18n("Root Directory (%1:)").arg(url.protocol()));

    const QString urlstr = url.url(1);

    debugC(debugString(urlstr));

    int pos = urlstr.find('/', 0); // getting past the protocol

    while( pos >= 0 ) {
	int newpos = urlstr.find('/', pos + 1);
	if (newpos < 0)
	    break;
	
	QString tmp = urlstr.mid(pos + 1, newpos - pos - 1);
	list.append(tmp);
	pos = newpos;
    }
    toolbar->getCombo(PATH_COMBO)->insertStringList(list);

    // add item to visitedDirs.
    if( !visitedDirs->contains(urlstr) ) {
	visitedDirs->append(urlstr);
	visitedDirs->sort();
    }

    // find index.
    int index = 0;

    for(QStringList::ConstIterator dir = visitedDirs->begin();
	dir != visitedDirs->end(); dir++, index++)
      if( *dir == urlstr )
	break;


    // setting locationEdit widget without telling locationChanged()
    disconnect( locationEdit, SIGNAL(textChanged(const QString&)),
		this, SLOT( locationChanged(const QString&) ));

    locationEdit->clear();
    locationEdit->insertStringList(*visitedDirs);
    locationEdit->setCurrentItem( index );

    if (!d->selection.isNull()) {
        QString tmp = urlstr + d->selection;
	if ( locationEdit->currentText() != tmp )
	    locationEdit->setEditText(tmp);
    }

    connect( locationEdit, SIGNAL(textChanged(const QString&)),
	     this, SLOT( locationChanged(const QString&) ));
}

void KFileDialog::comboActivated(int)
{
    debug("comboActivated");
}

void KFileDialog::addToBookmarks() // SLOT
{
    debugC("Add to bookmarks called");
    bookmarks->add(ops->url(), ops->url());
    bookmarks->write();
}

void KFileDialog::bookmarksChanged() // SLOT
{
    debugC("Bookmarks changed called");
}

void KFileDialog::fillBookmarkMenu( KFileBookmark *parent, QPopupMenu *menu, int &id )
{
    KFileBookmark *bm;

    for ( bm = parent->getChildren().first(); bm != NULL;
	  bm = parent->getChildren().next() )
    {
	if ( bm->getType() == KFileBookmark::URL )
	{
	    menu->insertItem( bm->getText(), id );
	    id++;
	}
	else
	{
	    QPopupMenu *subMenu = new QPopupMenu;
	    menu->insertItem( bm->getText(), subMenu );
	    fillBookmarkMenu( bm, subMenu, id );
	}
    }
}

void KFileDialog::toolbarCallback(int i) // SLOT
{
    KConfig *c= KGlobal::config();
    KConfigGroupSaver cs(c, ConfigGroup);
    QString cmd;
    switch(i) {
    case CONFIGURE_BUTTON: {
	KFileDialogConfigureDlg conf(this, "filedlgconf");
	if (conf.exec() == QDialog::Accepted) {
	
	    delete d->boxLayout; // this removes all child layouts too
	    d->boxLayout = 0;
	
	    d->showStatusLine =
		c->readBoolEntry(QString::fromLatin1("ShowStatusLine"), DefaultShowStatusLine);
	    debug("showStatusLine %d", d->showStatusLine);

	    initGUI(); // add them back to the layout managment
	}
	break;
    }
    default:
	warning("KFileDialog: Unknown toolbar button  (id number %d) pressed\n", i);
    }
}



void KFileDialog::toolbarPressedCallback(int i)
{
    int id= idStart;
    if (i == HOTLIST_BUTTON) {
	// Build the menu on first use
	if (bookmarksMenu == 0) {
	    bookmarksMenu= new QPopupMenu;
	    bookmarksMenu->insertItem(i18n("Add to bookmarks"), this,
				      SLOT(addToBookmarks()));
	    bookmarksMenu->insertSeparator();
	    fillBookmarkMenu( bookmarks->getRoot(), bookmarksMenu, id );
	}

	QPoint p;
	KToolBarButton *btn= toolbar->getButton(HOTLIST_BUTTON);
	p= btn->mapToGlobal(QPoint(0, btn->height()));
	bookmarksMenu->move(p);

	int choice= bookmarksMenu->exec();
	QEvent ev(QEvent::Leave);
	QMouseEvent mev (QEvent::MouseButtonRelease,
			 QCursor::pos(), LeftButton, LeftButton);
	QApplication::sendEvent(btn, &ev);
	QApplication::sendEvent(btn, &mev);

	if (choice == 0) {
	    // add current to bookmarks
	    addToBookmarks();
	}
	else if (choice > 0) {
	    // Select a bookmark (this will not work if there are submenus)
	    int i= 1;
	    fprintf(stderr, "choice was %d\n", choice);
	    KFileBookmark *root= bookmarks->getRoot();
	    for (KFileBookmark *b= root->getChildren().first();
		 b != 0; b= root->getChildren().next()) {
		if (i == choice) {
		    fprintf(stderr, "opening bookmark to %s\n",
                                    debugString(b->getURL()));
		    setURL(b->getURL(), true);
		}
		i++;
	    }
	}

	delete bookmarksMenu;
	bookmarksMenu= 0;
    }
}

void KFileDialog::setSelection(const QString& name)
{
    debugC("setSelection %s", debugString(name));

    if (!name) {
	d->selection = QString::null;
	return;
    }

    KURL u(name);
    if (u.isMalformed()) // perhaps we have a relative path!?
	u = ops->url() + name;
    if (u.isMalformed()) { // if it still is
	warning("%s is not a correct argument for setSelection!", debugString(name));
	return;
    }

    if (!u.isLocalFile()) { // no way to detect, if we have a directory!?
	d->filename_ = u.url();
	return;
    }

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KFileViewItem i(QString::fromLatin1("file:"), u.path());
    //    KFileViewItem i(u.path());
    if (i.isDir())
	setURL(u.url(), true);
    else {
	QString filename = u.url();
	int sep = filename.findRev('/');
	if (sep >= 0) { // there is a / in it
	    setURL(filename.left(sep), true);
	    filename = filename.mid(sep+1, filename.length() - sep);
	    debugC("filename %s", debugString(filename));
	    d->selection = filename;
	}
	d->filename_ = ops->url() + filename;
	locationEdit->setEditText(d->filename_);
    }
}


void KFileDialog::completion() // SLOT
{
    QString base = ops->url();

    // if someone uses completion, he doesn't like the current
    // selection
    d->selection = QString::null;

    QString text = locationEdit->currentText();

    if ( KURL(text).isMalformed() )
	return;                         // invalid entry in location

    if (text.left(base.length()) == base) {
	QString complete =
	      ops->makeCompletion( text.right(text.length() - base.length()));

	if (!complete.isNull()) {
	    debugC("Complete %s", debugString(complete));
	    disconnect( locationEdit, SIGNAL(textChanged(const QString&)),
			this, SLOT( locationChanged(const QString&) ));
	
	    QString newText = base + complete;
	    locationEdit->setCompletion( newText );
	    d->filename_ = newText;
	
	    connect( locationEdit, SIGNAL( textChanged( const QString& )),
		     this, SLOT( locationChanged( const QString& ) ));
	} else {
	    warning("no complete");
	}
    }
}


void KFileDialog::updateStatusLine(int dirs, int files)
{
    if (!d->myStatusLine)
	return;

    QString lStatusText;
    QString lFileText, lDirText;

    if ( dirs == 1 )
	lDirText = i18n("directory");
    else
	lDirText = i18n("directories");

    if ( files == 1 )
        lFileText = i18n("file");
    else
	lFileText = i18n("files");

    if (dirs != 0 && files != 0) {
	lStatusText = i18n("%1 %2 and %3 %4")
	    .arg(dirs).arg( lDirText )
	    .arg(files).arg( lFileText );
    } else if ( dirs == 0 )
	lStatusText = i18n("%1 %2").arg(files).arg( lFileText );
    else
	lStatusText = i18n("%1 %2").arg(dirs).arg( lDirText );

    d->myStatusLine->setText(lStatusText);
}


QString KFileDialog::getOpenFileName(const QString& dir, const QString& filter,
				     QWidget *parent,
				     const QString& caption)
{
    QString filename;
    KFileDialog *dlg= new KFileDialog(dir, filter, parent, "filedialog", true);

    dlg->setCaption(caption.isNull() ? i18n("Open") : caption);

    if (dlg->exec() == QDialog::Accepted){
      filename = dlg->selectedURL().path();
      KRecentDocument::add(filename, false);
    }
    delete dlg;

    return filename;
}

QString KFileDialog::getExistingDirectory(const QString& dir,
					  QWidget *parent,
					  const QString& caption)
{
    QString filename;
    KFileDialog dlg(dir, QString::null, parent, "filedialog", true);
    dlg.setMode(Directory);

    dlg.setCaption(caption.isNull() ? i18n("Select Directory") : caption);

    if ( dlg.exec() == QDialog::Accepted ) {
	filename = dlg.selectedURL().path();
	KRecentDocument::add(filename, false);
    }

    return filename;
}

KURL KFileDialog::selectedURL() const
{
    if ( result() == QDialog::Accepted )
	return d->filename_;
    else
	return KURL();
}

QString KFileDialog::getSaveFileName(const QString& dir, const QString& filter,
				     QWidget *parent,
				     const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);

    QString filename;

    if (dlg.exec() == QDialog::Accepted){
        filename= dlg.selectedURL().path();
        KRecentDocument::add(filename, false);
    }

    return filename;
}

void KFileDialog::show()
{
    QDialog::show();
}

void KFileDialog::setMode( Mode m )
{
    ops->setMode(m);
}

KFileDialog::Mode KFileDialog::mode() const
{
    return ops->mode();
}


void KFileComboBox::setCompletion(const QString& completion)
{
    int pos = edit->cursorPosition();

    if ( edit->text() != completion )
	edit->setText( completion );

    if ( KGlobal::completionMode() == KGlobal::CompletionAuto ) {
        edit->setSelection( pos, edit->text().length() );
	edit->setCursorPosition( pos );
    }
    else 	
      	edit->setCursorPosition( completion.length() );

}

bool KFileComboBox::eventFilter( QObject *o, QEvent *ev )
{
    if ( o == edit && ev->type() == QEvent::KeyPress ) {
        QKeyEvent *e = (QKeyEvent *) ev;
	if ( edit->cursorPosition() == (int) edit->text().length() ) {
	    if ( e->key() == Qt::Key_End || e->key() == Qt::Key_Right ) {
	        if ( KGlobal::completionMode() == KGlobal::CompletionEOL ) {
		    edit->deselect();
		    emit completion();
		    return true; // don't pass the event any further
		}
	    }
	}
    }

    return false;
}



#include "kfiledialog.moc"

