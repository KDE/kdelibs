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
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
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
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kprocess.h>
#include <kprotocolinfo.h>
#include <kstddirs.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>
#include <kurlcombobox.h>

#include "config-kfile.h"

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
#include <kstaticdeleter.h>

enum Buttons { HOTLIST_BUTTON,
               PATH_COMBO, CONFIGURE_BUTTON };

const int idStart = 1;

template class QList<KIO::StatJob>;


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

    // now following all kind of widgets, that I need to rebuild
    // the geometry managment
    QVBoxLayout *boxLayout;
    QGridLayout *lafBox;
    QHBoxLayout *btngroup;

    QWidget *mainWidget;

    QLabel *myStatusLine;

    QLabel *locationLabel;
    QLabel *filterLabel;
    KURLComboBox *pathCombo;
    QPushButton *okButton, *cancelButton;

    QList<KIO::StatJob> statJobs;

    // an indicator that we're currently in a completion operation
    // we need to lock some slots for this
    bool completionLock;

    KURL::List urlList; //the list of selected urls

    KMimeType::Ptr defaultType; // the default mimetype to save as
    KMimeType::List mimetypes; //the list of possible mimetypes to save as

    // indicates if the location edit should be kept or cleared when changing
    // directories
    bool keepLocation;
};

KURL *KFileDialog::lastDirectory; // to set the start path

static KStaticDeleter<KURL> ldd;

KFileDialog::KFileDialog(const QString& dirName, const QString& filter,
                         QWidget *parent, const char* name, bool modal)
    : KDialogBase( parent, name, modal, QString::null, 0 )
{
    d = new KFileDialogPrivate();
    d->boxLayout = 0;
    d->keepLocation = false;
    d->mainWidget = new QWidget( this, "KFileDialog::mainWidget");
    setMainWidget( d->mainWidget );
    d->okButton = new QPushButton( i18n("&OK"), d->mainWidget );
    d->okButton->setDefault( true );
    d->cancelButton = new QPushButton( i18n("&Cancel"), d->mainWidget );
    connect( d->okButton, SIGNAL( clicked() ), SLOT( slotOk() ));
    connect( d->cancelButton, SIGNAL( clicked() ), SLOT( slotCancel() ));

    d->completionLock = false;
    d->myStatusLine = 0;

    toolbar= new KToolBar( d->mainWidget, "KFileDialog::toolbar", true);

    KURLComboBox *combo = new KURLComboBox( KURLComboBox::Directories, true,
                                            toolbar, "path combo" );
    KURL u;
    u.setPath( QDir::rootDirPath() );
    QString text = i18n("Root Directory: %1").arg( u.path() );
    combo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ), text );

    u.setPath( QDir::homeDirPath() );
    text = i18n("Home Directory: %1").arg( u.path( +1 ) );
    combo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ), text );

    u.setPath( KGlobalSettings::desktopPath() );
    text = i18n("Desktop: %1").arg( u.path( +1 ) );
    combo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ), text );

    /*
    // search for device files on the desktop
    QDir dir( KGlobalSettings::desktopPath() );
    dir.setFilter(QDir::Files);
    const QFileInfoList *list = dir.entryInfoList();
    QFileInfoListIterator it(*list);
    QString tmp;
    if(list){
        QFileInfo *fi;
        for(; (fi = it.current()); ++it){
            KDesktopFile dFile(fi->absFilePath());
            if(dFile.hasDeviceType()) {
                kdDebug() << "got one: " << fi->absFilePath() << endl;
                tmp = dFile.readURL();
                if ( !tmp.isEmpty() )
                    combo->addDefaultURL( tmp,
                                          SmallIcon( dFile.readIcon() ),
                                          dFile.readName() );
            }
        }
    }
    */

    d->pathCombo = combo;
    QToolTip::add( d->pathCombo, i18n("Often used directories") );

    bookmarksMenu = 0L;

    if (!lastDirectory)
    {
        lastDirectory = ldd.setObject(new KURL());
    }

    if (!dirName.isEmpty())
        d->url = KCmdLineArgs::makeURL( QFile::encodeName(dirName) );
    else {
        if (lastDirectory->isEmpty())
            *lastDirectory = QDir::currentDirPath();
        d->url = *lastDirectory;
    }

    ops = new KDirOperator(d->url, d->mainWidget, "KFileDialog::ops");
    ops->setOnlyDoubleClickSelectsFiles( true );
    connect(ops, SIGNAL(updateInformation(int, int)),
            SLOT(updateStatusLine(int, int)));
    connect(ops, SIGNAL(urlEntered(const KURL&)),
            SLOT(urlEntered(const KURL&)));
    connect(ops, SIGNAL(fileHighlighted(const KFileViewItem *)),
            SLOT(fileHighlighted(const KFileViewItem *)));
    connect(ops, SIGNAL(fileSelected(const KFileViewItem *)),
            SLOT(fileSelected(const KFileViewItem *)));
    connect(ops, SIGNAL(finishedLoading()),
            SLOT(slotLoadingFinished()));

    KActionCollection *coll = ops->actionCollection();
    coll->action( "up" )->plug( toolbar );
    coll->action( "back" )->plug( toolbar );
    coll->action( "forward" )->plug( toolbar );
    coll->action( "home" )->plug( toolbar );
    coll->action( "reload" )->plug( toolbar );


    bookmarks= new KFileBookmarkManager();
    CHECK_PTR( bookmarks );
    connect( bookmarks, SIGNAL( changed() ),
             this, SLOT( bookmarksChanged() ) );

    QString bmFile = locate("data", QString::fromLatin1("kfile/bookmarks.html"));
    if (!bmFile.isNull())
        bookmarks->read(bmFile);

    toolbar->insertButton(QString::fromLatin1("bookmark"),
                          (int)HOTLIST_BUTTON, true,
                          i18n("Bookmarks"));
    /*
    toolbar->insertButton(QString::fromLatin1("configure"),
                          (int)CONFIGURE_BUTTON, true,
                          i18n("Configure this dialog"));
    */
    coll->action( "mkdir" )->plug( toolbar );

    connect(toolbar, SIGNAL(clicked(int)),
            SLOT(toolbarCallback(int)));
    // for the bookmark "menu"
    connect(toolbar, SIGNAL(pressed(int)),
            this, SLOT(toolbarPressedCallback(int)));

    toolbar->insertWidget(PATH_COMBO, 0, d->pathCombo);

    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setIconText(KToolBar::IconOnly);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->enableMoving(false);
    toolbar->adjustSize();

    locationEdit = new KURLComboBox(KURLComboBox::Files, true,
				    d->mainWidget, "LocationEdit");
    locationEdit->setInsertionPolicy(QComboBox::NoInsertion);
    locationEdit->setFocus();
    locationEdit->setCompletionObject( ops->completionObject(), false );

    connect( locationEdit, SIGNAL( returnPressed() ), SLOT( slotOk()));
    connect(locationEdit, SIGNAL( activated( const QString&  )),
            this,  SLOT( locationActivated( const QString& ) ));
    connect( locationEdit, SIGNAL( completion( const QString& )),
             SLOT( fileCompletion( const QString& )));
    connect( locationEdit, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             locationEdit, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    d->pathCombo->setCompletionObject( ops->dirCompletionObject(), false );

    connect( d->pathCombo, SIGNAL( urlActivated( const KURL&  )),
             this,  SLOT( pathComboActivated( const KURL& ) ));
    connect( d->pathCombo, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( pathComboReturnPressed( const QString& ) ));
    connect( d->pathCombo, SIGNAL(textChanged( const QString& )),
             SLOT( pathComboChanged( const QString& ) ));
    connect( d->pathCombo, SIGNAL( completion( const QString& )),
             SLOT( dirCompletion( const QString& )));
    connect( d->pathCombo, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             d->pathCombo, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    d->locationLabel = new QLabel(locationEdit, i18n("&Location:"),
                                  d->mainWidget);

    filterWidget = new KFileFilter(d->mainWidget, "KFileDialog::filterwidget");
    filterWidget->setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    connect(filterWidget, SIGNAL(filterChanged()), SLOT(slotFilterChanged()));
    ops->setNameFilter(filterWidget->currentFilter());

    // Get the config object
    KSimpleConfig *kc = new KSimpleConfig(QString::fromLatin1("kdeglobals"),
                                          false);
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );
    d->showStatusLine = kc->readBoolEntry(ConfigShowStatusLine,
                                          DefaultShowStatusLine);

    initGUI(); // activate GM

    readRecentFiles( KGlobal::config() );

    adjustSize();

    // we set the completionLock to avoid entering pathComboChanged() when
    // inserting the list of URLs into the combo.
    d->completionLock = true;
    readConfig( kc, ConfigGroup );
    d->completionLock = false;
    delete kc;

    setSelection(d->url.url());
    ops->setView(KFile::Default);

    ops->clearHistory();
}

void KFileDialog::setLocationLabel(const QString& text)
{
    d->locationLabel->setText(text);
}

void KFileDialog::setFilter(const QString& filter)
{
    filterWidget->setFilter(filter);
    ops->setNameFilter(filterWidget->currentFilter());
}

QString KFileDialog::currentFilter() const
{
    return filterWidget->currentFilter();
}

void KFileDialog::setFilterMimeType(const QString &label,
        const KMimeType::List &types, const KMimeType::Ptr &defaultType)
{
    d->mimetypes = types;
    d->defaultType = defaultType;
    d->filterLabel->setText(label);

    QString filter = i18n("*|Default (%1)").arg(defaultType->comment());

    for(KMimeType::List::ConstIterator it = types.begin();
        it != types.end();
        ++it)
    {
        KMimeType::Ptr type = *it;
        filter = filter + '\n' + type->patterns().join(QString::fromLatin1(" "))+'|'+type->comment();
    }

    setFilter(filter);
}

KMimeType::Ptr KFileDialog::currentFilterMimeType()
{
    int i = filterWidget->currentItem()-1;

    if ((i >= 0) && (i < (int) d->mimetypes.count()))
       return d->mimetypes[i];
    return d->defaultType;
}

void KFileDialog::setPreviewWidget(const QWidget *w) {
    ops->setPreviewWidget(w);
}


// FIXME: check for "existing" flag here?
void KFileDialog::slotOk()
{
    kdDebug(kfile_area) << "slotOK\n";

    if ( (mode() & KFile::Directory) != KFile::Directory ) {
        if ( locationEdit->currentText().stripWhiteSpace().isEmpty() ) {
	    const KFileViewItemList *items = ops->selectedItems();
	    if ( !items || items->isEmpty() )
	        return;

	    // weird case: the location edit is empty, but there are highlighted files
	    else {

		bool multi = (mode() & KFile::Files) != 0;
	        KFileViewItemListIterator it( *items );
		QString endQuote = QString::fromLatin1("\" ");
		QString name, files;
		while ( it.current() ) {
		    name = (*it)->name();
		    if ( multi ) {
		        name.prepend( '"' );
			name.append( endQuote );
		    }

		    files.append( name );
		    ++it;
		}
		locationEdit->setEditText( files );
		return;
	    }
	}
    }

    KURL selectedURL;

    if ( (mode() & KFile::Files) == KFile::Files ) {// multiselection mode
	if ( locationEdit->currentText().contains( '/' )) {

	    // relative path? -> prepend the current directory
	    KURL u( ops->url(), locationEdit->currentText() );
	    if ( !u.isMalformed() )
		selectedURL = u;
	    else
		selectedURL = ops->url();
	}
	else // simple filename -> just use the current URL
	    selectedURL = ops->url();
    }

    else {
        QString url = locationEdit->currentText();
        KURL u( ops->url(), url );
        selectedURL = u;
    }

    if ( selectedURL.isMalformed() ) {
       KMessageBox::sorry( d->mainWidget, i18n("Sorry,\n%1\ndoesn't look like a valid URL\nto me.").arg(d->url.url()), i18n("Invalid URL") );
       return;
    }

    if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
         !selectedURL.isLocalFile() ) {
        KMessageBox::sorry( d->mainWidget,
                            i18n("You can only select local files."),
                            i18n("Remote files not accepted") );
        return;
    }

    d->url = selectedURL;

    // d->url is a correct URL now

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
        kdDebug(kfile_area) << "Directory\n";
	bool done = true;
        if ( d->url.isLocalFile() ) {
	    if ( locationEdit->currentText().stripWhiteSpace().isEmpty() ) {
		QFileInfo info( d->url.path() );
		if ( info.isDir() ) {
		    locationEdit->insertItem( d->url.path(+1), 1 );
		    d->filenames = QString::null;
		    d->urlList.clear();
		    d->urlList.append( d->url );
		    accept();
		}

		else if (!info.exists() && (mode() & KFile::File) != KFile::File) {
		    // directory doesn't exist, create and enter it
		    if ( ops->mkdir( d->url.url(), true ))
			return;

		    else {
			locationEdit->insertItem( d->url.prettyURL(+1), 1 );
			accept();
		    }
		}
	
		else { // d->url is not a directory,
		    // maybe we are in File(s) | Directory mode
		    if ( mode() & KFile::File == KFile::File ||
			 mode() & KFile::Files == KFile::Files )
			done = false;
		}
	    }
	    else { // Directory mode, with file[s]/dir[s] selected
		d->filenames = locationEdit->currentText();
		accept(); // what can we do?
	    }

	}

	else { // FIXME: remote directory, should we allow that?
	}

	if ( done )
	    return;
    }

    KIO::StatJob *job = 0L;
    d->statJobs.clear();
    d->filenames = locationEdit->currentText();

    if ( (mode() & KFile::Files) == KFile::Files &&
	 !locationEdit->currentText().contains( '/' )) {
        kdDebug(kfile_area) << "Files\n";
        KURL::List list = parseSelectedURLs();
        KURL::List::ConstIterator it = list.begin();
        for ( ; it != list.end(); ++it ) {
            job = KIO::stat( *it, !(*it).isLocalFile() );
            d->statJobs.append( job );
            connect( job, SIGNAL( result(KIO::Job *) ),
                     SLOT( slotStatResult( KIO::Job *) ));
        }
        return;
    }

    job = KIO::stat(d->url,!d->url.isLocalFile());
    d->statJobs.append( job );
    connect(job, SIGNAL(result(KIO::Job*)), SLOT(slotStatResult(KIO::Job*)));
}


// FIXME : count all errors and show messagebox when d->statJobs.count() == 0
// in case of an error, we cancel the whole operation (clear d->statJobs and
// don't call accept)
void KFileDialog::slotStatResult(KIO::Job* job)
{
    kdDebug(kfile_area) << "slotStatResult" << endl;
    KIO::StatJob *sJob = static_cast<KIO::StatJob *>( job );

    if ( !d->statJobs.removeRef( sJob ) ) {
        return;
    }

    int count = d->statJobs.count();

    // errors mean in general, the location is no directory ;/
    // Can we be sure that it is exististant at all? (pfeiffer)
    if (sJob->error() && count == 0)
       accept();

    KIO::UDSEntry t = sJob->statResult();
    bool isDir = false;
    for (KIO::UDSEntry::ConstIterator it = t.begin();
        it != t.end(); ++it) {
       if ((*it).m_uds == KIO::UDS_FILE_TYPE ) {
           isDir = S_ISDIR( (mode_t)((*it).m_long));
           break;
       }
    }

    // currently, we only stat in File[s] mode, not Directory mode, so a
    // directory means ERROR
    if (isDir) {
        if ( count == 0 )
            setURL( sJob->url() );
        d->statJobs.clear();
        return;
    }

    kdDebug(kfile_area) << "filename " << sJob->url().url() << endl;
    locationEdit->insertItem( sJob->url().prettyURL(), 1 );

    if ( count == 0 )
        accept();
}


void KFileDialog::accept()
{
    *lastDirectory = ops->url();
    KSimpleConfig *c = new KSimpleConfig(QString::fromLatin1("kdeglobals"),
                                         false);
    saveConfig( c, ConfigGroup );
    saveRecentFiles( KGlobal::config() );
    delete c;

    KDialogBase::accept();

    if ( mode() & KFile::Files != KFile::Files ) // single selection
	emit fileSelected(d->url.url());

    ops->close();
    emit okClicked();
}


void KFileDialog::fileHighlighted(const KFileViewItem *i)
{
    if (i && i->isDir())
        return;


    if ( (ops->mode() & KFile::Files) != KFile::Files ) {
        if ( !i )
            return;

        d->url = i->url();

        if ( !d->completionLock ) {
            locationEdit->setCurrentItem( 0 );
            locationEdit->setEditText( i->name() );
        }
        emit fileHighlighted(d->url.url());
    }

    else {
        multiSelectionChanged();
        emit selectionChanged();
    }
}

void KFileDialog::fileSelected(const KFileViewItem *i)
{
    if (i && i->isDir())
        return;

    if ( (ops->mode() & KFile::Files) != KFile::Files ) {
        if ( !i )
            return;

        d->url = i->url();
        locationEdit->setCurrentItem( 0 );
        locationEdit->setEditText( i->name() );
    }
    else {
        multiSelectionChanged();
        emit selectionChanged();
    }
    slotOk();
}


// I know it's slow to always iterate thru the whole filelist
// (ops->selectedItems()), but what can we do?
void KFileDialog::multiSelectionChanged()
{
    if ( d->completionLock ) // FIXME: completion with multiselection?
        return;

    KFileViewItem *item;
    const KFileViewItemList *list = ops->selectedItems();
    if ( !list ) {
        locationEdit->clearEdit();
        return;
    }

    static const QString &begin = KGlobal::staticQString(" \"");
    KFileViewItemListIterator it ( *list );
    QString text;
    while ( (item = it.current()) ) {
        text.append( begin ).append( item->name() ).append( '\"' );
        ++it;
    }
    locationEdit->setCurrentItem( 0 );
    locationEdit->setEditText( text.stripWhiteSpace() );
}


void KFileDialog::initGUI()
{
    if (d->boxLayout)
        delete d->boxLayout; // deletes all sub layouts

    d->boxLayout = new QVBoxLayout( d->mainWidget, 0, KDialog::spacingHint());
    d->boxLayout->addWidget(toolbar, AlignTop);
    d->boxLayout->addWidget(ops, 4);
    d->boxLayout->addSpacing(3);

    d->lafBox= new QGridLayout(2, 3, KDialog::spacingHint());
    d->boxLayout->addLayout(d->lafBox, 0);
    d->lafBox->addWidget(d->locationLabel, 0, 0, AlignVCenter);
    d->lafBox->addWidget(locationEdit, 0, 1, AlignVCenter);
    d->lafBox->addWidget(d->okButton, 0, 2, AlignVCenter);

    d->lafBox->addWidget(d->filterLabel, 1, 0, AlignVCenter);
    d->lafBox->addWidget(filterWidget, 1, 1, AlignVCenter);
    d->lafBox->addWidget(d->cancelButton, 1, 2, AlignVCenter);

    d->lafBox->setColStretch(1, 4);

    delete d->myStatusLine;
    d->myStatusLine = 0L;

    // Add the status line
    if ( d->showStatusLine ) {
        d->myStatusLine = new QLabel( d->mainWidget, "StatusBar" );
        updateStatusLine(ops->numDirs(), ops->numFiles());
        d->myStatusLine->setFrameStyle( QFrame::Panel | QFrame::Sunken );
        d->myStatusLine->setAlignment( AlignHCenter | AlignVCenter );
        d->boxLayout->addWidget( d->myStatusLine, 0 );
        d->myStatusLine->show();
    }

    d->boxLayout->addSpacing(3);

    setTabOrder(ops,  locationEdit);
    setTabOrder(locationEdit, filterWidget);
    setTabOrder(filterWidget, d->okButton);
    setTabOrder(d->okButton, d->cancelButton);
    setTabOrder(d->cancelButton, d->pathCombo);
    setTabOrder(d->pathCombo, ops);
}

KFileDialog::~KFileDialog()
{
    hide();
    delete bookmarks;
    delete d->boxLayout; // we can't delete a widget being managed by a layout,
    d->boxLayout = 0;    // so we delete the layout before (Qt bug to be fixed)
    delete ops;
    delete d;
}

void KFileDialog::slotFilterChanged() // SLOT
{
    ops->setNameFilter(filterWidget->currentFilter());
    ops->rereadDir();
    emit filterChanged(filterWidget->currentFilter());
}


void KFileDialog::pathComboChanged( const QString& txt )
{
    if ( d->completionLock )
        return;

    static const QString& localRoot = KGlobal::staticQString("file:/");
    KURLComboBox *combo = d->pathCombo;
    QString text = txt;
    QString newText = text.left(combo->cursorPosition() -1);
    KURL url;
    if ( text.at( 0 ) == '/' )
        url.setPath( text );
    else
        url = text;


    // don't mess with malformed urls or remote urls without directory or host
    if ( url.isMalformed() ||
	 !KProtocolInfo::supportsListing( url.protocol() ) ||
         ( !url.url().startsWith( localRoot ) &&
           ( url.directory().isNull() || url.host().isNull()) )) {
        d->completionHack = newText;
        return;
    }

    // when editing somewhere in the middle of the text, don't complete or
    // follow directories
    if ( combo->cursorPosition() != (int) combo->currentText().length() ) {
        d->completionHack = newText;
        return;
    }


    // the user is backspacing -> don't annoy him with completions
    if ( autoDirectoryFollowing && d->completionHack.startsWith( newText ) ) {
        // but we can follow the directories, if configured so

        // find out the current directory according to combobox and cd into
        int l = text.length() - 1;
        while (!text.isEmpty() && text[l] != '/')
            l--;

        KURL newLocation(text.left(l+1));

        if ( !newLocation.isMalformed() && newLocation != ops->url() ) {
            setURL(newLocation, true);
            combo->setEditText(text);
        }
    }

    // typing forward, ending with a / -> cd into the directory
    else if ( autoDirectoryFollowing &&
              text.at(text.length()-1) == '/' && ops->url() != text ) {
        d->selection = QString::null;
        setURL( text, false );
    }

    d->completionHack = newText;
}


void KFileDialog::setURL(const KURL& url, bool clearforward)
{
    d->selection = QString::null;
    ops->setURL( url, clearforward);
}

// Protected
void KFileDialog::urlEntered(const KURL& url)
{
    QString filename = locationEdit->currentText();
    d->selection = QString::null;

    if ( d->pathCombo->count() != 0 ) { // little hack
        d->pathCombo->setURL( url );
    }

    locationEdit->blockSignals( true );
    locationEdit->setCurrentItem( 0 );
    if ( d->keepLocation )
	locationEdit->setEditText( filename );

    locationEdit->blockSignals( false );
    d->completionHack = d->pathCombo->currentText();
}

void KFileDialog::locationActivated( const QString& url )
{
    setSelection( url );
}

void KFileDialog::pathComboActivated( const KURL& url)
{
    setURL( url );
}

void KFileDialog::pathComboReturnPressed( const QString& url )
{
    setURL( url );
}

void KFileDialog::addToBookmarks() // SLOT
{
    bookmarks->add(ops->url().url(), ops->url().url());
    QString dir = KGlobal::dirs()->saveLocation("data",
						QString::fromLatin1("kfile/"));
    if ( !dir.isEmpty() )
	bookmarks->write( dir + QString::fromLatin1("bookmarks.html") );
}

void KFileDialog::bookmarksChanged() // SLOT
{
    //    kdDebug() << "bookmarksChanged" << endl;
}

void KFileDialog::fillBookmarkMenu( KFileBookmark *parent, QPopupMenu *menu, int &id )
{
    KFileBookmark *bm;

    for ( bm = parent->getChildren().first(); bm != NULL;
          bm = parent->getChildren().next() )
    {
        if ( bm->getType() == KFileBookmark::URL )
        {
            QPixmap pix = KMimeType::pixmapForURL( bm->getURL(), 0, 0,
						   KIcon::SizeSmall );
            if ( !pix.isNull() )
              menu->insertItem( pix, bm->getText(), id );
            else
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
            kdDebug(kfile_area) << "Bookmark: choice was " << choice << endl;
            KFileBookmark *root= bookmarks->getRoot();
            for (KFileBookmark *b= root->getChildren().first();
                 b != 0; b= root->getChildren().next()) {
                if (i == choice) {
                    kdDebug(kfile_area) << "Bookmark: opening " << b->getURL() << endl;
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

    KURL u;
    if ( KURL::isRelativeURL( url ) ) // perhaps we have a relative path!?
      u = KURL( ops->url(), url );
    else
      u = KURL( url );

    if (u.isMalformed()) { // if it still is
        kdWarning() << url << " is not a correct argument for setSelection!" << endl;
        return;
    }

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KFileViewItem i(-1, -1, u, true );
    //    KFileViewItem i(u.path());
    if ( i.isDir() && u.isLocalFile() && QFile::exists( u.path() ) )
        // trust isDir() only if the file is
        // local (we cannot stat non-local urls) and if it exists!
        // (as KFileItem does not check if the file exists or not
        // -> the statbuffer is undefined -> isDir() is unreliable) (Simon)
        setURL(u, true);
    else {
        QString filename = u.url();
        int sep = filename.findRev('/');
        if (sep >= 0) { // there is a / in it
            setURL(filename.left(sep), true);
            // filename must be decoded, or "name with space" would become
            // "name%20with%20space", so we use KURL::fileName()
            filename = u.fileName();
            kdDebug(kfile_area) << "filename " << filename << endl;
            d->selection = filename;
            locationEdit->setCurrentItem( 0 );
            locationEdit->setEditText( filename );
        }

        d->url = KURL(ops->url(), filename);
    }
}

void KFileDialog::slotLoadingFinished()
{
    if ( !d->selection.isNull() )
        ops->setCurrentItem( d->selection );
}


void KFileDialog::dirCompletion( const QString& dir ) // SLOT
{
    QString base = ops->url().url();

    // if someone uses completion, he doesn't like the current selection
    d->selection = QString::null;

    KURL url;
    if ( dir.at( 0 ) == '/' )
        url.setPath( dir );
    else
        url = dir;

    if ( url.isMalformed() )
        return; // invalid entry in path combo

    d->completionLock = true;

    if (url.url().startsWith( base )) {
        QString complete =
            ops->makeDirCompletion( url.fileName(false) );

        if (!complete.isNull()) {
	    QString newText = base + complete;
	    QString fileProt = QString::fromLatin1( "file:" );
	    if ( dir.startsWith( fileProt ) != newText.startsWith( fileProt ))
		newText = newText.mid( 5 ); // remove file:

            d->pathCombo->setCompletedText( newText );
            d->url = newText;
        }
    }
    d->completionLock = false;
}


void KFileDialog::fileCompletion( const QString& file )
{
    d->completionLock = true;
    QString text = ops->makeCompletion( file );
    if ( !text.isEmpty() )
        locationEdit->setCompletedText( text );
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

QString KFileDialog::getOpenFileName(const QString& dir, const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.ops->clearHistory();
    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

QStringList KFileDialog::getOpenFileNames(const QString& dir,const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    QStringList list = dlg.selectedFiles();
    QStringList::Iterator it = list.begin();
    for( ; it != list.end(); ++it )
        KRecentDocument::add( *it );

    return list;
}

KURL KFileDialog::getOpenURL(const QString& dir, const QString& filter,
                                QWidget *parent, const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.ops->clearHistory();
    dlg.exec();

    const KURL& url = dlg.selectedURL();
    if (!url.isMalformed()) {
        if ( url.isLocalFile() )
            KRecentDocument::add( url.path(-1) );
        else
            KRecentDocument::add( url.url(-1), true );
    }

    return url;
}

KURL::List KFileDialog::getOpenURLs(const QString& dir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    KURL::List list = dlg.selectedURLs();
    KURL::List::ConstIterator it = list.begin();
    KURL u;
    for( ; it != list.end(); ++it ) {
        u = *it;
        if ( !u.isMalformed() ) {
            if ( u.isLocalFile() )
                KRecentDocument::add( u.path(-1) );
            else
                KRecentDocument::add( u.url(-1), true );
        }
    }

    return list;
}

QString KFileDialog::getExistingDirectory(const QString& dir,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(dir, QString::null, parent, "filedialog", true);
    dlg.setMode(KFile::Directory);
    dlg.ops->clearHistory();
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
        if ( (ops->mode() & KFile::Files) == KFile::Files )
            list = parseSelectedURLs();
        else
            list.append( d->url );
    }
    return list;
}


KURL::List& KFileDialog::parseSelectedURLs() const
{
    if ( d->filenames.isEmpty() ) {
        return d->urlList;
    }

    d->urlList.clear();
    if ( d->filenames.contains( '/' )) { // assume _one_ absolute filename
        static const QString &prot = KGlobal::staticQString(":/");
        KURL u;
        if ( d->filenames.find( prot ) != -1 )
            u = d->filenames;
        else
            u.setPath( d->filenames );

        if ( !u.isMalformed() )
            d->urlList.append( u );
        else
            KMessageBox::error( d->mainWidget,
                                i18n("The chosen filename(s) don't\nappear to be valid."), i18n("Invalid filename(s)") );
    }

    else
        d->urlList = tokenize( d->filenames );

    d->filenames = QString::null; // indicate that we parsed that one

    return d->urlList;
}


// FIXME: current implementation drawback: a filename can't contain quotes
KURL::List KFileDialog::tokenize( const QString& line ) const
{
    KURL::List urls;
    KURL u( ops->url() );
    QString name;

    int count = line.contains( '"' );
    if ( count == 0 ) { // no " " -> assume one single file
        u.setFileName( line );
        if ( !u.isMalformed() )
            urls.append( u );

        return urls;
    }

    if ( (count % 2) == 1 ) { // odd number of " -> error (FIXME: Messagebox?)
        QWidget *that = const_cast<KFileDialog *>(this);
        KMessageBox::sorry(that, i18n("The requested filenames\n%1\ndon't look valid to me.\nMake sure every filename is enclosed in doublequotes").arg(line), i18n("Filename error"));
        return urls;
    }

    int start = 0;
    int index1 = -1, index2 = -1;
    while ( true ) {
        index1 = line.find( '"', start );
        index2 = line.find( '"', index1 + 1 );

        if ( index1 < 0 )
            break;

        // get everything between the " "
        name = line.mid( index1 + 1, index2 - index1 - 1 );
        u.setFileName( name );
        if ( !u.isMalformed() )
            urls.append( u );

        start = index2 + 1;
    }
    return urls;
}


QString KFileDialog::selectedFile() const
{
    if ( result() == QDialog::Accepted )
    {
       if (d->url.isLocalFile())
          return d->url.path();
    }
    return QString::null;
}

QStringList KFileDialog::selectedFiles() const
{
    QStringList list;

    if ( result() == QDialog::Accepted ) {
        if ( (ops->mode() & KFile::Files) == KFile::Files ) {
            KURL::List urls = parseSelectedURLs();
	    QValueListConstIterator<KURL> it = urls.begin();
	    while ( it != urls.end() ) {
		if ( (*it).isLocalFile() )
		    list.append( (*it).path() );
		++it;
	    }
	}

        else { // single-selection mode
	    if ( d->url.isLocalFile() )
		list.append( d->url.path() );
        }
    }

    return list;
}

KURL KFileDialog::baseURL() const
{
    return ops->url();
}

QString KFileDialog::getSaveFileName(const QString& dir, const QString& filter,
                                     QWidget *parent,
                                     const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);
    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);
    dlg.setKeepLocation( true );

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

KURL KFileDialog::getSaveURL(const QString& dir, const QString& filter,
                                     QWidget *parent,
                                     const QString& caption)
{
    KFileDialog dlg(dir, filter, parent, "filedialog", true);
    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);
    dlg.setKeepLocation( true );

    dlg.exec();

    KURL url = dlg.selectedURL();
    if (!url.isMalformed()) {
        if ( url.isLocalFile() )
            KRecentDocument::add( url.path(-1) );
        else
            KRecentDocument::add( url.url(-1) );
    }

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

    KURLComboBox *combo = d->pathCombo;
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
    kc->writeEntry( DialogWidth.arg( desk->width() ), width() );
    kc->writeEntry( DialogHeight.arg( desk->height() ), height() );

    ops->saveConfig( kc, group );
    kc->setGroup( oldGroup );
    kc->sync();
}


void KFileDialog::readRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    locationEdit->setMaxItems( kc->readNumEntry( RecentFilesNumber,
                                                 DefaultRecentURLsNumber ) );
    locationEdit->setURLs( kc->readListEntry( RecentFiles ) );
    locationEdit->insertItem( QString::null, 0 ); // dummy item without pixmap
    locationEdit->setCurrentItem( 0 );

    kc->setGroup( oldGroup );
}

void KFileDialog::saveRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    kc->writeEntry( RecentFiles, locationEdit->urls() );
    kc->sync();

    kc->setGroup( oldGroup );
}

QPushButton * KFileDialog::okButton() const
{
    return d->okButton;
}

QPushButton * KFileDialog::cancelButton() const
{
    return d->cancelButton;
}

void KFileDialog::slotCancel()
{
    ops->close();
    KDialogBase::slotCancel();
}

void KFileDialog::setKeepLocation( bool keep )
{
    d->keepLocation = keep;
}

bool KFileDialog::keepsLocation() const
{
    return d->keepLocation;
}

#include "kfiledialog.moc"
