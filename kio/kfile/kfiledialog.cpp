// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include <qptrcollection.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kcompletionbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/job.h>
#include <kio/previewjob.h>
#include <kio/scheduler.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kpopupmenu.h>
#include <kprotocolinfo.h>
#include <kpushbutton.h>
#include <krecentdirs.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kstaticdeleter.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>

#include "config-kfile.h"
#include "kpreviewwidgetbase.h"

#include <kfileview.h>
#include <krecentdocument.h>
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#include <kdiroperator.h>
#include <kimagefilepreview.h>

#include <kurlbar.h>
#include <kfilebookmarkhandler.h>

enum Buttons { HOTLIST_BUTTON,
               PATH_COMBO, CONFIGURE_BUTTON };

template class QPtrList<KIO::StatJob>;

static void silenceQToolBar(QtMsgType, const char *)
{
}

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

    // now following all kind of widgets, that I need to rebuild
    // the geometry managment
    QBoxLayout *boxLayout;
    QWidget *mainWidget;
    
    QLabel *locationLabel;
    
    // @deprecated remove in KDE4
    QLabel *filterLabel;
    KURLComboBox *pathCombo;
    KPushButton *okButton, *cancelButton;
    KURLBar *urlBar;
    QWidget *customWidget;

    QPtrList<KIO::StatJob> statJobs;

    KURL::List urlList; //the list of selected urls

    QStringList mimetypes; //the list of possible mimetypes to save as
    
    // indicates if the location edit should be kept or cleared when changing
    // directories
    bool keepLocation :1;

    // the KDirOperators view is set in KFileDialog::show(), so to avoid
    // setting it again and again, we have this nice little boolean :)
    bool hasView :1;

    // do we show the speedbar for the first time?
    bool initializeSpeedbar :1;

    // an indicator that we're currently in a completion operation
    // we need to lock some slots for this
    bool completionLock :1;

    bool hasDefaultFilter :1; // necessary for the operationMode
    KFileDialog::OperationMode operationMode;

    // The file class used for KRecentDirs
    QString fileClass;

    KFileBookmarkHandler *bookmarkHandler;
    
    // the ID of the path drop down so subclasses can place their custom widgets properly
    int m_pathComboIndex;
};

KURL *KFileDialog::lastDirectory; // to set the start path

static KStaticDeleter<KURL> ldd;

KFileDialog::KFileDialog(const QString& startDir, const QString& filter,
                         QWidget *parent, const char* name, bool modal)
    : KDialogBase( parent, name, modal, QString::null, 0 )
{
    init( startDir, filter, 0 );
}

KFileDialog::KFileDialog(const QString& startDir, const QString& filter,
                         QWidget *parent, const char* name, bool modal, QWidget* widget)
    : KDialogBase( parent, name, modal, QString::null, 0 )
{
    init( startDir, filter, widget );
}

KFileDialog::~KFileDialog()
{
    hide();

    KConfig *config = KGlobal::config();

    if ( d->initializeSpeedbar && d->urlBar->isModified() ) {
        QString oldGroup = config->group();
        config->setGroup( ConfigGroup );
        // write to kdeglobals
        config->writeEntry( "Set speedbar defaults", false, true, true );
        config->setGroup( oldGroup );
    }

    d->urlBar->writeConfig( config, "KFileDialog Speedbar" );
    config->sync();

    delete ops;
    delete d;
}

void KFileDialog::setLocationLabel(const QString& text)
{
    d->locationLabel->setText(text);
}

void KFileDialog::setFilter(const QString& filter)
{
    if ( filter.contains( '/' )) { // mimetype filter!
        QStringList filters = QStringList::split( " ", filter );
        setMimeFilter( filters );
        return;
    }

    ops->clearFilter();
    filterWidget->setFilter(filter);
    ops->setNameFilter(filterWidget->currentFilter());
    d->hasDefaultFilter = false;
    filterWidget->setEditable( true );
}

QString KFileDialog::currentFilter() const
{
    return filterWidget->currentFilter();
}

// deprecated
void KFileDialog::setFilterMimeType(const QString &label,
                                    const KMimeType::List &types,
                                    const KMimeType::Ptr &defaultType)
{
    d->mimetypes.clear();
    d->filterLabel->setText(label);

    KMimeType::List::ConstIterator it;
    for( it = types.begin(); it != types.end(); ++it)
        d->mimetypes.append( (*it)->name() );

    setMimeFilter( d->mimetypes, defaultType->name() );
}

void KFileDialog::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->mimetypes = mimeTypes;
    filterWidget->setMimeFilter( mimeTypes, defaultType );

    QStringList types = QStringList::split(" ", filterWidget->currentFilter());
    types.append( QString::fromLatin1( "inode/directory" ));
    ops->clearFilter();
    ops->setMimeFilter( types );
    d->hasDefaultFilter = !defaultType.isEmpty();
    filterWidget->setEditable( !d->hasDefaultFilter ||
                               d->operationMode != Saving );
}

void KFileDialog::clearFilter()
{
    d->mimetypes.clear();
    filterWidget->setFilter( QString::null );
    ops->clearFilter();
    d->hasDefaultFilter = false;
    filterWidget->setEditable( true );
}

QString KFileDialog::currentMimeFilter() const
{
    int i = filterWidget->currentItem();
    if (filterWidget->showsAllTypes())
        i--;

    if ((i >= 0) && (i < (int) d->mimetypes.count()))
        return d->mimetypes[i];
    return QString::null; // The "all types" item has no mimetype
}

KMimeType::Ptr KFileDialog::currentFilterMimeType()
{
    return KMimeType::mimeType( currentMimeFilter() );
}

void KFileDialog::setPreviewWidget(const QWidget *w) {
    ops->setPreviewWidget(w);
}

void KFileDialog::setPreviewWidget(const KPreviewWidgetBase *w) {
    ops->setPreviewWidget(w);
}

// FIXME: check for "existing" flag here?
void KFileDialog::slotOk()
{
    kdDebug(kfile_area) << "slotOK\n";

    // a list of all selected files/directories (if any)
    // can only be used if the user didn't type any filenames/urls himself
    const KFileItemList *items = ops->selectedItems();

    if ( (mode() & KFile::Directory) != KFile::Directory ) {
        if ( locationEdit->currentText().stripWhiteSpace().isEmpty() ) {
	    if ( !items || items->isEmpty() )
	        return;

	    // weird case: the location edit is empty, but there are
	    // highlighted files
	    else {

		bool multi = (mode() & KFile::Files) != 0;
	        KFileItemListIterator it( *items );
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
		locationEdit->lineEdit()->setEdited( false );
		return;
	    }
	}
    }

    bool dirOnly = !(mode() & (KFile::File | KFile::Files));

    // we can use our kfileitems, no need to parse anything
    if ( items && !locationEdit->lineEdit()->edited() &&
	 !(items->isEmpty() && !dirOnly) ) {

	d->urlList.clear();
	d->filenames = QString::null;

	if ( dirOnly ) {
	    d->url = ops->url();
	}
	else {
	    if ( !(mode() & KFile::Files) ) {// single selection
		d->url = items->getFirst()->url();
	    }

	    else { // multi (dirs and/or files)
		d->url = ops->url();
		KFileItemListIterator it( *items );
		while ( it.current() ) {
		    d->urlList.append( (*it)->url() );
		    ++it;
		}
	    }
	}

	accept();
	return;
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
        QString text = locationEdit->currentText();
        if ( KURL::isRelativeURL(text) ) // only a full URL isn't relative. Even /path is.
        {
            if ( !text.isEmpty() && text[0] == '/' ) // absolute path
                selectedURL.setPath( text );
            else
            {
                selectedURL = ops->url();
                selectedURL.addPath( text ); // works for filenames and relative paths
            }
        } else // complete URL
            selectedURL = text;
    }

    if ( selectedURL.isMalformed() ) {
       KMessageBox::sorry( d->mainWidget, i18n("%1\ndoes not appear to be a valid URL.\n").arg(d->url.url()), i18n("Invalid URL") );
       return;
    }

    if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
         !selectedURL.isLocalFile() ) {
        KMessageBox::sorry( d->mainWidget,
                            i18n("You can only select local files."),
                            i18n("Remote Files not Accepted") );
        return;
    }

    d->url = selectedURL;

    // d->url is a correct URL now

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
        kdDebug(kfile_area) << "Directory" << endl;
        bool done = true;
        if ( d->url.isLocalFile() ) {
            if ( locationEdit->currentText().stripWhiteSpace().isEmpty() ) {
                QFileInfo info( d->url.path() );
                if ( info.isDir() ) {
                    d->filenames = QString::null;
                    d->urlList.clear();
                    d->urlList.append( d->url );
                    accept();
                }
                else if (!info.exists() && (mode() & KFile::File) != KFile::File) {
                    // directory doesn't exist, create and enter it
                    if ( ops->mkdir( d->url.url(), true ))
                        return;
                    else
                        accept();
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
    //             qDebug( "**** Selected remote directory: %s", d->url.url().latin1());
    //             d->filenames = QString::null;
    //             d->urlList.clear();
    //             d->urlList.append( d->url );
    //             accept();
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
            KIO::Scheduler::scheduleJob( job );
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
        if ( count == 0 ) {
            locationEdit->clearEdit();
            locationEdit->lineEdit()->setEdited( false );
            setURL( sJob->url() );
        }
        d->statJobs.clear();
        return;
    }

    kdDebug(kfile_area) << "filename " << sJob->url().url() << endl;

    if ( count == 0 )
        accept();
}


void KFileDialog::accept()
{
    setResult( QDialog::Accepted ); // parseSelectedURLs() checks that

    *lastDirectory = ops->url();
    if (!d->fileClass.isEmpty())
       KRecentDirs::add(d->fileClass, ops->url().url());

    // clear the topmost item, we insert it as full path later on as item 1
    locationEdit->changeItem( QString::null, 0 );

    KURL::List list = selectedURLs();
    QValueListConstIterator<KURL> it = list.begin();
    for ( ; it != list.end(); ++it ) {
        const KURL& url = *it;
        // we strip the last slash (-1) because KURLComboBox does that as well
        // when operating in file-mode. If we wouldn't , dupe-finding wouldn't
        // work.
        QString file = url.isLocalFile() ? url.path(-1) : url.prettyURL(-1);

        // remove dupes
        for ( int i = 1; i < locationEdit->count(); i++ ) {
            if ( locationEdit->text( i ) == file ) {
                locationEdit->removeItem( i-- );
                break;
            }
        }
        locationEdit->insertItem( file, 1 );
    }

    KConfig *config = KGlobal::config();
    config->setForceGlobal( true );
    writeConfig( config, ConfigGroup );
    config->setForceGlobal( false );

    saveRecentFiles( config );
    config->sync();

    KDialogBase::accept();

    addToRecentDocuments();

    if ( (mode() & KFile::Files) != KFile::Files ) // single selection
        emit fileSelected(d->url.url());

    ops->close();
    emit okClicked();
}


void KFileDialog::fileHighlighted(const KFileItem *i)
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
            locationEdit->lineEdit()->setEdited( false );
        }
        emit fileHighlighted(d->url.url());
    }

    else {
        multiSelectionChanged();
        emit selectionChanged();
    }
}

void KFileDialog::fileSelected(const KFileItem *i)
{
    if (i && i->isDir())
        return;

    if ( (ops->mode() & KFile::Files) != KFile::Files ) {
        if ( !i )
            return;

        d->url = i->url();
        locationEdit->setCurrentItem( 0 );
        locationEdit->setEditText( i->name() );
        locationEdit->lineEdit()->setEdited( false );
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

    locationEdit->lineEdit()->setEdited( false );
    KFileItem *item;
    const KFileItemList *list = ops->selectedItems();
    if ( !list ) {
        locationEdit->clearEdit();
        return;
    }

    static const QString &begin = KGlobal::staticQString(" \"");
    KFileItemListIterator it ( *list );
    QString text;
    while ( (item = it.current()) ) {
        text.append( begin ).append( item->name() ).append( '\"' );
        ++it;
    }
    locationEdit->setCurrentItem( 0 );
    locationEdit->setEditText( text.stripWhiteSpace() );
}

void KFileDialog::init(const QString& startDir, const QString& filter, QWidget* widget)
{
    d = new KFileDialogPrivate();
    d->boxLayout = 0;
    d->keepLocation = false;
    d->operationMode = Opening;
    d->hasDefaultFilter = false;
    d->hasView = false;
    d->mainWidget = new QWidget( this, "KFileDialog::mainWidget");
    setMainWidget( d->mainWidget );
    d->okButton = new KPushButton( KStdGuiItem::ok(), d->mainWidget );
    d->okButton->setDefault( true );
    d->cancelButton = new KPushButton(KStdGuiItem::cancel(), d->mainWidget);
    connect( d->okButton, SIGNAL( clicked() ), SLOT( slotOk() ));
    connect( d->cancelButton, SIGNAL( clicked() ), SLOT( slotCancel() ));
    d->urlBar = new KURLBar( true, d->mainWidget, "url bar" );
    connect( d->urlBar, SIGNAL( activated( const KURL& )),
             SLOT( enterURL( const KURL& )) );
    d->customWidget = widget;

    KConfig *config = KGlobal::config();
    KConfigGroupSaver cs( config, ConfigGroup );
    d->initializeSpeedbar = config->readBoolEntry( "Set speedbar defaults",
                                                   true );
    d->urlBar->readConfig( config, "KFileDialog Speedbar" );

    if ( d->initializeSpeedbar ) {
        KURL u;
        u.setPath( KGlobalSettings::desktopPath() );
        d->urlBar->insertItem( u, i18n("Desktop"), false );

        if (KGlobalSettings::documentPath() != QDir::homeDirPath())
        {
            u.setPath( KGlobalSettings::documentPath() );
            d->urlBar->insertItem( u, i18n("Documents"), false, "document" );
        }
        
        u.setPath( QDir::homeDirPath() );
        d->urlBar->insertItem( u, i18n("Home Directory"), false,
                               "folder_home" );
        u = "floppy:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            d->urlBar->insertItem( u, i18n("Floppy"), false,
                                   KProtocolInfo::icon( "floppy" ) );
        QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
        u.setProtocol( "file" );
        u.setPath( tmpDirs.isEmpty() ? QString("/tmp") : tmpDirs.first() );
        d->urlBar->insertItem( u, i18n("Temporary Files"), false,
                               "file_temporary" );
        u = "lan:/";
        if ( KProtocolInfo::isKnownProtocol( u ) )
            d->urlBar->insertItem( u, i18n("Network"), false,
                                   "network_local" );
    }

    d->completionLock = false;
    
    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );
    toolbar = new KToolBar( d->mainWidget, "KFileDialog::toolbar", true);
    toolbar->setFlat(true);
    qInstallMsgHandler( oldHandler );

    QString autocompletionWhatsThisText = i18n("<p>While typing in the text area you may be presented "
                                               "with possible matches. "
                                               "This feature can be controlled by clicking with the right mouse button "
                                               "and selecting a prefered mode from the <b>Text Completion</b> menu.") + "</qt>";
    d->pathCombo = new KURLComboBox( KURLComboBox::Directories, true,
                                     toolbar, "path combo" );
    QToolTip::add( d->pathCombo, i18n("Often used directories") );
    QWhatsThis::add( d->pathCombo, i18n("<qt>Commonly used locations are listed here. "
                                        "This includes standard locations such as your home directory as well as "
                                        "locations that have been visited recently.") + autocompletionWhatsThisText);
    
    KURL u;
    u.setPath( QDir::rootDirPath() );
    QString text = i18n("Root Directory: %1").arg( u.path() );
    d->pathCombo->addDefaultURL( u, 
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    u.setPath( QDir::homeDirPath() );
    text = i18n("Home Directory: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    KURL docPath;
    docPath.setPath( KGlobalSettings::documentPath() );
    if ( u.path(+1) != docPath.path(+1) ) {
        text = i18n("Documents: %1").arg( docPath.path( +1 ) );
        d->pathCombo->addDefaultURL( u, 
                                     KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                     text );
    }

    u.setPath( KGlobalSettings::desktopPath() );
    text = i18n("Desktop: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u, 
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ), 
                                 text );

    u.setPath( "/tmp" );

    if (!lastDirectory)
    {
        lastDirectory = ldd.setObject(new KURL());
    }

    bool defaultStartDir = startDir.isEmpty();
    if ( !defaultStartDir )
        if (startDir[0] == ':')
        {
            d->fileClass = startDir;
            d->url = KRecentDirs::dir(d->fileClass);
        }
        else
        {
            d->url = KCmdLineArgs::makeURL( QFile::encodeName(startDir) );
            // If we won't be able to list it (e.g. http), then use default
            if ( !KProtocolInfo::supportsListing( d->url.protocol() ) )
                defaultStartDir = true;
        }

    if ( defaultStartDir )
    {
        if (lastDirectory->isEmpty()) {
            *lastDirectory = KGlobalSettings::documentPath();
            KURL home;
            home.setPath( QDir::homeDirPath() );
            // if there is no docpath set (== home dir), we prefer the current
            // directory over it. We also prefer the homedir when our CWD is
            // different from our homedirectory
            if ( lastDirectory->path(+1) == home.path(+1) ||
                 QDir::currentDirPath() != QDir::homeDirPath() )
                *lastDirectory = QDir::currentDirPath();
        }
        d->url = *lastDirectory;
    }

    ops = new KDirOperator(d->url, d->mainWidget, "KFileDialog::ops");
    ops->setOnlyDoubleClickSelectsFiles( true );
    connect(ops, SIGNAL(urlEntered(const KURL&)),
            SLOT(urlEntered(const KURL&)));
    connect(ops, SIGNAL(fileHighlighted(const KFileItem *)),
            SLOT(fileHighlighted(const KFileItem *)));
    connect(ops, SIGNAL(fileSelected(const KFileItem *)),
            SLOT(fileSelected(const KFileItem *)));
    connect(ops, SIGNAL(finishedLoading()),
            SLOT(slotLoadingFinished()));

    ops->setupMenu(KDirOperator::SortActions |
                   KDirOperator::FileActions |
                   KDirOperator::ViewActions);
    KActionCollection *coll = ops->actionCollection();
    
    // plug nav items into the toolbar
    coll->action( "up" )->plug( toolbar );
    coll->action( "up" )->setWhatsThis(i18n("<qt>Click this button to enter the parent directory.<p>"
                                            "For instance if the current location is file:/home/%1 clicking this "
                                            " button will take you to file:/home.</qt>").arg(getlogin()));
    coll->action( "back" )->plug( toolbar );
    coll->action( "back" )->setWhatsThis(i18n("Click this button to move backwards one step in the browsing history."));
    coll->action( "forward" )->plug( toolbar );
    coll->action( "forward" )->setWhatsThis(i18n("Click this button to move forward one step in the browsing history."));
    coll->action( "reload" )->plug( toolbar );
    coll->action( "reload" )->setWhatsThis(i18n("Click this button to reload the contents of the current location."));
    coll->action( "mkdir" )->setShortcut(Key_F10);
    coll->action( "mkdir" )->plug( toolbar );
    coll->action( "mkdir" )->setWhatsThis(i18n("Click this button to create a new directory."));
    
    d->bookmarkHandler = new KFileBookmarkHandler( this );
    toolbar->insertButton(QString::fromLatin1("bookmark"),
                          (int)HOTLIST_BUTTON, true,
                          i18n("Bookmarks"));
    toolbar->getButton(HOTLIST_BUTTON)->setPopup( d->bookmarkHandler->menu(),
                                                  true);
    QWhatsThis::add(toolbar->getButton(HOTLIST_BUTTON), 
                    i18n("<qt>This button allows you to bookmark specific locations. "
                         "Click on this button to open the bookmark menu where you may add, "
                         "edit or select a bookmark.<p>"
                         "These bookmarks are specific to the file dialog but otherwise operate "
                         "like bookmarks elsewhere in KDE.</qt>"));
    connect( d->bookmarkHandler, SIGNAL( openURL( const QString& )),
             SLOT( enterURL( const QString& )));
    
    KToggleAction *showSidebarAction =
        new KToggleAction(i18n("Show Quick Access Sidebar"), Key_F9, coll,"toggleSpeedbar");
    connect( showSidebarAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleSpeedbar( bool )) );

    KActionMenu *menu = new KActionMenu( i18n("Configure"), "configure", this, "extra menu" );
    menu->setWhatsThis(i18n("<qt>This is the configuration menu for the file dialog. "
                            "Various option can be accessed from this menu including: <ul>" 
                            "<li>how files are sorted in the list</li>"
                            "<li>types of views, including icon and list</li>"
                            "<li>showing of hidden files</li>"
                            "<li>the Quick Access sidebar</li>"
                            "<li>file previews</li>"
                            "<li>seperating directories from files</li></ul></qt>"));
    menu->insert( coll->action( "sorting menu" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "short view" )->setShortcut(Key_F6);
    menu->insert( coll->action( "short view" ));
    coll->action( "detailed view" )->setShortcut(Key_F7);
    menu->insert( coll->action( "detailed view" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "show hidden" )->setShortcut(Key_F8);
    menu->insert( coll->action( "show hidden" ));
    menu->insert( showSidebarAction );
    coll->action( "preview" )->setShortcut(Key_F11);
    menu->insert( coll->action( "preview" ));
    coll->action( "separate dirs" )->setShortcut(Key_F12);
    menu->insert( coll->action( "separate dirs" ));
    
    menu->setDelayed( false );
    connect( menu->popupMenu(), SIGNAL( aboutToShow() ),
             ops, SLOT( updateSelectionDependentActions() ));
    menu->plug( toolbar );
    
    /*
     * ugly little hack to have a 5 pixel space between the buttons
     * and the combo box
     */
    QWidget *spacerWidget = new QWidget(toolbar);
    spacerWidget->setMinimumWidth(spacingHint());
    spacerWidget->setMaximumWidth(spacingHint());
    d->m_pathComboIndex = toolbar->insertWidget(-1, -1, spacerWidget);
    toolbar->insertWidget(PATH_COMBO, 0, d->pathCombo);
    

    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setIconText(KToolBar::IconOnly);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->setMovingEnabled(false);
    toolbar->adjustSize();
    
    d->pathCombo->setCompletionObject( ops->dirCompletionObject(), false );

    connect( d->pathCombo, SIGNAL( urlActivated( const KURL&  )),
             this,  SLOT( enterURL( const KURL& ) ));
    connect( d->pathCombo, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( enterURL( const QString& ) ));
    connect( d->pathCombo, SIGNAL(textChanged( const QString& )),
             SLOT( pathComboChanged( const QString& ) ));
    connect( d->pathCombo, SIGNAL( completion( const QString& )),
             SLOT( dirCompletion( const QString& )));
    connect( d->pathCombo, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             d->pathCombo, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    QString whatsThisText;
    if (d->operationMode == KFileDialog::Saving)
    {
        whatsThisText = i18n("<qt>This is the name to save the file as.") + 
                        autocompletionWhatsThisText;
    }
    else if (ops->mode() & KFile::Files)
    {
        whatsThisText = i18n("<qt>This is the list of files to open. More than "
                             "one file can be specified by listing several "
                             "files seperated by spaces.") +
                        autocompletionWhatsThisText;    
    }
    else
    {
        whatsThisText = i18n("<qt>This is the name of the file to open.") +
                        autocompletionWhatsThisText;
    }
    
    // the Location label/edit
    d->locationLabel = new QLabel(i18n("&Location:"), d->mainWidget);
    QWhatsThis::add(d->locationLabel, whatsThisText);
    locationEdit = new KURLComboBox(KURLComboBox::Files, true, 
                                    d->mainWidget, "LocationEdit");
    d->locationLabel->setBuddy(locationEdit);
    QWhatsThis::add(locationEdit, whatsThisText);
    // to get the completionbox-signals connected:
    locationEdit->setHandleSignals( true );
    (void) locationEdit->completionBox();

    locationEdit->setFocus();
//     locationEdit->setCompletionObject( new KURLCompletion() );
//     locationEdit->setAutoDeleteCompletionObject( true );
    locationEdit->setCompletionObject( ops->completionObject(), false );

    connect( locationEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotOk()));
    connect(locationEdit, SIGNAL( activated( const QString&  )),
            this,  SLOT( locationActivated( const QString& ) ));
    connect( locationEdit, SIGNAL( completion( const QString& )),
             SLOT( fileCompletion( const QString& )));
    connect( locationEdit, SIGNAL( textRotation(KCompletionBase::KeyBindingType) ),
             locationEdit, SLOT( rotateText(KCompletionBase::KeyBindingType) ));

    // the Filter label/edit
    whatsThisText = i18n("<qt>This is the filter to apply to the file list. "
                         "File names that do not match the filter will not be shown<p>"
                         "You may select from one of the preset filters in the "
                         "drop down menu or you may enter a custom filter "
                         "directly into the text area.<p>"
                         "Wildcards such as * and ? are allowed.</qt>");
    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    QWhatsThis::add(d->filterLabel, whatsThisText);
    filterWidget = new KFileFilterCombo(d->mainWidget,
                                        "KFileDialog::filterwidget");
    QWhatsThis::add(filterWidget, whatsThisText);
    setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    connect(filterWidget, SIGNAL(filterChanged()), SLOT(slotFilterChanged()));

    initGUI(); // activate GM

    readRecentFiles( config );

    adjustSize();

    // we set the completionLock to avoid entering pathComboChanged() when
    // inserting the list of URLs into the combo.
    d->completionLock = true;
    readConfig( config, ConfigGroup );
    d->completionLock = false;

    // need to set the current url of the urlbar manually (not via urlEntered()
    // here, because the initial url of KDirOperator might be the same as the
    // one that will be set later (and then urlEntered() won't be emitted).
    // ### REMOVE THIS when KDirOperator's initial URL (in the c'tor) is gone.
    if ( d->urlBar )
        d->urlBar->setCurrentItem( d->url );
    setSelection(d->url.url()); // ### move that into show() as well?
}

void KFileDialog::initGUI()
{
    delete d->boxLayout; // deletes all sub layouts

    d->boxLayout = new QVBoxLayout( d->mainWidget, 0, KDialog::spacingHint());
    d->boxLayout->addWidget(toolbar, AlignTop);

    QBoxLayout *horiz = new QHBoxLayout( d->boxLayout );
    horiz->addWidget( d->urlBar );

    QVBoxLayout *vbox = new QVBoxLayout( horiz );

    vbox->addWidget(ops, 4);
    vbox->addSpacing(3);

    QGridLayout* lafBox= new QGridLayout(2, 3, KDialog::spacingHint());
    vbox->addLayout(lafBox, 0);
    lafBox->addWidget(d->locationLabel, 0, 0, AlignVCenter);
    lafBox->addWidget(locationEdit, 0, 1, AlignVCenter);
    lafBox->addWidget(d->okButton, 0, 2, AlignVCenter);

    lafBox->addWidget(d->filterLabel, 1, 0, AlignVCenter);
    lafBox->addWidget(filterWidget, 1, 1, AlignVCenter);
    lafBox->addWidget(d->cancelButton, 1, 2, AlignVCenter);

    lafBox->setColStretch(1, 4);
    
    vbox->addSpacing(3);

    setTabOrder(ops,  locationEdit);
    setTabOrder(locationEdit, filterWidget);
    setTabOrder(filterWidget, d->okButton);
    setTabOrder(d->okButton, d->cancelButton);
    setTabOrder(d->cancelButton, d->pathCombo);
    setTabOrder(d->pathCombo, ops);

    // If a custom widget was specified...
    if ( d->customWidget != 0 )
    {
        // ...add it to the dialog, below the filter list box.

        // Change the parent so that this widget is a child of the main widget
        d->customWidget->reparent( d->mainWidget, QPoint() );

        vbox->addWidget( d->customWidget );

        // FIXME: This should adjust the tab orders so that the custom widget
        // comes after the Cancel button. The code appears to do this, but the result
        // somehow screws up the tab order of the file path combo box. Not a major
        // problem, but ideally the tab order with a custom widget should be
        // the same as the order without one.
        setTabOrder(d->cancelButton, d->customWidget);
        setTabOrder(d->customWidget, d->pathCombo);
    }
    else
    {
        setTabOrder(d->cancelButton, d->pathCombo);
    }

    setTabOrder(d->pathCombo, ops);
}

void KFileDialog::slotFilterChanged()
{
    QString filter = filterWidget->currentFilter();
    ops->clearFilter();

    if ( filter.find( '/' ) > -1 ) {
        QStringList types = QStringList::split( " ", filter );
        types.prepend( "inode/directory" );
        ops->setMimeFilter( types );
    }
    else
        ops->setNameFilter( filter );

    ops->updateDir();
    emit filterChanged( filter );
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

    if ( d->urlBar )
        d->urlBar->setCurrentItem( url );
}

void KFileDialog::locationActivated( const QString& url )
{
    setSelection( url );
}

void KFileDialog::enterURL( const KURL& url)
{
    setURL( url );
}

void KFileDialog::enterURL( const QString& url )
{
    setURL( url );
}

void KFileDialog::toolbarCallback(int) // SLOT
{
    /*
     * yes, nothing uses this anymore. 
     * it used to be used to show the configure dialog
     */
}


void KFileDialog::setSelection(const QString& url)
{
    kdDebug(kfile_area) << "setSelection " << url << endl;

    if (url.isEmpty()) {
        d->selection = QString::null;
        return;
    }

    // This code is the same as the one in slotOK
    KURL u;
    if ( KURL::isRelativeURL(url) )
    {
        if (!url.isEmpty() && url[0] == '/' ) // absolute path
            u.setPath( url );
        else
        {
            u = ops->url();
            u.addPath( url ); // works for filenames and relative paths
        }
    } else // complete URL
        u = url;

    if (u.isMalformed()) { // if it still is
        kdWarning() << url << " is not a correct argument for setSelection!" << endl;
        return;
    }

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KFileItem i(KFileItem::Unknown, KFileItem::Unknown, u, true );
    //    KFileItem i(u.path());
    if ( i.isDir() && u.isLocalFile() && QFile::exists( u.path() ) ) {
        // trust isDir() only if the file is
        // local (we cannot stat non-local urls) and if it exists!
        // (as KFileItem does not check if the file exists or not
        // -> the statbuffer is undefined -> isDir() is unreliable) (Simon)
        setURL(u, true);
    }
    else {
        QString filename = u.url();
        int sep = filename.findRev('/');
        if (sep >= 0) { // there is a / in it
            if ( KProtocolInfo::supportsListing( u.protocol() ))
                setURL(filename.left(sep), true);

            // filename must be decoded, or "name with space" would become
            // "name%20with%20space", so we use KURL::fileName()
            filename = u.fileName();
            kdDebug(kfile_area) << "filename " << filename << endl;
            d->selection = filename;
            locationEdit->setCurrentItem( 0 );
            locationEdit->setEditText( filename );
        }

        d->url = ops->url();
        d->url.addPath(filename);
    }
}

void KFileDialog::slotLoadingFinished()
{
    if ( !d->selection.isNull() )
        ops->setCurrentItem( d->selection );
}


void KFileDialog::dirCompletion( const QString& dir ) // SLOT
{
    // we don't support popup completion here, sorry
    if ( ops->dirCompletionObject()->completionMode() ==
         KGlobalSettings::CompletionPopup )
        return;

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
        QString complete = ops->makeDirCompletion( url.fileName(false) );

        if (!complete.isNull()) {
	    if(!base.endsWith("/"))
	        base.append('/');
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
    if ( !text.isEmpty() ) {
        KCompletion *comp = ops->completionObject();
        if ( comp->completionMode() == KGlobalSettings::CompletionPopup ||
            comp->completionMode() == KGlobalSettings::CompletionPopupAuto )
            locationEdit->setCompletedItems( comp->allMatches() );
        else
            locationEdit->setCompletedText( text );
    }
    else
        if (locationEdit->completionMode() == KGlobalSettings::CompletionPopup ||
            locationEdit->completionMode() == KGlobalSettings::CompletionPopupAuto )
            locationEdit->completionBox()->hide();

    d->completionLock = false;
}

void KFileDialog::updateStatusLine(int /* dirs */, int /* files */)
{
    kdWarning() << "KFileDialog::updateStatusLine is deprecated! The status line no longer exists. Do not try and use it!" << endl;
}

QString KFileDialog::getOpenFileName(const QString& startDir,
                                     const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
    dlg.setOperationMode( Opening );

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);

    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedFile();
}

QStringList KFileDialog::getOpenFileNames(const QString& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files | KFile::LocalOnly);
    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedFiles();
}

KURL KFileDialog::getOpenURL(const QString& startDir, const QString& filter,
                                QWidget *parent, const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode( KFile::File );
    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedURL();
}

KURL::List KFileDialog::getOpenURLs(const QString& startDir,
                                          const QString& filter,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
    dlg.setOperationMode( Opening );

    dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedURLs();
}

QString KFileDialog::getExistingDirectory(const QString& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
    KFileDialog dlg(startDir, QString::null, parent, "filedialog", true);
    dlg.setMode(KFile::Directory | KFile::LocalOnly); // local for now
    // to get "All Directories" instead of "All Files" in the combo
    dlg.setFilter( QString::null );
    dlg.ops->clearHistory();
    dlg.setCaption(caption.isNull() ? i18n("Select Directory") : caption);
    dlg.exec();

    return dlg.selectedFile();
}

KURL KFileDialog::getImageOpenURL( const QString& startDir, QWidget *parent,
                                   const QString& caption)
{
    KFileDialog dlg(startDir,
		    KImageIO::pattern( KImageIO::Reading ),
		    parent, "filedialog", true);
    dlg.setOperationMode( Opening );
    dlg.setCaption( caption.isNull() ? i18n("Open") : caption );
    dlg.setMode( KFile::File );

    KImageFilePreview *ip = new KImageFilePreview( &dlg );
    dlg.setPreviewWidget( ip );
    dlg.exec();

    return dlg.selectedURL();
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
                                i18n("The chosen filename(s) don't\nappear to be valid."), i18n("Invalid Filename(s)") );
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

    if ( (count % 2) == 1 ) { // odd number of " -> error
        QWidget *that = const_cast<KFileDialog *>(this);
        KMessageBox::sorry(that, i18n("The requested filenames\n%1\ndon't look valid to me.\nMake sure every filename is enclosed in double quotes.").arg(line), i18n("Filename Error"));
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
    bool specialDir = dir.at(0) == ':';
    KFileDialog dlg( specialDir ? dir : QString::null, filter, parent, "filedialog", true);
    if ( !specialDir )
        dlg.setSelection( dir ); // may also be a filename

    dlg.setOperationMode( Saving );
    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

KURL KFileDialog::getSaveURL(const QString& dir, const QString& filter,
                             QWidget *parent, const QString& caption)
{
    bool specialDir = dir.at(0) == ':';
    KFileDialog dlg(specialDir ? dir : QString::null, filter, parent, "filedialog", true);
    if ( !specialDir )
    dlg.setSelection( dir ); // may also be a filename

    dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);
    dlg.setOperationMode( Saving );

    dlg.exec();

    KURL url = dlg.selectedURL();
    if (!url.isMalformed())
        KRecentDocument::add( url );

    return url;
}

void KFileDialog::show()
{
    if ( !d->hasView ) { // delayed view-creation
        ops->setView(KFile::Default);
        ops->clearHistory();
        d->hasView = true;
    }

    KDialogBase::show();
}

void KFileDialog::setMode( KFile::Mode m )
{
    ops->setMode(m);
    if ( ops->dirOnlyMode() ) {
        filterWidget->setDefaultFilter( i18n("*|All Directories") );
    }
    else {
        filterWidget->setDefaultFilter( i18n("*|All Files") );
    }
}

void KFileDialog::setMode( unsigned int m )
{
    setMode(static_cast<KFile::Mode>( m ));
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
    combo->setURLs( kc->readListEntry( RecentURLs ) );
    combo->setMaxItems( kc->readNumEntry( RecentURLsNumber,
                                          DefaultRecentURLsNumber ) );
    combo->setURL( ops->url() );
    autoDirectoryFollowing = kc->readBoolEntry( AutoDirectoryFollowing,
                                                DefaultDirectoryFollowing );

    KGlobalSettings::Completion cm = (KGlobalSettings::Completion)
			   kc->readNumEntry( PathComboCompletionMode,
					     KGlobalSettings::CompletionAuto );
    if ( cm != KGlobalSettings::completionMode() )
	combo->setCompletionMode( cm );

    cm = (KGlobalSettings::Completion)
         kc->readNumEntry( LocationComboCompletionMode,
                           KGlobalSettings::CompletionAuto );
    if ( cm != KGlobalSettings::completionMode() )
	locationEdit->setCompletionMode( cm );

    bool showSpeedbar = kc->readBoolEntry(ShowSpeedbar, true);
    toggleSpeedbar( showSpeedbar );
    ((KToggleAction *) actionCollection()->action("toggleSpeedbar"))->setChecked( showSpeedbar );

    int w1 = minimumSize().width();
    int w2 = toolbar->sizeHint().width() + 10;
    if (w1 < w2)
        setMinimumWidth(w2);

    QSize size = configDialogSize( group );
    resize( size );
    kc->setGroup( oldGroup );
}

void KFileDialog::writeConfig( KConfig *kc, const QString& group )
{
    if ( !kc )
        return;

    QString oldGroup = kc->group();
    if ( !group.isEmpty() )
        kc->setGroup( group );

    kc->writeEntry( RecentURLs, d->pathCombo->urls() );
    saveDialogSize( group, true );
    kc->writeEntry( PathComboCompletionMode, static_cast<int>(d->pathCombo->completionMode()) );
    kc->writeEntry(LocationComboCompletionMode, static_cast<int>(locationEdit->completionMode()) );
    kc->writeEntry( ShowSpeedbar, !d->urlBar->isHidden() );

    ops->writeConfig( kc, group );
    kc->setGroup( oldGroup );
}


void KFileDialog::readRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    locationEdit->setMaxItems( kc->readNumEntry( RecentFilesNumber,
                                                 DefaultRecentURLsNumber ) );
    locationEdit->setURLs( kc->readListEntry( RecentFiles ),
                           KURLComboBox::RemoveBottom );
    locationEdit->insertItem( QString::null, 0 ); // dummy item without pixmap
    locationEdit->setCurrentItem( 0 );

    kc->setGroup( oldGroup );
}

void KFileDialog::saveRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    kc->writeEntry( RecentFiles, locationEdit->urls() );

    kc->setGroup( oldGroup );
}

KPushButton * KFileDialog::okButton() const
{
    return d->okButton;
}

KPushButton * KFileDialog::cancelButton() const
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

void KFileDialog::setOperationMode( OperationMode mode )
{
    d->operationMode = mode;
    d->keepLocation = true;
    filterWidget->setEditable( !d->hasDefaultFilter || mode != Saving );
    d->okButton->setGuiItem( (mode == Saving) ? KStdGuiItem::save() : KStdGuiItem::ok() );
}

KFileDialog::OperationMode KFileDialog::operationMode() const
{
    return d->operationMode;
}

// adds the selected files/urls to 'recent documents'
void KFileDialog::addToRecentDocuments()
{
    int m = ops->mode();

    if ( m & KFile::LocalOnly ) {
        QStringList files = selectedFiles();
        QStringList::ConstIterator it = files.begin();
        for ( ; it != files.end(); ++it )
            KRecentDocument::add( *it );
    }

    else { // urls
        KURL::List urls = selectedURLs();
        KURL::List::ConstIterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            if ( (*it).isValid() )
                KRecentDocument::add( *it );
        }
    }
}

KActionCollection * KFileDialog::actionCollection() const
{
    return ops->actionCollection();
}

void KFileDialog::toggleSpeedbar( bool show )
{
    if ( show )
        d->urlBar->show();
    else
        d->urlBar->hide();
}

int KFileDialog::pathComboIndex()
{
    return d->m_pathComboIndex;
}

void KFileDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kfiledialog.moc"
#include "kpreviewwidgetbase.moc"
