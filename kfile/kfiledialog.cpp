#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <qtimer.h>
#include <qtabdlg.h>
#include <qstrlist.h>
#include <qstring.h>
#include <qstack.h>
#include <qpushbt.h>
#include <qpopmenu.h>
#include <qpixmap.h>
#include <qobjcoll.h>
#include <qmsgbox.h>
#include <qlistbox.h>
#include <qlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombo.h>
#include <qcolor.h>
#include <qchkbox.h>
#include <qbttngrp.h>
#include <qaccel.h>
#include <kurl.h>
#include <ktoolbar.h>
#include <knewpanner.h>
#include <kiconloader.h>
#include "kfileinfocontents.h"
#include "kfiledialogconf.h"
#include "kfiledialog.h"
#include "kfiledetaillist.h"
#include "kfilesimpleview.h"
#include "kcombiview.h"
#include "kdirlistbox.h"
#include "kdir.h"
#include "bookmark.h"
#include <kprocess.h>
#include <kapp.h>

enum Buttons { BACK_BUTTON= 1000, FORWARD_BUTTON, PARENT_BUTTON, 
	       HOME_BUTTON, RELOAD_BUTTON, HOTLIST_BUTTON, 
	       PATH_COMBO, MKDIR_BUTTON, FIND_BUTTON, CONFIGURE_BUTTON };

enum SortItem { BY_NAME = 1000, BY_SIZE, BY_DATE, BY_OWNER, BY_GROUP };

const int idStart = 1;

KFileBaseDialog::KFileBaseDialog(const char *dirName, const char *filter,
			 QWidget *parent, const char *name, bool modal,
			 bool acceptURLs)
    : QDialog(parent, name, modal), boxLayout(0)
{
    QAccel *a = new QAccel( this );
    a->connectItem(a->insertItem(Key_S + CTRL), this,
		   SLOT(completion()));

    // I hard code this for now
    acceptOnlyExisting = false;
    wrapper= swallower();
    bookmarksMenu= 0;
    acceptUrls = acceptURLs;

    // Create the KDir
    if (dirName != 0) {
	KFileInfo i(dirName);
	if (i.isDir())
	    dir = new KDir(dirName);
	else {
	    debug("I got a filename");
	    QString filename = dirName;
	    int sep = filename.findRev('/');
	    if (sep < 0) {
		dir = new KDir(filename.left(sep));
		filename_ = filename;
	    } else {
		dir = new KDir(); // take current path
		debug("path %s", dir->url().data());
		if (acceptURLs)
		    filename_ = dir->url() + filename;
		else
		    filename_ = dir->path() + filename;
	    }
	   
	}
	    
    } else 
	dir = new KDir();
    
    // For testing
    connect(dir, SIGNAL(dirEntry(KFileInfo *)),
	    SLOT(slotDirEntry(KFileInfo *)));
    connect(dir, SIGNAL(newFilesArrived(const KFileInfoList *)),
	    SLOT(insertNewFiles(const KFileInfoList *)));

    // Init flags
    finished = true;

    filters = new QStrList( true );
    QString tmp = filter; // deep copy
    char *g = strtok(tmp.data(), "\n");
    while (g) {
	filters->append(g);
	g = strtok(0, "\n");
    }
    
    connect(dir, SIGNAL(finished()),
	    SLOT(slotFinished()));
    connect(dir, SIGNAL(error(int, const char *)),
	    SLOT(slotKfmError(int, const char *)));
    
    backStack.setAutoDelete( true );
    forwardStack.setAutoDelete( true );
    
}

void KFileBaseDialog::init() 
{
    // Get the config object
    KConfig *c= kapp->getConfig();
    QString oldgroup= c->group();
    c->setGroup("KFileDialog Settings");
    
    //
    // Read configuration from the config file
    // 
    int w = c->readNumEntry("Width", 400);
    int h = c->readNumEntry("Height", 400);
    
    showHidden = (c->readNumEntry("ShowHidden", 0) != 0);
    showStatusLine = (c->readNumEntry("ShowStatusLine", 1) != 0);
      
    toolbar= new KToolBar(wrapper, "fileDialogToolbar");
    
    // Build the toolbar
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("back.xpm"),
			  BACK_BUTTON, false, 
			  i18n("Back"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("forward.xpm"),
			  FORWARD_BUTTON, false,
			  i18n("Forward"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("up.xpm"),
			  PARENT_BUTTON, true, 
			  i18n("Up to parent"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("home.xpm"),
			  HOME_BUTTON, true, 
			  i18n("Home"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("reload.xpm"),
			  RELOAD_BUTTON, true,
			  i18n("Reload"));
    
    //    c->readListEntry("Bookmarks", bookmarks);
    //    bookmarks.insert(0, i18n("Add to bookmarks"));
    bookmarks= new KBookmarkManager();
    CHECK_PTR( bookmarks );
    connect( bookmarks, SIGNAL( changed() ), 
	     this, SLOT( bookmarksChanged() ) );

    QString bmFile = KApplication::localkdedir() + 
	"/share/apps/kdeui/bookmarks.html";

    bookmarks->read(bmFile);

    toolbar->insertButton(kapp->getIconLoader()->loadIcon("flag.xpm"),
			  HOTLIST_BUTTON, true,
			  i18n("Bookmarks"), -1);
    
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("idea.xpm"),
			  MKDIR_BUTTON, true,
			  i18n("Create new folder"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("viewmag.xpm"),
			  FIND_BUTTON, true,
			  i18n("Search for a file"));
    toolbar->insertButton(kapp->getIconLoader()->loadIcon("configure.xpm"),
			  CONFIGURE_BUTTON, true,
			  i18n("Configure this dialog [development only]"));
    
    connect(toolbar, SIGNAL(clicked(int)),
	    SLOT(toolbarCallback(int)));
    // try this again
    connect(toolbar, SIGNAL(pressed(int)),
    	    this, SLOT(toolbarPressedCallback(int)));
    
    toolbar->insertCombo(i18n("The path"), PATH_COMBO, false, 
			 SIGNAL(activated(int)),
			 this, SLOT(comboActivated(int)),
			 true, "Current Path");
    
    toolbar->setFullWidth(true);
    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->enableMoving(false);
    toolbar->adjustSize();
    setMinimumWidth(toolbar->width());
    
    // Create subclass children
    initUpperChildren();
    
    if ( c->readNumEntry("KeepDirsFirst", 1) )
        dir->setSorting( QDir::Name | QDir::DirsFirst);
    else
        dir->setSorting( QDir::Name );

    showFilter = getShowFilter();

    fileList = initFileList( wrapper );
    
    locationEdit = new KCombo(true, wrapper, "locationedit");
    locationEdit->setInsertionPolicy(QComboBox::NoInsertion);
    locationEdit->setFocus();

    locationLabel = new QLabel(locationEdit, i18n("&Location:"), wrapper);
    locationLabel->adjustSize();
    locationEdit->adjustSize();
    locationLabel->setMinimumSize(locationLabel->width(), 
				  locationEdit->height());
    locationEdit->setMinimumHeight(locationLabel->height() + 8);
    
    connect(locationEdit, SIGNAL(activated(const char*)),
	    SLOT(locationChanged(const char*)));

    // Add the filter
    if (showFilter) {
	filterLabel = new QLabel(i18n("&Filter:"), wrapper);
	filterLabel->adjustSize();
	filterLabel->setMinimumWidth(filterLabel->width());
	filterLabel->resize(locationLabel->width(), filterLabel->height());

	if (filters->count() == 1) {
	    filterEdit = new QLineEdit(wrapper, "filteredit");
	    filterEdit->adjustSize();
	    filterEdit->setMinimumSize(100,filterLabel->size().height()+8);
	    filterCombo = 0;
	    connect(filterEdit, SIGNAL(returnPressed()), 
		    SLOT(filterChanged()));
	    filterEdit->setText(filters->first());
	    filterLabel->setBuddy(filterEdit);
	} else {
	    filterCombo = new QComboBox(true, wrapper, "filtercombo");
	    filterEdit = 0;
	    QString name;
	    for (const char *item = filters->first(); item; 
		 item = filters->next()) {
		name = item;
		int tab = name.find('|');
		filterCombo->insertItem((tab < 0) ? name :
					name.mid(tab + 1, name.length() - tab));
	    }
	    filterCombo->adjustSize();
	    filterCombo->setFixedHeight(filterCombo->height());
	    filterCombo->setInsertionPolicy(QComboBox::NoInsertion);

	    connect(filterCombo, SIGNAL(activated(const char*)),
		    SLOT(filterChanged()));
	    filterLabel->setBuddy(filterCombo);
	}
	hiddenToggle= new QCheckBox(i18n("Show hidden"), this, "hiddentoggle");
	hiddenToggle->adjustSize();
	hiddenToggle->setChecked(showHidden);
	hiddenToggle->setMinimumSize(hiddenToggle->size());
	connect(hiddenToggle, SIGNAL(toggled(bool)), 
		SLOT(setHiddenToggle(bool)));
	
    }
    
    initLowerChildren();
    
    // Add the status line
    if ( showStatusLine ) {
	myStatusLine = new QLabel( this, "StatusBar" );
	myStatusLine->setText("Dir and File Info");
        myStatusLine->adjustSize();
	myStatusLine->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	myStatusLine->setAlignment( AlignHCenter | AlignVCenter );
	myStatusLine->setMinimumSize( myStatusLine->sizeHint() );
    } else
	myStatusLine = 0L;
        
    bOk= new QPushButton(i18n("OK"), this, "_ok");
    bOk->adjustSize();
    bOk->setMinimumWidth(bOk->width());
    bOk->setFixedHeight(bOk->height());
    connect(bOk, SIGNAL(clicked()), SLOT(accept()));

    bCancel= new QPushButton(i18n("Cancel"), this, "_cancel");
    bCancel->adjustSize();
    bCancel->setMinimumWidth(bCancel->width());
    bCancel->setFixedHeight(bCancel->height());
    connect(bCancel, SIGNAL(clicked()), SLOT(reject()));
    
    bHelp= new QPushButton(i18n("Help"), this, "_help");
    bHelp->adjustSize();
    bHelp->setMinimumWidth(bHelp->width());
    bHelp->setFixedHeight(bHelp->height());
    connect(bHelp, SIGNAL(clicked()), SLOT(help()));

    // c->setGroup(oldgroup); // reset the group
    initGUI(); // activate GM

    visitedDirs = new QStrIList();
    
    if (showFilter)
       filterChanged();
    else
       pathChanged();

    if (!filename_.isNull()) {
	debug("edit %s", locationEdit->text(0));
	checkPath(filename_);
	locationEdit->setText(filename_);
    }
}

void KFileBaseDialog::initGUI()
{
    if (boxLayout)
	delete boxLayout; // deletes all sub layouts

    boxLayout = new QVBoxLayout(wrapper, 4);
    boxLayout->addSpacing(toolbar->height());
    boxLayout->addWidget(fileList->widget(), 4);
    boxLayout->addSpacing(3);
    
    lafBox= new QGridLayout(showFilter ? 2 : 1, 3, 7);
    boxLayout->addLayout(lafBox, 0);
    lafBox->addWidget(locationLabel, 0, 0);
    lafBox->addMultiCellWidget(locationEdit, 0, 0, 1, 2);

    if (showFilter) {
	lafBox->addWidget(filterLabel, 1, 0);
	lafBox->addWidget(filterEdit ? filterEdit : filterCombo, 1, 1);
	lafBox->addWidget(hiddenToggle, 1, 2);
    }

    lafBox->setColStretch(0, 1);
    lafBox->setColStretch(1, 4);
    lafBox->setColStretch(2, 1);
    
    if ( myStatusLine )
	boxLayout->addWidget( myStatusLine, 0 );
    
    boxLayout->addSpacing(3);
    btngroup= new QHBoxLayout(10);
    boxLayout->addLayout(btngroup, 0);
    btngroup->addWidget(bOk);
    btngroup->addWidget(bCancel);
    btngroup->addWidget(bHelp);
    boxLayout->activate();

    fileList->connectDirSelected(this, SLOT(dirActivated()));
    fileList->connectFileSelected(this, SLOT(fileActivated()));
    fileList->connectFileHighlighted(this, SLOT(fileHighlighted()));
}

KFileBaseDialog::~KFileBaseDialog()
{
    backStack.clear();
    delete bookmarks;
    delete visitedDirs;
    delete dir;
    delete filters;
}

void KFileBaseDialog::help() // SLOT
{
    kapp->invokeHTMLHelp("kfiledialog/index.html", "");
}

bool KFileDialog::getShowFilter() 
{
    return (kapp->getConfig()->readNumEntry("ShowFilter", 1) != 0);
}

void KFileBaseDialog::filterChanged() // SLOT
{
    if (!showFilter)
       return;

    if (filterEdit)
	dir->setNameFilter(filterEdit->text());
    else {
	QString filter = filterCombo->currentText();
	if (filter == filterCombo->text(filterCombo->currentItem()))
	    filter = filters->at(filterCombo->currentItem());

	int tab = filter.find('|');
	if (tab < 0)
	    dir->setNameFilter(filter);
	else
	    dir->setNameFilter(filter.left(tab));
    }

    pathChanged();
}

void KFileBaseDialog::setHiddenToggle(bool b) // SLOT
{
    showHidden = b;
    
    warning("setHiddenToggle(): %s", b ? "true" : "false");
    pathChanged();
}

void KFileBaseDialog::locationChanged(const char *_txt)
{
    bool highlighted = strcmp(_txt, locationEdit->text(0));
    checkPath(_txt, highlighted);
}

void KFileBaseDialog::checkPath(const char *_txt, bool takeFiles) // SLOT
{
    QString text = _txt;
    text = text.stripWhiteSpace();
    if (text.find(':') < 0 && text[0] != '/')
	text.insert(0, dir->url());
    
    KURL u = text.data(); // I have to take care of entered URLs
    bool filenameEntered = false;
    
    if (u.isLocalFile()) {
	// the empty path is kind of a hack
	KFileInfo i("", u.path()); 
	if (i.isDir())
	    setDir(text, true);
	else {
	    if (takeFiles)
		if (acceptOnlyExisting && !i.isFile())
		    warning("you entered an invalid URL");
		else
		    filenameEntered = true;
	}
    } else
	setDir(text, true);

    if (filenameEntered) {
	if (acceptUrls)
	    filename_ = u.url();
	else
	    filename_ = u.path();
	emit fileSelected(filename_);
	accept();
    }
}


QString KFileBaseDialog::selectedFile()
{
    KURL u = filename_.data();
    return u.path();
}

QString KFileBaseDialog::dirPath()
{
    return dir->path();
}

void KFileBaseDialog::setDir(const char *_pathstr, bool clearforward)
{
    QString pathstr = _pathstr;
    
    if (pathstr.right(1)[0] != '/')
	pathstr += "/";
    
    if (pathstr.left(1)[0] == '/')
	pathstr.insert(0, "file:");

    if (dir->url() == pathstr) { // already set
	return;
    } 

    if (clearforward) {
	// autodelete should remove this one
	backStack.push(new QString(dir->url()));
	forwardStack.clear();
    }

    // make a little test to avoid creating of a KURL object
    // in most cases
    if ( !acceptUrls && strchr(_pathstr, ':') ) {
	KURL testURL(pathstr);
	if ( !testURL.isLocalFile() ) {
	    QMessageBox::message(i18n("Error: Not local file"),
				 i18n("The specified directory is not a "
				      "local directory\n"
				      "But the application accepts just "
				      "local files."),
				 i18n("OK"), this);
	    return;
	}
    }
	    
    
    QString backup = dir->url();
    dir->setPath(pathstr);
    
    if (!dir->isReadable()) {
	QMessageBox::message(i18n("Error: Cannot Open Directory"),
			     i18n("The specified directory does not exist\n"
				  "or was not readable."),
			     i18n("Dismiss"),
			     this, "kfiledlgmsg");      
	dir->setPath(backup);
    } else {
	
	emit dirEntered(pathstr);
	
	updateHistory( !forwardStack.isEmpty(), !backStack.isEmpty() );
	
	pathChanged();
    }

    toolbar->getButton(PARENT_BUTTON)->enable(!dir->isRoot());
}

void KFileBaseDialog::rereadDir()
{
    // some would call this dirty. I don't ;)
    dir->setPath(dir->url());
    pathChanged();
}

KFileDialog::KFileDialog(const char *dirName, const char *filter,
				 QWidget *parent, const char *name, 
				 bool modal, bool acceptURLs)
    : KFileBaseDialog(dirName, filter, parent, name, modal, acceptURLs)
{
    init();
}

QString KFileDialog::getOpenFileName(const char *dir, const char *filter,
				     QWidget *parent, const char *name)
{
    QString filename;
    KFileDialog *dlg= new KFileDialog(dir, filter, parent, name, true, false);
    
    dlg->setCaption(i18n("Open"));
    
    if (dlg->exec() == QDialog::Accepted)
	filename = dlg->selectedFile();
    
    delete dlg;
    
    return filename;
}
 
QString KFileDialog::getSaveFileName(const char *dir, const char *filter,
				     QWidget *parent, const char *name)
{
    KFileDialog *dlg= new KFileDialog(dir, filter, parent, name, true, false);
    
    dlg->setCaption(i18n("Save As"));
    
    QString filename;
    
    if (dlg->exec() == QDialog::Accepted)
	filename= dlg->selectedFile();
    
    delete dlg;
    
    return filename;
}

// Protected
void KFileBaseDialog::pathChanged()
{
    // Not forgetting of course the path combo box
    toolbar->clearCombo(PATH_COMBO);
    
    QString path= dir->path();
    QString pos= strtok(path.data(), "/");
    QStrList list;
    
    list.insert(0, i18n("Root Directory"));
    while (!(pos.isNull())) {
	list.insert(0, pos+"/");
	pos= strtok(0, "/");
    }
    toolbar->getCombo(PATH_COMBO)->insertStrList(&list);
    
    fileList->clear();
    
    int filter = QDir::Files | QDir::Dirs;
    if (showHidden)
	filter |= QDir::Hidden;

    // it may be, that we weren't ready at this time
    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = false;
    
    // when kfm emits finished, the slot will restore the cursor
    QApplication::setOverrideCursor( waitCursor );

    const KFileInfoList *il = dir->entryInfoList(filter, 
						 QDir::Name | QDir::IgnoreCase);
    if (!dir->isReadable()) {
	QMessageBox::message(i18n("Error: Cannot Open Directory"),
			     i18n("The specified directory does not exist "
				  "or was not readable."),
			     i18n("Dismiss"),
			     this, "kfiledlgmsg");
	if (backStack.isEmpty())
	    home();
	else
	    back();
    } else {
	QString url;
	
	if (acceptUrls)
	    url = dir->url();
	else
	    url = dir->path();

	if (url.right(1)[0] != '/')
	    url += "/";
	if (visitedDirs->find(url) == -1)
	    visitedDirs->inSort(url);
	
	locationEdit->clear();
	locationEdit->insertStrList(visitedDirs);
	locationEdit->setCurrentItem(visitedDirs->at());

	insertNewFiles(il);
    }
}

void KFileBaseDialog::insertFile(const KFileInfo *entry)
{
    repaint_files |= fileList->addItem(entry);
}

void KFileBaseDialog::insertNewFiles(const KFileInfoList *newone) 
{
    repaint_files = false;

    fileList->setAutoUpdate(false);
    
    KFileInfoListIterator it(*newone);
    for (; it.current(); ++it)
	addDirEntry(it.current(), false);
    
    fileList->setAutoUpdate(true);

    if (repaint_files)
	fileList->repaint(true);

    if ( showStatusLine )
	updateStatusLine();
}

void KFileBaseDialog::addDirEntry(KFileInfo *entry, bool disableUpdating)
{
    if (disableUpdating) {
	repaint_files = false;
	fileList->setAutoUpdate(false);
    }
    insertFile(entry);
    
    if (disableUpdating) {
	fileList->setAutoUpdate(true);
	if (repaint_files)
	    fileList->repaint(true);
    }
    if ( showStatusLine )
	updateStatusLine();
}

void KFileBaseDialog::slotDirEntry(KFileInfo *entry)
{
    addDirEntry(entry, true);
}

void KFileBaseDialog::slotFinished()
{
    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = true;
}

void KFileBaseDialog::slotKfmError(int, const char *)
{
    debug("slotKfmError");    
    if (!finished)
	QApplication::restoreOverrideCursor();
    finished = true;
}

void KFileBaseDialog::comboActivated(int index)
{
    KDir tmp = *dir;
    for (int i= 0; i < index; i++) {
	debug("cdUp %s",tmp.url().data());
	tmp.cdUp();
    }
    setDir(tmp.url(), true);
}

// Code pinched from kfm then hacked
void KFileBaseDialog::back()
{
    if ( backStack.isEmpty() )
	return;
    
    QString *tmp= new QString(dir->path());
    forwardStack.push( tmp );
    
    QString *s = backStack.pop();

    updateHistory( true, !backStack.isEmpty());

    setDir(*s, false);
}

// Code pinched from kfm then hacked
void KFileBaseDialog::forward()
{
    if ( forwardStack.isEmpty() )
	return;

    QString *tmp= new QString(dir->path());
    backStack.push(tmp);

    QString *s = forwardStack.pop();
    updateHistory( !forwardStack.isEmpty(), true);
    setDir(*s, false);
}

// SLOT
void KFileBaseDialog::updateHistory(bool f, bool b)
{
    emit historyUpdate(f, b);
    toolbar->setItemEnabled(FORWARD_BUTTON, f);
    toolbar->setItemEnabled(BACK_BUTTON, b);
}

void KFileBaseDialog::addToBookmarks() // SLOT
{
    debug("Add to bookmarks called");
    bookmarks->add(dir->url(), dir->url());
    bookmarks->write();
}

void KFileBaseDialog::bookmarksChanged() // SLOT
{
  debug("Bookmarks changed called");
}

void KFileBaseDialog::fillBookmarkMenu( KBookmark *parent, QPopupMenu *menu, int &id )
{
    KBookmark *bm;
        
    for ( bm = parent->getChildren().first(); bm != NULL;
	  bm = parent->getChildren().next() )
    {
	if ( bm->getType() == KBookmark::URL )
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

void KFileBaseDialog::toolbarCallback(int i) // SLOT
{
    KConfig *c= kapp->getConfig();
    QString oldgroup= c->group();
    c->setGroup("KFileDialog Settings");
    QString cmd;
    switch(i) {
    case BACK_BUTTON:
	back();
	break;
    case FORWARD_BUTTON:
	forward();
	break;
    case PARENT_BUTTON:
	cdUp();
	break;
    case HOME_BUTTON:
	home();
	break;
    case RELOAD_BUTTON:
	rereadDir();
	break;
    case HOTLIST_BUTTON:
	// It's done on the pressed() signal
	debug("Got pressed signal() for hot list");
	//	toolbarPressedCallback(i);
	break;
    case MKDIR_BUTTON:
	mkdir();
	break;
    case FIND_BUTTON: {
	KShellProcess proc;
	proc << c->readEntry(QString("FindCommandPath"), "kfind");
	proc.start(KShellProcess::DontCare);     
	break;
    }
    case CONFIGURE_BUTTON: {
	KFileDialogConfigureDlg conf(this, "filedlgconf");
	conf.exec();
	fileList->widget()->hide(); 
	delete boxLayout; // this removes all child layouts too
	boxLayout = 0;

	// recreate this widget
	delete fileList;
        fileList = initFileList( wrapper );
	initGUI(); // add them back to the layout managment
	fileList->widget()->show();
	resize(width(), height());
	pathChanged(); // refresh now
	break;
    }
    default:
	warning("KFileDialog: Unknown toolbar button  (id number %d) pressed\n", i);
    }
    c->setGroup(oldgroup);
}

void KFileBaseDialog::cdUp()
{
    KURL u = dir->url().data();
    u.cd("..");
    setDir(u.url(), true);
}

void KFileBaseDialog::home()
{
    setDir( QDir::homeDirPath(), true);
}


void KFileBaseDialog::mkdir()
{
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
    label->setText(QString(i18n("Create new directory in :  ")) + dir->path() );
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
    okButton->setText( i18n("Ok") );
    okButton->setFixedHeight( okButton->height() );
    okButton-> setMinimumWidth( okButton->width() );

    cancelButton = new QPushButton( lMakeDir, "cancelButton" );
    cancelButton->setText( i18n("Cancel") );
    cancelButton->adjustSize();
    cancelButton->setFixedHeight( cancelButton->height() );
    cancelButton->setMinimumWidth( cancelButton->width() );
    
    connect( okButton, SIGNAL(pressed()), lMakeDir, SLOT(accept()) );
    connect( cancelButton, SIGNAL(pressed()), lMakeDir, SLOT(reject()) );
    
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
    if ( lMakeDir->exec() == Accepted ) {
	if ( QDir(dir->path()).mkdir(ed->text()) == true ) {  // !! don't like this move it into KDir ??
	    setDir( QString(dir->url()+ed->text()), true );
	} else {

	    /* Stephan: I don't understand, what this is meant for:
	    QString tmp;
	    if ( !dirnameList->contains( ed->text() )) {
		tmp.sprintf(i18n("check permissions of directory < %s >"), 
			    dir->url().data());
		QMessageBox::message(i18n("Error: could not create directory"),
				     tmp,
				     i18n("Dismiss"),
				     this, "mkdirerrormsg");
	    } else {
		tmp.sprintf(i18n("directory < %s > already exists!"), 
			    dir->url().data());
		QMessageBox::message(i18n("Error: directory already exists"),
				     tmp,
				     i18n("Dismiss"),
				     this, "mkdirerrormsg2");
	    }
	    */
	}
    }
    
    delete lMakeDir;
}

void KFileBaseDialog::toolbarPressedCallback(int i)
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
	QEvent ev(Event_Leave);
	QMouseEvent mev (Event_MouseButtonRelease,
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
	  KBookmark *root= bookmarks->getRoot();
	  for (KBookmark *b= root->getChildren().first();
	       b != 0; b= root->getChildren().next()) {
	    if (i == choice) {
	      fprintf(stderr, "opening bookmark to %s\n", b->getURL());
	      setDir(b->getURL(), true);
	    }
	    i++;
	  }
	}
	
	delete bookmarksMenu;
	bookmarksMenu= 0;
    }    
}

void KFileBaseDialog::dirActivated()
{
    QString tmp = dir->url();
    if (tmp.right(1)[0] != '/')
	tmp += "/";
    QString tmps = fileList->selectedDir()->fileName();
    KURL::encodeURL(tmps);
    tmp += tmps;
    tmp += "/";
    debug("dirActivated %s", tmp.data());
    setDir(tmp, true);
}

void KFileBaseDialog::fileActivated()
{
    debug("fileAct");
	
    if (acceptUrls)
	filename_ = dir->url();
    else
	filename_ = dir->path();
    
    if (filename_.right(1)[0] != '/')
	filename_ += "/";
    
    QString tmps= fileList->selectedFile()->fileName();
    
    KURL::encodeURL(tmps);
    filename_ += tmps;
    emit fileSelected(filename_);
    accept();
}

void KFileBaseDialog::fileHighlighted()
{
    const char *highlighted = 
	fileList->highlightedFile()->fileName();
    
    if (acceptUrls)
	filename_ = dir->url();
    else
	filename_ = dir->path();
    
    if (filename_.right(1)[0] != '/')
	filename_ += "/";

    QString tmp = highlighted;
    KURL::encodeURL(tmp);
    filename_ += tmp;
    locationEdit->setText(filename_);
    emit fileHighlighted(highlighted);
}

KFileInfoContents *KFileDialog::initFileList( QWidget *parent )
{
    bool mixDirsAndFiles = 
	kapp->getConfig()->readNumEntry("MixDirsAndFiles", 0);
    
    bool showDetails = 
	(kapp->getConfig()->readEntry("ViewStyle", 
				      "DetailView") == "DetailView");
    
    bool useSingleClick =
	kapp->getConfig()->readNumEntry("SingleClick",1);
    
    QDir::SortSpec sort = static_cast<QDir::SortSpec>(dir->sorting() &
                                                      QDir::SortByMask);
                                                      
    if (kapp->getConfig()->readNumEntry("KeepDirsFirst",1))
        sort = static_cast<QDir::SortSpec>(sort | QDir::DirsFirst);
    
    dir->setSorting(sort);    

    if (!mixDirsAndFiles)
	
	return new KCombiView(KCombiView::DirList, 
				  showDetails ? KCombiView::DetailView 
				  : KCombiView::SimpleView,
				  useSingleClick, dir->sorting(),
				  parent, "_combi");
    
    else
	
	if (showDetails)
	    return new KFileDetailList(useSingleClick, dir->sorting(), parent, "_details");
	else
	    return new KFileSimpleView(useSingleClick, dir->sorting(), parent, "_simple");
    
}

void KFileBaseDialog::completion() // SLOT
{
    bool useSingleClick =
	kapp->getConfig()->readNumEntry("SingleClick",1);

    QString base;
    if (acceptUrls)
	base = dir->url().copy();
    else
	base = dir->path();

    QString text = locationEdit->currentText();
    if ( KURL(text).isMalformed() )
      return;                         // invalid entry in location
            
    if (text.left(base.length()) == base) {

	QString complete = 
	    fileList->findCompletion(text.right(text.length() - 
						base.length()));

	if (!complete.isNull()) {
	    // debug("Complete %s", complete.data());
	    if ( complete != "../" )
                locationEdit->setText(base + complete);
	} else {
	    // warning("no complete");
	}
    } else {                   // we changed into a parent directory -> go there first
	int l = text.length() - 1;
	while (!text.isEmpty() && text[l] != '/')
	    l--;
	setDir(text.left(l), true);
	locationEdit->setText(text);
	// this recursion is very dangerous.
	// I'm not *that* sure, that it works in every case
        // dg: added a little timeout: time to reread the new directory
        QTimer::singleShot( 500, this, SLOT(completion()) );
	// completion();
    }
}

void KFileBaseDialog::resizeEvent(QResizeEvent *)
{
    toolbar->updateRects(true);
}

void KFileBaseDialog::initUpperChildren()
{

}

void KFileBaseDialog::initLowerChildren()
{

}

/**
 * Returns the url for the selected filename
 */
QString KFileBaseDialog::selectedFileURL()
{
    if (filename_.isNull())
	return 0;
    else {
	KURL u = filename_.data();
	return u.url(); // let KURL check the rest
    }

}

QString KFileDialog::getOpenFileURL(const char *url, const char *filter,
				    QWidget *parent, const char *name)
{
    QString retval;
    
    KFileDialog *dlg= new KFileDialog(url, filter, parent, name, true, true);
    
    dlg->setCaption(i18n("Open"));
    
    if (dlg->exec() == QDialog::Accepted)
	retval = dlg->selectedFileURL();
    else
	retval= 0;
    
    delete dlg;
    if (!retval.isNull())
	debug("getOpenFileURL: returning %s", retval.data());
    
    return retval;
}

KDirDialog::KDirDialog(const char *url, QWidget *parent, const char *name) 
    : KFileBaseDialog(url, 0, parent, name, true, false) 
{
    init();
}
    
KFileInfoContents *KDirDialog::initFileList( QWidget *parent )
{
    return new KDirListBox( true, dir->sorting(), parent, "_dirs" );
}

QString KFileBaseDialog::getDirectory(const char *url,
				  QWidget *parent, const char *name)
{
    QString retval;
    
    KDirDialog *dlg = new KDirDialog(url, parent, name);
    
    dlg->setCaption(i18n("Choose Directory"));
    
    if (dlg->exec() == QDialog::Accepted)
	retval = dlg->selectedFile();
    else
	retval= 0;
    
    delete dlg;
    if (!retval.isNull())
	debug("getDirectory: returning %s", retval.data());
    
    return retval;
}

QString KFileDialog::getSaveFileURL(const char *url, const char *filter,
				    QWidget *parent, const char *name)
{
    QString retval;
    
    KFileDialog *dlg= new KFileDialog(url, filter, parent, name, true, true);
    
    dlg->setCaption(i18n("Save"));
    
    if (dlg->exec() == QDialog::Accepted)
	retval= dlg->selectedFileURL();
    
    delete dlg;
    
    return retval;
}

/**
 * Returns the selected files

QStrList KFileDialog::selectedFileURLList()
{
  // TODO
}

QStrList KFileDialog::getOpenFileURLList(const char *,
					 const char *,
					 QWidget *,
					 const char *)
{
  // TODO
}

QStrList KFileDialog::getSaveFileURLList(const char *, const char *,
		       QWidget *, const char *)
{
  // TODO
}
*/

/**
 * If the argument is true the dialog will accept multiple selections.
 */
void KFileBaseDialog::setMultiSelection(bool)
{
    warning("WARNING: Multi Select is not supported yet");
    // fileList->setMultiSelection(isMulti);
}

void KFileBaseDialog::updateStatusLine()
{
    QString lStatusText;
    QString lFileText("files"), lDirText("directories");
    if ( fileList->numDirs() == 1 )
        lDirText = "directory";
    if ( fileList->numFiles() == 1 )
        lFileText = "file";
    
    lStatusText.sprintf(i18n("%d %s and %d %s"),
			     fileList->numDirs(), lDirText.data(), 
                             fileList->numFiles(), lFileText.data());
    myStatusLine->setText(lStatusText);
}


#include "kfiledialog.moc"

