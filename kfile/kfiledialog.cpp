// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
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
#include <stdlib.h>
#include <stdio.h>

#include <qaccel.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qcollection.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qstack.h>
#include <qstrlist.h>
#include <qtabdialog.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qtimer.h>

#include <kapp.h>
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>

#include "config-kfile.h"

#include <kdircombobox.h>
#include <kfileview.h>
#include <krecentdocument.h>
#include <kfiledialogconf.h>
#include <kfiledialog.h>
#include <kfileiconview.h>
#include <kfiledetailview.h>
#include <kcombiview.h>
#include <kfilefilter.h>
#include <kfilebookmark.h>
#include <kdiroperator.h>

enum Buttons { HOTLIST_BUTTON,
	       PATH_COMBO, CONFIGURE_BUTTON };

const int idStart = 1;

struct KFileDialogPrivate
{
    // the last selected url
    KURL url;

    // the selected filenames in multiselection mode -- FIXME
    QString filenames;

    // the name of the filename set by setSelection
    QString selection;

    // we need this to determine what has changed in the location bar
    QString completionHack;

    // indicates, if the status bar should be shown.
    // Initialized by "ShowStatusLine"
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
    KDirComboBox *pathCombo;

    KIO::StatJob *statjob;

    // an indicator that we're currently in a completion operation
    // we need to lock some slots for this
    bool completionLock;
};

KURL *KFileDialog::lastDirectory; // to set the start path

KFileDialog::KFileDialog(const QString& dirName, const QString& filter,
			 QWidget *parent, const char* name, bool modal)
    : KDialogBase( parent, name, modal, QString::null,
		   KDialogBase::Ok | KDialogBase::Cancel,
		   KDialogBase::Ok )
{
    d = new KFileDialogPrivate();
    d->boxLayout = 0;
    d->mainWidget = new QWidget( this, "KFileDialog::mainWidget");
    setMainWidget( d->mainWidget );

    d->completionLock = false;
    d->myStatusLine = 0;
    KDirComboBox *combo = new KDirComboBox( this, "path combo" );
    connect( combo, SIGNAL( urlActivated( const QString&  )),
	     this,  SLOT( comboActivated( const QString& )));
    QToolTip::add( combo, i18n("Often used directories") );
    d->pathCombo = combo;

    // I hard code this for now
    d->acceptOnlyExisting = false;
    bookmarksMenu = 0L;

    if (!lastDirectory)
    {
	qAddPostRoutine( cleanup );
	lastDirectory = new KURL();
    }

	static QString dot = QString::fromLatin1(".");
    if (!dirName.isEmpty())
		if ( dirName == dot)
			*lastDirectory = QDir::currentDirPath();
		else
			*lastDirectory = dirName;
    else if (lastDirectory->isEmpty())
        *lastDirectory = QDir::currentDirPath();

    // we remember the selected name for init()
    d->url = *lastDirectory;

    ops = new KDirOperator(*lastDirectory, d->mainWidget, "KFileDialog::ops");
    connect(ops, SIGNAL(updateInformation(int, int)),
	    SLOT(updateStatusLine(int, int)));
    connect(ops, SIGNAL(urlEntered(const KURL&)),
	    SLOT(urlEntered(const KURL&)));
    connect(ops, SIGNAL(fileHighlighted(const KFileViewItem *)),
	    SLOT(fileHighlighted(const KFileViewItem *)));
    connect(ops, SIGNAL(fileSelected(const KFileViewItem *)),
	    SLOT(fileSelected(const KFileViewItem *)));

    visitedDirs = new QStringList();

    toolbar= new KToolBar( d->mainWidget, "KFileDialog::toolbar");
    QActionCollection *coll = ops->actionCollection();
    coll->action( "up" )->plug( toolbar );
    coll->action( "back" )->plug( toolbar );
    coll->action( "forward" )->plug( toolbar );
    coll->action( "home" )->plug( toolbar );
    coll->action( "reload" )->plug( toolbar );


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
    // for the bookmark "menu"
    connect(toolbar, SIGNAL(pressed(int)),
    	    this, SLOT(toolbarPressedCallback(int)));

    toolbar->insertWidget(PATH_COMBO, 30, d->pathCombo);

    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->enableMoving(false);
    toolbar->adjustSize();

    locationEdit = new KFileComboBox(true, d->mainWidget, "LocationEdit");
    locationEdit->setInsertionPolicy(QComboBox::NoInsertion);
    locationEdit->setFocus();

    d->locationLabel = new QLabel(locationEdit, i18n("&Location:"), d->mainWidget);
    d->locationLabel->adjustSize();
    locationEdit->adjustSize();
    d->locationLabel->setMinimumSize(d->locationLabel->width(),
				     locationEdit->height());
    locationEdit->setFixedHeight(d->locationLabel->height());

    connect(locationEdit, SIGNAL(textChanged(const QString&)),
	    SLOT(locationChanged(const QString&)));
    connect(locationEdit, SIGNAL(completion()), SLOT(completion()));
    connect(locationEdit, SIGNAL( previous()),
    	    &(ops->myCompletion), SLOT( slotPreviousMatch() ) );
    connect(locationEdit, SIGNAL( next()),
    	    &(ops->myCompletion), SLOT( slotNextMatch() ) );

    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    d->filterLabel->adjustSize();
    d->filterLabel->setMinimumWidth(d->filterLabel->width());

    filterWidget = new KFileFilter(d->mainWidget, "KFileDialog::filterwidget");
    filterWidget->setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    filterWidget->adjustSize();
    filterWidget->setMinimumWidth(100);
    filterWidget->setFixedHeight(filterWidget->height());
    connect(filterWidget, SIGNAL(filterChanged()),
	    SLOT(filterChanged()));
    ops->setNameFilter(filterWidget->currentFilter());

    connect( locationEdit, SIGNAL( returnPressed() ),
    	     SLOT( slotOk()));

    // Get the config object
    KSimpleConfig *kc = new KSimpleConfig(QString::fromLatin1("kdeglobals"),
                                          false);
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );
    d->showStatusLine = kc->readBoolEntry(ConfigShowStatusLine,
					  DefaultShowStatusLine);

    initGUI(); // activate GM

    if (!d->url.isEmpty()) {
	checkPath(d->url.url());
	locationEdit->setEditText(d->url.url());
    }
    adjustSize();
    readConfig( kc, ConfigGroup );
    delete kc;

    if (d->url.isEmpty())
	d->url = QDir::currentDirPath();

    // filename is remembered as the dirName argument for the constructor
    // FIXME:
    // set the view _after_ calling setSelection(), otherwise we would read
    // the startdirectory twice. This must be fixed somehow else, tho.
    setSelection(d->url.url());
    ops->setView(KFile::Default);
}

void KFileDialog::setLocationLabel(const QString& text)
{
    d->locationLabel->setText(text);
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

void KFileDialog::setPreviewWidget(const QWidget *w) {
    ops->setPreviewWidget(w);
}


// FIXME: check for "existing" flag here?
void KFileDialog::slotOk()
{
    kdDebug(kfile_area) << "slotOK\n";

    if ( locationEdit->currentText().stripWhiteSpace().isEmpty() )
        return;

    d->url = locationEdit->currentText();

    if ( (mode() & KFile::Files) == KFile::Files ) {
	kdDebug(kfile_area) << "Files\n";
	d->filenames = locationEdit->currentText(); // FIXME
	accept();
	return;
    }

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
	kdDebug(kfile_area) << "Directory\n";
	if ( QFileInfo(d->url.path()).isDir() )
	    accept();
	return;
    }

    d->statjob = KIO::stat(d->url);
    connect(d->statjob, SIGNAL(result(KIO::Job*)),
	    SLOT(slotStatResult(KIO::Job*)));
}

void KFileDialog::slotStatResult(KIO::Job* job) {
    kdDebug(kfile_area) << "slotStatResult" << endl;

    if (d->statjob != job)
       return; // something is really weird here

    // errors mean in general, the location is no directory ;/
    if (job->error())
       accept();

    KIO::UDSEntry t = d->statjob->statResult();
    bool dir = false;
    for (KIO::UDSEntry::ConstIterator it = t.begin();
        it != t.end(); ++it) {
       if ((*it).m_uds == KIO::UDS_FILE_TYPE ) {
           dir = S_ISDIR( (mode_t)((*it).m_long));
           break;
       }
    }
    if (dir) {
       setURL( d->statjob->url() );
       d->statjob = 0;
       return;
    }

    d->statjob = 0;
    kdDebug(kfile_area) << "filename " << d->url.url() << endl;

    accept();
}

	
void KFileDialog::accept()
{
    *lastDirectory = ops->url();
    KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"),
                                         false);
    saveConfig( c, ConfigGroup );
    delete c;

    KDialogBase::accept();
}


void KFileDialog::fileHighlighted(const KFileViewItem *i)
{
  debug("fileHighlighted");
    if (i->isDir())
        return;

    d->url = i->url();
    if ( (ops->mode() & KFile::Files) == KFile::Files )
	multiSelectionChanged( i );
    else
	if ( !d->completionLock )
	    locationEdit->setEditText(d->url.url());
    emit fileHighlighted(d->url.url());
}

void KFileDialog::fileSelected(const KFileViewItem *i)
{
  debug("fileSelected");
    if (i->isDir())
        return;
    d->url = i->url();
    if ( (ops->mode() & KFile::Files) == KFile::Files )
	multiSelectionChanged( i );
    else
	locationEdit->setEditText(d->url.url());

    emit fileSelected(d->url.url());
}


// I know it's slow to always iterate thru the whole filelist
// (ops->selectedItems()), but what can we do?
void KFileDialog::multiSelectionChanged(const KFileViewItem *)
{
    KFileViewItem *item;
    const KFileViewItemList *list = ops->selectedItems();
    if ( !list ) {
	locationEdit->clearEdit();
	return;
    }

    static QString begin = QString::fromLatin1(" \"");
    KFileViewItemListIterator it ( *list );
    QString text;
    while ( (item = it.current()) ) {
	text.append( begin ).append( item->name() ).append( '\"' );
	++it;
    }
    locationEdit->setEditText( text.stripWhiteSpace() );
}


void KFileDialog::returnPressed()
{
    if ( locationEdit->currentText().stripWhiteSpace().isEmpty() )
        return; // FIXME: show error box?

    slotOk();
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
    hide();
    delete bookmarks;
    delete visitedDirs;
    delete d->boxLayout; // we can't delete a widget being managed by a layout,
    d->boxLayout = 0;    // so we delete the layout before
    delete ops;
    delete d;
}

void KFileDialog::filterChanged() // SLOT
{
    ops->setNameFilter(filterWidget->currentFilter());
}

void KFileDialog::locationChanged(const QString& txt)
{
    if ( d->completionLock )
	return;
    
    // no completion and directory following in multiselection mode!
    if ( (ops->mode() & KFile::Files) == KFile::Files )
	return;

    QString text = txt; //.stripWhiteSpace();
    QString newText = text.left(locationEdit->cursorPosition() -1);
    if ( text.at( 0 ) == '/' )
	text.prepend(QString::fromLatin1("file:"));

    KURL url( text );

    // don't mess with malformed urls or remote urls without directory or host
    if ( url.isMalformed() ||
	 ( text.find(QString::fromLatin1("file:/")) != 0 &&
	   ( url.directory().isNull() || url.host().isNull()) ) ) {
        d->completionHack = newText;
        return;
    }

    // the user is backspacing -> don't annoy him with completions
    if ( autoDirectoryFollowing && d->completionHack.find( newText ) == 0 ) {
        // but we can follow the directories, if configured so

	// find out the current directory according to locationEdit and cd into
	int l = text.length() - 1;
	while (!text.isEmpty() && text[l] != '/')
	    l--;

	KURL newLocation(text.left(l+1));

	if ( !newLocation.isMalformed() && newLocation != ops->url() ) {
	    setURL(text.left(l), true);
	    locationEdit->setEditText(text);
	}
	d->url = text; // FIXME: dunno about this one...
    }

    // typing forward, ending with a / -> cd into the directory
    else if ( autoDirectoryFollowing &&
	      text.at(text.length()-1) == '/' && ops->url() != text ) {
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
	text.insert(0, ops->url().url());

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
	d->url = u.url();
	emit fileSelected(d->url.url());

	accept();
    }
}

#if 0
void KFileDialog::setURL(const QString& _pathstr, bool clearforward)
{
    ops->setURL(_pathstr, clearforward);
}
#endif

void KFileDialog::setURL(const KURL& url, bool clearforward)
{
    ops->setURL( url, clearforward);
}

// Protected
void KFileDialog::urlEntered(const KURL& url)
{
    if ( d->pathCombo->count() != 0 ) // little hack
	d->pathCombo->setURL( url );
    const QString urlstr = url.url(1);

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

void KFileDialog::comboActivated( const QString& url)
{
    kDebugInfo(kfile_area, "comboActivated");
    setURL( url );
}

void KFileDialog::addToBookmarks() // SLOT
{
    kDebugInfo(kfile_area, "Add to bookmarks called");
    bookmarks->add(ops->url().url(), ops->url().url());
    bookmarks->write();
}

void KFileDialog::bookmarksChanged() // SLOT
{
    kDebugInfo(kfile_area, "Bookmarks changed called");
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
    if (i == CONFIGURE_BUTTON) {
	KFileDialogConfigureDlg conf(this, "filedlgconf");
	if (conf.exec() == QDialog::Accepted) {
	
	    KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"),
						 false);
	    c->setGroup( ConfigGroup );
	
	    delete d->boxLayout; // this removes all child layouts too
	    d->boxLayout = 0;
	
	    d->showStatusLine =
		c->readBoolEntry(QString::fromLatin1("ShowStatusLine"), DefaultShowStatusLine);
	    delete c;
	    kdDebug(kfile_area) << "showStatusLine " << d->showStatusLine
				<< endl;

	    initGUI(); // add them back to the layout managment
	}
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

void KFileDialog::setSelection(const QString& url)
{
    kdDebug(kfile_area) << "setSelection " << url << endl;

    if (url.isEmpty()) {
	d->selection = QString::null;
	return;
    }

    KURL u(url);
    if (u.isMalformed()) // perhaps we have a relative path!?
	u = KURL(ops->url(),  url);

    if (u.isMalformed()) { // if it still is
        warning("%s is not a correct argument for setSelection!", debugString(url));
	return;
    }

    if (!u.isLocalFile()) { // no way to detect, if we have a directory!?
	d->url = u;
	return;
    }

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KFileViewItem i(QString::fromLatin1("file:"), u.path());
    //    KFileViewItem i(u.path());
    if (i.isDir())
	setURL(u, true);
    else {
	QString filename = u.url();
	int sep = filename.findRev('/');
	if (sep >= 0) { // there is a / in it
	    setURL(filename.left(sep), true);
	    filename = filename.mid(sep+1, filename.length() - sep);
	    kdDebug(kfile_area) << "filename " << filename << endl;
	    d->selection = filename;
	}
	d->url = KURL(ops->url(), filename); // FIXME make filename an url
	locationEdit->setEditText(d->url.url());
    }
}

void KFileDialog::completion() // SLOT
{
    QString base = ops->url().url();

    // if someone uses completion, he doesn't like the current selection
    d->selection = QString::null;

    QString text = locationEdit->currentText();
    if ( text.at( 0 ) == '/' )
	text.prepend( QString::fromLatin1("file:") );

    if ( KURL(text).isMalformed() )
	return;                         // invalid entry in location
    
    d->completionLock = true;

    if (text.left(base.length()) == base) {
	QString complete =
	      ops->makeCompletion( text.right(text.length() - base.length()));

	if (!complete.isNull()) {
	    kdDebug(kfile_area) << "Complete " << complete << endl;
	
	    QString newText = base + complete;
	    locationEdit->setCompletion( newText );
	    d->url = newText;
	}
    }
    d->completionLock = false;
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

#if 0
QString KFileDialog::getOpenFileName(const QString& dir, const QString& filter,
				     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename, false);

    return filename;
}

QStringList KFileDialog::getOpenFileNames(const QString& dir,const QString& filter,
				     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    QStringList list = dlg.selectedFiles();
    QStringList::Iterator it = list.begin();
    for( ; it != list.end(); ++it )
        KRecentDocument::add( *it, false );

    return list;
}
#endif

KURL KFileDialog::getOpenURL(const QString& dir, const QString& filter,
				QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    const KURL& url = dlg.selectedURL();
    if (!url.isMalformed())
        KRecentDocument::add(url.url(-1), false);

    return url;
}

KURL::List KFileDialog::getOpenURLs(const QString& dir,
					  const QString& filter,
					  QWidget *parent,
					  const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.exec();

    KURL::List list = dlg.selectedURLs();
    KURL::List::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it ) {
	if ( !(*it).isMalformed() )
	    KRecentDocument::add( (*it).url(-1), false );
    }

    return list;
}

QString KFileDialog::getExistingDirectory(const QString& dir,
					  QWidget *parent,
					  const QString& caption)
{
    KFileDialog dlg(dir, QString::null, parent, "filedialog", true);
    dlg.setMode(KFile::Directory);

    dlg.setCaption(caption.isNull() ? i18n("Select Directory") : caption);
    dlg.exec();

    return dlg.selectedURL().path();
}

KURL KFileDialog::selectedURL() const
{
    if ( result() == QDialog::Accepted )
	return d->url;
    else
	return KURL();
}

KURL::List KFileDialog::selectedURLs() const
{
    KURL::List list;
    if ( result() == QDialog::Accepted ) {
	if ( (ops->mode() & KFile::Files) == KFile::Files ) {
	    QString name;
	    QString url = ops->url().url( +1 );
	    // FIXME: check for local files?

	    static QRegExp r( QString::fromLatin1( "\"" ) );
	    static QString empty = QString::fromLatin1("");
	    QTextStream t( &(d->filenames), IO_ReadOnly );
	    while ( !t.eof() ) {
		t >> name;
		name.replace( r, empty );
		name.prepend( url );
		list.append( KURL( name ) );
	    }
	}
	else
	    list.append( d->url );
    }
    return list;
}


KURL KFileDialog::baseURL() const
{
    return ops->url();
}

#if 0
QString KFileDialog::selectedFile() const
{
    if ( result() == QDialog::Accepted )
    {
       KURL url( d->filename );
       if (url.isLocalFile())
          return url.path();
    }
    return QString::null;
}

QStringList KFileDialog::selectedFiles() const
{
    QStringList list;

    if ( result() == QDialog::Accepted ) {
	QString name;
	QString url = ops->url().path( +1 );
	// FIXME: check for local files?

	static QRegExp r( QString::fromLatin1( "\"" ) );
	static QString empty = QString::fromLatin1("");
	QTextStream t( &(d->filename), IO_ReadOnly );
	while ( !t.eof() ) {
	    t >> name;
	    name.replace( r, empty );
	    name.prepend( url );
	    list.append( name );
	}
    }
    return list;
}

QString KFileDialog::getSaveFileName(const QString& dir, const QString& filter,
				     QWidget *parent,
				     const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename, false);

    return filename;
}
#endif

KURL KFileDialog::getSaveURL(const QString& dir, const QString& filter,
				     QWidget *parent,
				     const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);

    dlg.exec();

    KURL url = dlg.selectedURL();
    if (!url.isMalformed())
        KRecentDocument::add(url.url(-1), false);

    return url;
}

void KFileDialog::show()
{
    QDialog::show();
}

void KFileDialog::setMode( KFile::Mode m )
{
    ops->setMode(m);
}

KFile::Mode KFileDialog::mode() const
{
    return ops->mode();
}


void KFileDialog::readConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
	return;

    QString oldGroup = kc->group();
    if ( !group.isEmpty() )
	kc->setGroup( group );

    ops->readConfig( kc, group );

    KDirComboBox *combo = d->pathCombo;
    combo->setMaxItems( kc->readNumEntry( RecentURLsNumber,
					  DefaultRecentURLsNumber ) );
    combo->setURLs( kc->readListEntry( RecentURLs ) );
    combo->setURL( ops->url() );
    autoDirectoryFollowing = kc->readBoolEntry( AutoDirectoryFollowing,
						DefaultDirectoryFollowing );


    int w, h;
    QWidget *desk = QApplication::desktop();
    w = QMIN( 530, (int) (desk->width() * 0.5)); // maximum default width = 530
    h = (int) (desk->height() * 0.4);

    w = kc->readNumEntry( DialogWidth.arg( desk->width()), w );
    h = kc->readNumEntry( DialogHeight.arg( desk->height()), h );

    int w1 = minimumSize().width();
    int w2 = toolbar->sizeHint().width() + 10;
    if (w1 < w2)
	setMinimumWidth(w2);

    resize(w, h);

    kc->setGroup( oldGroup );
}

void KFileDialog::saveConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
	return;

    QString oldGroup = kc->group();
    if ( !group.isEmpty() )
	kc->setGroup( group );

    QWidget *desk = kapp->desktop();
    kc->writeEntry( RecentURLs, d->pathCombo->urls() );
    kc->writeEntry( RecentURLsNumber, d->pathCombo->maxItems() );
    kc->writeEntry( DialogWidth.arg( desk->width() ), width() );
    kc->writeEntry( DialogHeight.arg( desk->height() ), height() );

    ops->saveConfig( kc, group );
    kc->setGroup( group );
    kc->sync();
}


///////////////////


void KFileComboBox::setCompletion(const QString& completion)
{
    int pos = edit->cursorPosition();

    if ( edit->text() != completion )
	edit->setText( completion );

    if ( KGlobal::completionMode() == KGlobal::CompletionAuto ||
	 KGlobal::completionMode() == KGlobal::CompletionMan ) {
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

	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter ) {
	    emit returnPressed();
	    return true;
	}
	
	bool ret = false;
	if ( edit->cursorPosition() == (int) edit->text().length() ) {
	    if ( e->key() == Qt::Key_End || e->key() == Qt::Key_Right ) {
	        if ( KGlobal::completionMode() == KGlobal::CompletionShell ) {
		    edit->deselect();
		    emit completion();
		    ret = true; // don't pass the event any further
		}
		else if ( KGlobal::completionMode() == KGlobal::CompletionMan ) {
		    edit->deselect();
		    emit completion();
		    ret = true; // don't pass the event any further
		}
	    }
	}

	if ( e->key() == Qt::Key_Next ) {
	    emit next();
	    ret = true;
	}
	else if ( e->key() == Qt::Key_Prior ) {
	    emit previous();
	    ret = true;
	}

	return ret;
    }

    return QComboBox::eventFilter( o, ev );
}


#include "kfiledialog.moc"
