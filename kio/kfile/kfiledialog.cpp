// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>

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

#include "kfiledialog.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <q3ptrcollection.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3ptrlist.h>
#include <qpixmap.h>
#include <qtextcodec.h>
#include <qtimer.h>
#include <q3filedialog.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kcharsets.h>
#include <kcmdlineargs.h>
#include <kcompletionbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kio/kservicetypefactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kmenu.h>
#include <kprotocolinfo.h>
#include <kpushbutton.h>
#include <krecentdirs.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kstaticdeleter.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kurl.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kuser.h>

#include "config-kfile.h"
#include "kpreviewwidgetbase.h"

#include <kdirselectdialog.h>
#include <kfileview.h>
#include <krecentdocument.h>
#include <kfilefiltercombo.h>
#include <kdiroperator.h>
#include <kimagefilepreview.h>

#include <kfilespeedbar.h>
#include <kfilebookmarkhandler.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

enum Buttons { HOTLIST_BUTTON,
               PATH_COMBO, CONFIGURE_BUTTON };

template class Q3PtrList<KIO::StatJob>;

namespace {
    static void silenceQToolBar(QtMsgType, const char *)
    {
    }
}

struct KFileDialogPrivate
{
    // the last selected url
    KURL url;

    // the selected filenames in multiselection mode -- FIXME
    QString filenames;

    // the name of the filename set by setSelection
    QString selection;

    // now following all kind of widgets, that I need to rebuild
    // the geometry management
    QBoxLayout *boxLayout;
    QWidget *mainWidget;

    QLabel *locationLabel;

    // @deprecated remove in KDE4
    QLabel *filterLabel;
    KURLComboBox *pathCombo;
    KPushButton *okButton, *cancelButton;
    KFileSpeedBar *urlBar;
    QHBoxLayout *urlBarLayout;
    QWidget *customWidget;

    // Automatically Select Extension stuff
    QCheckBox *autoSelectExtCheckBox;
    bool autoSelectExtChecked; // whether or not the _user_ has checked the above box
    QString extension; // current extension for this filter

    Q3PtrList<KIO::StatJob> statJobs;

    KURL::List urlList; //the list of selected urls

    QStringList mimetypes; //the list of possible mimetypes to save as

    // indicates if the location edit should be kept or cleared when changing
    // directories
    bool keepLocation :1;

    // the KDirOperators view is set in KFileDialog::show(), so to avoid
    // setting it again and again, we have this nice little boolean :)
    bool hasView :1;

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

    if (d->urlBar)
        d->urlBar->save( config );

    config->sync();

    delete d->bookmarkHandler; // Should be deleted before ops!
    delete ops;
    delete d;
}

void KFileDialog::setLocationLabel(const QString& text)
{
    d->locationLabel->setText(text);
}

void KFileDialog::setFilter(const QString& filter)
{
    int pos = filter.indexOf('/');

    // Check for an un-escaped '/', if found
    // interpret as a MIME filter.

    if (pos > 0 && filter[pos - 1] != '\\') {
        QStringList filters = QStringList::split( " ", filter );
        setMimeFilter( filters );
        return;
    }

    // Strip the escape characters from
    // escaped '/' characters.

    QString copy (filter);
    for (pos = 0; (pos = copy.indexOf("\\/", pos)) != -1; ++pos)
        copy.remove(pos, 1);

    ops->clearFilter();
    filterWidget->setFilter(copy);
    ops->setNameFilter(filterWidget->currentFilter());
    d->hasDefaultFilter = false;
    filterWidget->setEditable( true );

    updateAutoSelectExtension ();
}

QString KFileDialog::currentFilter() const
{
    return filterWidget->currentFilter();
}

void KFileDialog::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->mimetypes = mimeTypes;
    filterWidget->setMimeFilter( mimeTypes, defaultType );

    QStringList types = QStringList::split(" ", filterWidget->currentFilter());
    types.append( QLatin1String( "inode/directory" ));
    ops->clearFilter();
    ops->setMimeFilter( types );
    d->hasDefaultFilter = !defaultType.isEmpty();
    filterWidget->setEditable( !d->hasDefaultFilter ||
                               d->operationMode != Saving );

    updateAutoSelectExtension ();
}

void KFileDialog::clearFilter()
{
    d->mimetypes.clear();
    filterWidget->setFilter( QString::null );
    ops->clearFilter();
    d->hasDefaultFilter = false;
    filterWidget->setEditable( true );

    updateAutoSelectExtension ();
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

void KFileDialog::setPreviewWidget(const KPreviewWidgetBase *w) {
    ops->setPreviewWidget(w);
    ops->clearHistory();
    d->hasView = true;
}

KURL KFileDialog::getCompleteURL(const QString &_url)
{
    QString url = KShell::tildeExpand(_url);
    KURL u;

    if ( KURL::isRelativeURL(url) ) // only a full URL isn't relative. Even /path is.
    {
        if (!url.isEmpty() && !QDir::isRelativePath(url) ) // absolute path
            u.setPath( url );
        else
        {
            u = ops->url();
            u.addPath( url ); // works for filenames and relative paths
            u.cleanPath(); // fix "dir/.."
        }
    }
    else // complete URL
        u = url;

    return u;
}

// FIXME: check for "existing" flag here?
void KFileDialog::slotOk()
{
    kdDebug(kfile_area) << "slotOK\n";

    // a list of all selected files/directories (if any)
    // can only be used if the user didn't type any filenames/urls himself
    const KFileItemList *items = ops->selectedItems();

    if ( (mode() & KFile::Directory) != KFile::Directory ) {
        if ( locationEdit->currentText().trimmed().isEmpty() ) {
            if ( !items || items->isEmpty() )
            {
                QString msg;
                if ( d->operationMode == Saving )
                    msg = i18n("Please specify the filename to save to.");
                else
                    msg = i18n("Please select the file to open.");
                KMessageBox::information(this, msg);
                return;
            }

            // weird case: the location edit is empty, but there are
            // highlighted files
            else {

                bool multi = (mode() & KFile::Files) != 0;
                KFileItemListIterator it( *items );
                QString endQuote = QLatin1String("\" ");
                QString name, files;
                while ( it.current() ) {
                    name = (*it)->name();
                    if ( multi ) {
                        name.prepend( QLatin1Char( '"' ) );
                        name.append( endQuote );
                    }

                    files.append( name );
                    ++it;
                }
                setLocationText( files );
                return;
            }
        }
    }

    bool dirOnly = ops->dirOnlyMode();

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

        if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
             !d->url.isLocalFile() ) {
// ### after message freeze, add message for directories!
            KMessageBox::sorry( d->mainWidget,
                                i18n("You can only select local files."),
                                i18n("Remote Files Not Accepted") );
            return;
        }

        accept();
        return;
    }


    KURL selectedURL;

    if ( (mode() & KFile::Files) == KFile::Files ) {// multiselection mode
        QString locationText = locationEdit->currentText();
        if ( locationText.contains( '/' )) {
            // relative path? -> prepend the current directory
            KURL u( ops->url(), KShell::tildeExpand(locationText));
            if ( u.isValid() )
                selectedURL = u;
            else
                selectedURL = ops->url();
        }
        else // simple filename -> just use the current URL
            selectedURL = ops->url();
    }

    else {
        selectedURL = getCompleteURL(locationEdit->currentText());

        // appendExtension() may change selectedURL
        appendExtension (selectedURL);
    }

    if ( !selectedURL.isValid() ) {
       KMessageBox::sorry( d->mainWidget, i18n("%1\ndoes not appear to be a valid URL.\n").arg(d->url.url()), i18n("Invalid URL") );
       return;
    }

    if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
         !selectedURL.isLocalFile() ) {
        KMessageBox::sorry( d->mainWidget,
                            i18n("You can only select local files."),
                            i18n("Remote Files Not Accepted") );
        return;
    }

    d->url = selectedURL;

    // d->url is a correct URL now

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
        kdDebug(kfile_area) << "Directory" << endl;
        bool done = true;
        if ( d->url.isLocalFile() ) {
            if ( locationEdit->currentText().trimmed().isEmpty() ) {
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
                    if ( (mode() & KFile::File) == KFile::File ||
                        (mode() & KFile::Files) == KFile::Files )
                        done = false;
                }
            }
            else  // Directory mode, with file[s]/dir[s] selected
            {
                if ( mode() & KFile::ExistingOnly )
                {
                    if ( ops->dirOnlyMode() )
                    {
                        KURL fullURL(d->url, locationEdit->currentText());
                        if ( QFile::exists( fullURL.path() ) )
                        {
                            d->url = fullURL;
                            d->filenames = QString::null;
                            d->urlList.clear();
                            accept();
                            return;
                        }
                        else // doesn't exist -> reject
                            return;
                    }
                }

                d->filenames = locationEdit->currentText();
                accept(); // what can we do?
            }

        }
        else { // FIXME: remote directory, should we allow that?
//             qDebug( "**** Selected remote directory: %s", d->url.url().latin1());
            d->filenames = QString::null;
            d->urlList.clear();
            d->urlList.append( d->url );

            if ( mode() & KFile::ExistingOnly )
                done = false;
            else
                accept();
        }

        if ( done )
            return;
    }

    if (!KAuthorized::authorizeURLAction("open", KURL(), d->url))
    {
        QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, d->url.prettyURL());
        KMessageBox::error( d->mainWidget, msg);
        return;
    }

    KIO::StatJob *job = 0L;
    d->statJobs.clear();
    d->filenames = KShell::tildeExpand(locationEdit->currentText());

    if ( (mode() & KFile::Files) == KFile::Files &&
         !locationEdit->currentText().contains( '/' )) {
        kdDebug(kfile_area) << "Files\n";
        KURL::List list = parseSelectedURLs();
        for ( KURL::List::ConstIterator it = list.begin();
              it != list.end(); ++it )
        {
            if (!KAuthorized::authorizeURLAction("open", KURL(), *it))
            {
                QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, (*it).prettyURL());
                KMessageBox::error( d->mainWidget, msg);
                return;
            }
        }
        for ( KURL::List::ConstIterator it = list.begin();
              it != list.end(); ++it )
        {
            job = KIO::stat( *it, !(*it).isLocalFile() );
            job->setWindow (topLevelWidget());
            KIO::Scheduler::scheduleJob( job );
            d->statJobs.append( job );
            connect( job, SIGNAL( result(KIO::Job *) ),
                     SLOT( slotStatResult( KIO::Job *) ));
        }
        return;
    }

    job = KIO::stat(d->url,!d->url.isLocalFile());
    job->setWindow (topLevelWidget());
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
    if (sJob->error() && count == 0 && !ops->dirOnlyMode())
    {
        accept();
        return;
    }

    KIO::UDSEntry t = sJob->statResult();
    if (t.isDir())
    {
        if ( ops->dirOnlyMode() )
        {
            d->filenames = QString::null;
            d->urlList.clear();
            accept();
        }
        else // in File[s] mode, directory means error -> cd into it
        {
            if ( count == 0 ) {
                locationEdit->clearEdit();
                locationEdit->lineEdit()->setEdited( false );
                setURL( sJob->url() );
            }
        }
        d->statJobs.clear();
        return;
    }
    else if ( ops->dirOnlyMode() )
    {
        return; // ### error message?
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
    QList<KURL>::const_iterator it = list.begin();
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

        if ( !locationEdit->hasFocus() ) { // don't disturb while editing
            setLocationText( i->name() );
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
        setLocationText( i->name() );
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
    if ( locationEdit->hasFocus() ) // don't disturb
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
        text.append( begin ).append( item->name() ).append( QLatin1Char( '"' ) );
        ++it;
    }

    setLocationText( text.trimmed() );
}

void KFileDialog::setLocationText( const QString& text )
{
    // setCurrentItem() will cause textChanged() being emitted,
    // so slotLocationChanged() will be called. Make sure we don't clear
    // the KDirOperator's view-selection in there
    disconnect( locationEdit, SIGNAL( textChanged( const QString& ) ),
                this, SLOT( slotLocationChanged( const QString& ) ) );
    locationEdit->setCurrentItem( 0 );
    connect( locationEdit, SIGNAL( textChanged( const QString& ) ),
             SLOT( slotLocationChanged( const QString& )) );
    locationEdit->setEditText( text );

    // don't change selection when user has clicked on an item
    if ( d->operationMode == Saving && !locationEdit->isVisible())
       setNonExtSelection();
}

static QString autocompletionWhatsThisText = i18n("<p>While typing in the text area, you may be presented "
                                                  "with possible matches. "
                                                  "This feature can be controlled by clicking with the right mouse button "
                                                  "and selecting a preferred mode from the <b>Text Completion</b> menu.") + "</qt>";
void KFileDialog::updateLocationWhatsThis (void)
{
    QString whatsThisText;
    if (d->operationMode == KFileDialog::Saving)
    {
        whatsThisText = "<qt>" + i18n("This is the name to save the file as.") +
                             autocompletionWhatsThisText;
    }
    else if (ops->mode() & KFile::Files)
    {
        whatsThisText = "<qt>" + i18n("This is the list of files to open. More than "
                             "one file can be specified by listing several "
                             "files, separated by spaces.") +
                              autocompletionWhatsThisText;
    }
    else
    {
        whatsThisText = "<qt>" + i18n("This is the name of the file to open.") +
                             autocompletionWhatsThisText;
    }

    d->locationLabel->setWhatsThis(whatsThisText);
    locationEdit->setWhatsThis(whatsThisText);
}

void KFileDialog::init(const QString& startDir, const QString& filter, QWidget* widget)
{
    initStatic();
    d = new KFileDialogPrivate();

    d->boxLayout = 0;
    d->keepLocation = false;
    d->operationMode = Opening;
    d->bookmarkHandler = 0;
    d->hasDefaultFilter = false;
    d->hasView = false;
    d->mainWidget = new QWidget( this );
    d->mainWidget->setObjectName( "KFileDialog::mainWidget" );
    setMainWidget( d->mainWidget );
    d->okButton = new KPushButton( KStdGuiItem::ok(), d->mainWidget );
    d->okButton->setDefault( true );
    d->cancelButton = new KPushButton(KStdGuiItem::cancel(), d->mainWidget);
    connect( d->okButton, SIGNAL( clicked() ), SLOT( slotOk() ));
    connect( d->cancelButton, SIGNAL( clicked() ), SLOT( slotCancel() ));
    d->customWidget = widget;
    d->autoSelectExtCheckBox = 0; // delayed loading
    d->autoSelectExtChecked = false;
    d->urlBar = 0; // delayed loading

    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );
    toolbar = new KToolBar( d->mainWidget, "KFileDialog::toolbar", true);
    toolbar->setFlat(true);
    qInstallMsgHandler( oldHandler );

    d->pathCombo = new KURLComboBox( KURLComboBox::Directories, true,
                                     toolbar);
    d->pathCombo->setToolTip( i18n("Current location") );
    d->pathCombo->setWhatsThis("<qt>" + i18n("This is the currently listed location. "
                                                 "The drop-down list also lists commonly used locations. "
                                                 "This includes standard locations, such as your home folder, as well as "
                                                 "locations that have been visited recently.") + autocompletionWhatsThisText);

    KURL u;
    u.setPath( QDir::rootPath() );
    QString text = i18n("Root Folder: %1").arg( u.path() );
    d->pathCombo->addDefaultURL( u,
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    u.setPath( QDir::homePath() );
    text = i18n("Home Folder: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u, KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    KURL docPath;
    docPath.setPath( KGlobalSettings::documentPath() );
    if ( (u.path(+1) != docPath.path(+1)) &&
         QDir(docPath.path(+1)).exists() )
    {
        text = i18n("Documents: %1").arg( docPath.path( +1 ) );
        d->pathCombo->addDefaultURL( docPath,
                                     KMimeType::pixmapForURL( docPath, 0, KIcon::Small ),
                                     text );
    }

    u.setPath( KGlobalSettings::desktopPath() );
    text = i18n("Desktop: %1").arg( u.path( +1 ) );
    d->pathCombo->addDefaultURL( u,
                                 KMimeType::pixmapForURL( u, 0, KIcon::Small ),
                                 text );

    d->url = getStartURL( startDir, d->fileClass );
    d->selection = d->url.url();

    // If local, check it exists. If not, go up until it exists.
    if ( d->url.isLocalFile() )
    {
        if ( !QFile::exists( d->url.path() ) )
        {
            d->url = d->url.upURL();
            QDir dir( d->url.path() );
            while ( !dir.exists() )
            {
                d->url = d->url.upURL();
                dir.setPath( d->url.path() );
            }
        }
    }

    ops = new KDirOperator(d->url, d->mainWidget );
    ops->setObjectName( "KFileDialog::ops" );
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
    coll->action( "up" )->setWhatsThis(i18n("<qt>Click this button to enter the parent folder.<p>"
                                            "For instance, if the current location is file:/home/%1 clicking this "
                                            "button will take you to file:/home.</qt>").arg( KUser().loginName() ));
    coll->action( "back" )->plug( toolbar );
    coll->action( "back" )->setWhatsThis(i18n("Click this button to move backwards one step in the browsing history."));
    coll->action( "forward" )->plug( toolbar );
    coll->action( "forward" )->setWhatsThis(i18n("Click this button to move forward one step in the browsing history."));
    coll->action( "reload" )->plug( toolbar );
    coll->action( "reload" )->setWhatsThis(i18n("Click this button to reload the contents of the current location."));
    coll->action( "mkdir" )->setShortcut(Qt::Key_F10);
    coll->action( "mkdir" )->plug( toolbar );
    coll->action( "mkdir" )->setWhatsThis(i18n("Click this button to create a new folder."));

    KToggleAction *showSidebarAction =
        new KToggleAction(i18n("Show Quick Access Navigation Panel"), Qt::Key_F9, coll,"toggleSpeedbar");
    showSidebarAction->setCheckedState(i18n("Hide Quick Access Navigation Panel"));
    connect( showSidebarAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleSpeedbar( bool )) );

    KToggleAction *showBookmarksAction =
            new KToggleAction(i18n("Show Bookmarks"), 0, coll, "toggleBookmarks");
    showBookmarksAction->setCheckedState(i18n("Hide Bookmarks"));
    connect( showBookmarksAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleBookmarks( bool )) );

    KActionMenu *menu = new KActionMenu( i18n("Configure"), "configure", coll, "extra menu" );
    menu->setWhatsThis(i18n("<qt>This is the configuration menu for the file dialog. "
                            "Various options can be accessed from this menu including: <ul>"
                            "<li>how files are sorted in the list</li>"
                            "<li>types of view, including icon and list</li>"
                            "<li>showing of hidden files</li>"
                            "<li>the Quick Access navigation panel</li>"
                            "<li>file previews</li>"
                            "<li>separating folders from files</li></ul></qt>"));
    menu->insert( coll->action( "sorting menu" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "short view" )->setShortcut(Qt::Key_F6);
    menu->insert( coll->action( "short view" ));
    coll->action( "detailed view" )->setShortcut(Qt::Key_F7);
    menu->insert( coll->action( "detailed view" ));
    menu->insert( coll->action( "separator" ));
    coll->action( "show hidden" )->setShortcut(Qt::Key_F8);
    menu->insert( coll->action( "show hidden" ));
    menu->insert( showSidebarAction );
    menu->insert( showBookmarksAction );
    coll->action( "preview" )->setShortcut(Qt::Key_F11);
    menu->insert( coll->action( "preview" ));
    coll->action( "separate dirs" )->setShortcut(Qt::Key_F12);
    menu->insert( coll->action( "separate dirs" ));

    menu->setDelayed( false );
    connect( menu->popupMenu(), SIGNAL( aboutToShow() ),
             ops, SLOT( updateSelectionDependentActions() ));
    menu->plug( toolbar );

    //Insert a separator.
    KToolBarSeparator* spacerWidget = new KToolBarSeparator(Qt::Horizontal, false /*no line*/,
                                                            toolbar);
    d->m_pathComboIndex = toolbar->insertWidget(-1, -1, spacerWidget);
    toolbar->insertWidget(PATH_COMBO, 0, d->pathCombo);


    toolbar->setItemAutoSized (PATH_COMBO);
    toolbar->setIconText(KToolBar::IconOnly);
    toolbar->setBarPos(KToolBar::Top);
    toolbar->setMovingEnabled(false);
    toolbar->adjustSize();

    KURLCompletion *pathCompletionObj = new KURLCompletion( KURLCompletion::DirCompletion );
    d->pathCombo->setCompletionObject( pathCompletionObj );
    d->pathCombo->setAutoDeleteCompletionObject( true );

    connect( d->pathCombo, SIGNAL( urlActivated( const KURL&  )),
             this,  SLOT( enterURL( const KURL& ) ));
    connect( d->pathCombo, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( enterURL( const QString& ) ));

    QString whatsThisText;

    // the Location label/edit
    d->locationLabel = new QLabel(i18n("&Location:"), d->mainWidget);
    locationEdit = new KURLComboBox(KURLComboBox::Files, true,
                                    d->mainWidget);
    connect( locationEdit, SIGNAL( textChanged( const QString& ) ),
             SLOT( slotLocationChanged( const QString& )) );

    updateLocationWhatsThis ();
    d->locationLabel->setBuddy(locationEdit);

    locationEdit->setFocus();
    KURLCompletion *fileCompletionObj = new KURLCompletion( KURLCompletion::FileCompletion );
    QString dir = d->url.url(+1);
    pathCompletionObj->setDir( dir );
    fileCompletionObj->setDir( dir );
    locationEdit->setCompletionObject( fileCompletionObj );
    locationEdit->setAutoDeleteCompletionObject( true );
    connect( fileCompletionObj, SIGNAL( match( const QString& ) ),
             SLOT( fileCompletion( const QString& )) );

    connect( locationEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotOk()));
    connect(locationEdit, SIGNAL( activated( const QString&  )),
            this,  SLOT( locationActivated( const QString& ) ));

    // the Filter label/edit
    whatsThisText = i18n("<qt>This is the filter to apply to the file list. "
                         "File names that do not match the filter will not be shown.<p>"
                         "You may select from one of the preset filters in the "
                         "drop down menu, or you may enter a custom filter "
                         "directly into the text area.<p>"
                         "Wildcards such as * and ? are allowed.</qt>");
    d->filterLabel = new QLabel(i18n("&Filter:"), d->mainWidget);
    d->filterLabel->setWhatsThis(whatsThisText);
    filterWidget = new KFileFilterCombo(d->mainWidget);
    filterWidget->setWhatsThis(whatsThisText);
    setFilter(filter);
    d->filterLabel->setBuddy(filterWidget);
    connect(filterWidget, SIGNAL(filterChanged()), SLOT(slotFilterChanged()));

    // the Automatically Select Extension checkbox
    // (the text, visibility etc. is set in updateAutoSelectExtension(), which is called by readConfig())
    d->autoSelectExtCheckBox = new QCheckBox (d->mainWidget);
    connect(d->autoSelectExtCheckBox, SIGNAL(clicked()), SLOT(slotAutoSelectExtClicked()));

    initGUI(); // activate GM

    KConfig* config = KGlobal::config();
    readRecentFiles( config );

    adjustSize();

    ops->setViewConfig( config, ConfigGroup );
    readConfig( config, ConfigGroup );
    setSelection(d->selection);
}

void KFileDialog::initSpeedbar()
{
    d->urlBar = new KFileSpeedBar( d->mainWidget, "url bar" );
    connect( d->urlBar, SIGNAL( activated( const KURL& )),
             SLOT( enterURL( const KURL& )) );

    // need to set the current url of the urlbar manually (not via urlEntered()
    // here, because the initial url of KDirOperator might be the same as the
    // one that will be set later (and then urlEntered() won't be emitted).
    // ### REMOVE THIS when KDirOperator's initial URL (in the c'tor) is gone.
    d->urlBar->setCurrentItem( d->url );

    d->urlBarLayout->insertWidget( 0, d->urlBar );
}

void KFileDialog::initGUI()
{
    delete d->boxLayout; // deletes all sub layouts

    d->boxLayout = new QVBoxLayout( d->mainWidget, 0, KDialog::spacingHint());
    d->boxLayout->addWidget(toolbar, 0, Qt::AlignTop);

    d->urlBarLayout = new QHBoxLayout( d->boxLayout ); // needed for the urlBar that may appear
    QVBoxLayout *vbox = new QVBoxLayout( d->urlBarLayout );

    vbox->addWidget(ops, 4);
    vbox->addSpacing(3);

    QGridLayout* lafBox= new QGridLayout(2, 3, KDialog::spacingHint());

    lafBox->addWidget(d->locationLabel, 0, 0, Qt::AlignVCenter);
    lafBox->addWidget(locationEdit, 0, 1, Qt::AlignVCenter);
    lafBox->addWidget(d->okButton, 0, 2, Qt::AlignVCenter);

    lafBox->addWidget(d->filterLabel, 1, 0, Qt::AlignVCenter);
    lafBox->addWidget(filterWidget, 1, 1, Qt::AlignVCenter);
    lafBox->addWidget(d->cancelButton, 1, 2, Qt::AlignVCenter);

    lafBox->setColStretch(1, 4);

    vbox->addLayout(lafBox, 0);
    vbox->addSpacing(3);

    // add the Automatically Select Extension checkbox
    vbox->addWidget (d->autoSelectExtCheckBox);
    vbox->addSpacing (3);

    setTabOrder(ops, d->autoSelectExtCheckBox);
    setTabOrder (d->autoSelectExtCheckBox, locationEdit);
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
        vbox->addSpacing(3);

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

    if ( filter.indexOf( '/' ) > -1 ) {
        QStringList types = QStringList::split( " ", filter );
        types.prepend( "inode/directory" );
        ops->setMimeFilter( types );
    }
    else
        ops->setNameFilter( filter );

    ops->updateDir();

    updateAutoSelectExtension ();

    emit filterChanged( filter );
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

    QString dir = url.url(+1);
    static_cast<KURLCompletion*>( d->pathCombo->completionObject() )->setDir( dir );
    static_cast<KURLCompletion*>( locationEdit->completionObject() )->setDir( dir );

    if ( d->urlBar )
        d->urlBar->setCurrentItem( url );
}

void KFileDialog::locationActivated( const QString& url )
{
    // This guard prevents any URL _typed_ by the user from being interpreted
    // twice (by returnPressed/slotOk and here, activated/locationActivated)
    // after the user presses Enter.  Without this, _both_ setSelection and
    // slotOk would "u.addPath( url )" ...so instead we leave it up to just
    // slotOk....
    if (!locationEdit->lineEdit()->edited())
        setSelection( url );
}

void KFileDialog::enterURL( const KURL& url)
{
    setURL( url );
}

void KFileDialog::enterURL( const QString& url )
{
    setURL( KURL::fromPathOrURL( KURLCompletion::replacedPath( url, true, true )) );
}


void KFileDialog::setSelection(const QString& url)
{
    kdDebug(kfile_area) << "setSelection " << url << endl;

    if (url.isEmpty()) {
        d->selection = QString::null;
        return;
    }

    KURL u = getCompleteURL(url);
    if (!u.isValid()) { // if it still is
        kdWarning() << url << " is not a correct argument for setSelection!" << endl;
        return;
    }

//     #warning FIXME: http URLs, e.g. from KURLCombo

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
        int sep = filename.lastIndexOf('/');
        if (sep >= 0) { // there is a / in it
            if ( KProtocolInfo::supportsListing( u )) {
                KURL dir(u);
                dir.setQuery( QString::null );
                dir.setFileName( QString::null );
                setURL(dir, true );
            }

            // filename must be decoded, or "name with space" would become
            // "name%20with%20space", so we use KURL::fileName()
            filename = u.fileName();
            kdDebug(kfile_area) << "filename " << filename << endl;
            d->selection = filename;
            setLocationText( filename );

            // tell the line edit that it has been edited
            // otherwise we won't know this was set by the user
            // and it will be ignored if there has been an
            // auto completion. this caused bugs where automcompletion
            // would start, the user would pick something from the
            // history and then hit Ok only to get the autocompleted
            // selection. OOOPS.
            locationEdit->lineEdit()->setEdited( true );
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

void KFileDialog::fileCompletion( const QString& match )
{
    if ( match.isEmpty() && ops->view() )
        ops->view()->clearSelection();
    else
        ops->setCurrentItem( match );
}

void KFileDialog::slotLocationChanged( const QString& text )
{
    if ( text.isEmpty() && ops->view() )
        ops->view()->clearSelection();

    updateFilter();
}

QString KFileDialog::getOpenFileName(const QString& startDir,
                                     const QString& filter,
                                     QWidget *parent, const QString& caption)
{
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
    dlg.setOperationMode( Opening );

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setWindowTitle(caption.isNull() ? i18n("Open") : caption);

    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedFile();
}

QString KFileDialog::getOpenFileNameWId(const QString& startDir,
                                        const QString& filter,
                                        WId parent_id, const QString& caption)
{
    QWidget* parent = QWidget::find( parent_id );
    KFileDialog dlg(startDir, filter, parent, "filedialog", true);
#ifdef Q_WS_X11
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint( QX11Info::display(), dlg.winId(), parent_id );
#else
    // TODO
#endif

    dlg.setOperationMode( KFileDialog::Opening );

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.setWindowTitle(caption.isNull() ? i18n("Open") : caption);

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

    dlg.setWindowTitle(caption.isNull() ? i18n("Open") : caption);
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

    dlg.setWindowTitle(caption.isNull() ? i18n("Open") : caption);
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

    dlg.setWindowTitle(caption.isNull() ? i18n("Open") : caption);
    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    return dlg.selectedURLs();
}

KURL KFileDialog::getExistingURL(const QString& startDir,
                                       QWidget *parent,
                                       const QString& caption)
{
    return KDirSelectDialog::selectDirectory(startDir, false, parent, caption);
}

QString KFileDialog::getExistingDirectory(const QString& startDir,
                                          QWidget *parent,
                                          const QString& caption)
{
#ifdef Q_WS_WIN
    return Q3FileDialog::getExistingDirectory(startDir, parent, "getExistingDirectory",
                                             caption, true, true);
#else
    KURL url = KDirSelectDialog::selectDirectory(startDir, true, parent,
                                                 caption);
    if ( url.isValid() )
        return url.path();

    return QString::null;
#endif
}

KURL KFileDialog::getImageOpenURL( const QString& startDir, QWidget *parent,
                                   const QString& caption)
{
    QStringList mimetypes = KImageIO::mimeTypes( KImageIO::Reading );
    KFileDialog dlg(startDir,
                    mimetypes.join(" "),
                    parent, "filedialog", true);
    dlg.setOperationMode( Opening );
    dlg.setWindowTitle( caption.isNull() ? i18n("Open") : caption );
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

        if ( u.isValid() )
            d->urlList.append( u );
        else
            KMessageBox::error( d->mainWidget,
                                i18n("The chosen filenames do not\n"
                                     "appear to be valid."),
                                i18n("Invalid Filenames") );
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

    const int count = line.count( QLatin1Char( '"' ) );
    if ( count == 0 ) { // no " " -> assume one single file
        u.setFileName( line );
        if ( u.isValid() )
            urls.append( u );

        return urls;
    }

    if ( (count % 2) == 1 ) { // odd number of " -> error
        QWidget *that = const_cast<KFileDialog *>(this);
        KMessageBox::sorry(that, i18n("The requested filenames\n"
                                      "%1\n"
                                      "do not appear to be valid;\n"
                                      "make sure every filename is enclosed in double quotes.").arg(line),
                           i18n("Filename Error"));
        return urls;
    }

    int start = 0;
    int index1 = -1, index2 = -1;
    while ( true ) {
        index1 = line.indexOf( '"', start );
        index2 = line.indexOf( '"', index1 + 1 );

        if ( index1 < 0 )
            break;

        // get everything between the " "
        name = line.mid( index1 + 1, index2 - index1 - 1 );
        u.setFileName( name );
        if ( u.isValid() )
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
       else {
           KMessageBox::sorry( d->mainWidget,
                               i18n("You can only select local files."),
                               i18n("Remote Files Not Accepted") );
       }
    }
    return QString::null;
}

QStringList KFileDialog::selectedFiles() const
{
    QStringList list;

    if ( result() == QDialog::Accepted ) {
        if ( (ops->mode() & KFile::Files) == KFile::Files ) {
            KURL::List urls = parseSelectedURLs();
            QList<KURL>::const_iterator it = urls.begin();
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
    bool specialDir = (!dir.isEmpty()) && (dir.at(0) == ':');
    KFileDialog dlg( specialDir ? dir : QString::null, filter, parent, "filedialog", true);
    if ( !specialDir )
        dlg.setSelection( dir ); // may also be a filename

    dlg.setOperationMode( Saving );
    dlg.setMode( KFile::File );
    dlg.setWindowTitle(caption.isNull() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

QString KFileDialog::getSaveFileNameWId(const QString& dir, const QString& filter,
                                     WId parent_id,
                                     const QString& caption)
{
    bool specialDir = (!dir.isEmpty()) && (dir.at(0) == ':');
    QWidget* parent = QWidget::find( parent_id );
    KFileDialog dlg( specialDir ? dir : QString::null, filter, parent, "filedialog", true);
#ifdef Q_WS_X11
    if( parent == NULL && parent_id != 0 )
        XSetTransientForHint(QX11Info::display(), dlg.winId(), parent_id);
#else
    // TODO
#endif

    if ( !specialDir )
        dlg.setSelection( dir ); // may also be a filename

    dlg.setOperationMode( KFileDialog::Saving);
    dlg.setMode( KFile::File );
    dlg.setWindowTitle(caption.isNull() ? i18n("Save As") : caption);

    dlg.exec();

    QString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        KRecentDocument::add(filename);

    return filename;
}

KURL KFileDialog::getSaveURL(const QString& dir, const QString& filter,
                             QWidget *parent, const QString& caption)
{
    bool specialDir = (!dir.isEmpty()) && (dir.at(0) == ':');
    KFileDialog dlg(specialDir ? dir : QString::null, filter, parent, "filedialog", true);
    if ( !specialDir )
    dlg.setSelection( dir ); // may also be a filename

    dlg.setWindowTitle(caption.isNull() ? i18n("Save As") : caption);
    dlg.setOperationMode( Saving );
    dlg.setMode( KFile::File );

    dlg.exec();

    KURL url = dlg.selectedURL();
    if (url.isValid())
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
        filterWidget->setDefaultFilter( i18n("*|All Folders") );
    }
    else {
        filterWidget->setDefaultFilter( i18n("*|All Files") );
    }

    updateAutoSelectExtension ();
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
    combo->setURLs( kc->readPathListEntry( RecentURLs ), KURLComboBox::RemoveTop );
    combo->setMaxItems( kc->readNumEntry( RecentURLsNumber,
                                          DefaultRecentURLsNumber ) );
    combo->setURL( ops->url() );
    autoDirectoryFollowing = kc->readBoolEntry( AutoDirectoryFollowing,
                                                DefaultDirectoryFollowing );

    KGlobalSettings::Completion cm = (KGlobalSettings::Completion)
                                      kc->readNumEntry( PathComboCompletionMode,
                                      KGlobalSettings::completionMode() );
    if ( cm != KGlobalSettings::completionMode() )
        combo->setCompletionMode( cm );

    cm = (KGlobalSettings::Completion)
         kc->readNumEntry( LocationComboCompletionMode,
                           KGlobalSettings::completionMode() );
    if ( cm != KGlobalSettings::completionMode() )
        locationEdit->setCompletionMode( cm );

    // show or don't show the speedbar
    toggleSpeedbar( kc->readBoolEntry(ShowSpeedbar, true) );

    // show or don't show the bookmarks
    toggleBookmarks( kc->readBoolEntry(ShowBookmarks, false) );

    // does the user want Automatically Select Extension?
    d->autoSelectExtChecked = kc->readBoolEntry (AutoSelectExtChecked, DefaultAutoSelectExtChecked);
    updateAutoSelectExtension ();

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

    kc->writePathEntry( RecentURLs, d->pathCombo->urls() );
    saveDialogSize( group, true );
    kc->writeEntry( PathComboCompletionMode, static_cast<int>(d->pathCombo->completionMode()) );
    kc->writeEntry( LocationComboCompletionMode, static_cast<int>(locationEdit->completionMode()) );
    kc->writeEntry( ShowSpeedbar, d->urlBar && !d->urlBar->isHidden() );
    kc->writeEntry( ShowBookmarks, d->bookmarkHandler != 0 );
    kc->writeEntry( AutoSelectExtChecked, d->autoSelectExtChecked );

    ops->writeConfig( kc, group );
    kc->setGroup( oldGroup );
}


void KFileDialog::readRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    locationEdit->setMaxItems( kc->readNumEntry( RecentFilesNumber,
                                                 DefaultRecentURLsNumber ) );
    locationEdit->setURLs( kc->readPathListEntry( RecentFiles ),
                           KURLComboBox::RemoveBottom );
    locationEdit->insertItem( QString::null, 0 ); // dummy item without pixmap
    locationEdit->setCurrentItem( 0 );

    kc->setGroup( oldGroup );
}

void KFileDialog::saveRecentFiles( KConfig *kc )
{
    QString oldGroup = kc->group();
    kc->setGroup( ConfigGroup );

    kc->writePathEntry( RecentFiles, locationEdit->urls() );

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

KURLBar * KFileDialog::speedBar()
{
    return d->urlBar;
}

void KFileDialog::slotCancel()
{
    ops->close();
    KDialogBase::slotCancel();

    KConfig *config = KGlobal::config();
    config->setForceGlobal( true );
    writeConfig( config, ConfigGroup );
    config->setForceGlobal( false );
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
    d->keepLocation = (mode == Saving);
    filterWidget->setEditable( !d->hasDefaultFilter || mode != Saving );
    if ( mode == Opening )
       d->okButton->setGuiItem( KGuiItem( i18n("&Open"), "fileopen") );
    else if ( mode == Saving ) {
       d->okButton->setGuiItem( KStdGuiItem::save() );
       setNonExtSelection();
    }
    else
       d->okButton->setGuiItem( KStdGuiItem::ok() );
    updateLocationWhatsThis ();
    updateAutoSelectExtension ();
}

KFileDialog::OperationMode KFileDialog::operationMode() const
{
    return d->operationMode;
}

void KFileDialog::slotAutoSelectExtClicked()
{
    kdDebug (kfile_area) << "slotAutoSelectExtClicked(): "
                         << d->autoSelectExtCheckBox->isChecked () << endl;

    // whether the _user_ wants it on/off
    d->autoSelectExtChecked = d->autoSelectExtCheckBox->isChecked ();

    // update the current filename's extension
    updateLocationEditExtension (d->extension /* extension hasn't changed */);
}

static QString getExtensionFromPatternList (const QStringList &patternList)
{
    QString ret;
    kdDebug (kfile_area) << "\tgetExtension " << patternList << endl;

    QStringList::ConstIterator patternListEnd = patternList.end ();
    for (QStringList::ConstIterator it = patternList.begin ();
         it != patternListEnd;
         it++)
    {
        kdDebug (kfile_area) << "\t\ttry: \'" << (*it) << "\'" << endl;

        // is this pattern like "*.BMP" rather than useless things like:
        //
        // README
        // *.
        // *.*
        // *.JP*G
        // *.JP?
        if ((*it).startsWith ("*.") &&
            (*it).length () > 2 &&
            (*it).find ('*', 2) < 0 && (*it).find ('?', 2) < 0)
        {
            ret = (*it).mid (1);
            break;
        }
    }

    return ret;
}

static QString stripUndisplayable (const QString &string)
{
    QString ret = string;

    ret.remove (':');
    ret.remove ('&');

    return ret;
}


QString KFileDialog::currentFilterExtension (void)
{
    return d->extension;
}

void KFileDialog::updateAutoSelectExtension (void)
{
    if (!d->autoSelectExtCheckBox) return;

    //
    // Figure out an extension for the Automatically Select Extension thing
    // (some Windows users apparently don't know what to do when confronted
    // with a text file called "COPYING" but do know what to do with
    // COPYING.txt ...)
    //

    kdDebug (kfile_area) << "Figure out an extension: " << endl;
    QString lastExtension = d->extension;
    d->extension = QString::null;

    // Automatically Select Extension is only valid if the user is _saving_ a _file_
    if ((operationMode () == Saving) && (mode () & KFile::File))
    {
        //
        // Get an extension from the filter
        //

        QString filter = currentFilter ();
        if (!filter.isEmpty ())
        {
            // e.g. "*.cpp"
            if (filter.find ('/') < 0)
            {
                d->extension = getExtensionFromPatternList (QStringList::split (" ", filter)).toLower ();
                kdDebug (kfile_area) << "\tsetFilter-style: pattern ext=\'"
                                    << d->extension << "\'" << endl;
            }
            // e.g. "text/html"
            else
            {
                KMimeType::Ptr mime = KMimeType::mimeType (filter);

                // first try X-KDE-NativeExtension
                QString nativeExtension = mime->property ("X-KDE-NativeExtension").toString ();
                if (!nativeExtension.isEmpty() && nativeExtension.at (0) == '.')
                {
                    d->extension = nativeExtension.toLower ();
                    kdDebug (kfile_area) << "\tsetMimeFilter-style: native ext=\'"
                                         << d->extension << "\'" << endl;
                }

                // no X-KDE-NativeExtension
                if (d->extension.isEmpty ())
                {
                    d->extension = getExtensionFromPatternList (mime->patterns ()).toLower ();
                    kdDebug (kfile_area) << "\tsetMimeFilter-style: pattern ext=\'"
                                         << d->extension << "\'" << endl;
                }
            }
        }


        //
        // GUI: checkbox
        //

        QString whatsThisExtension;
        if (!d->extension.isEmpty ())
        {
            // remember: sync any changes to the string with below
            d->autoSelectExtCheckBox->setText (i18n ("Automatically select filename e&xtension (%1)").arg (d->extension));
            whatsThisExtension = i18n ("the extension <b>%1</b>").arg (d->extension);

            d->autoSelectExtCheckBox->setEnabled (true);
            d->autoSelectExtCheckBox->setChecked (d->autoSelectExtChecked);
        }
        else
        {
            // remember: sync any changes to the string with above
            d->autoSelectExtCheckBox->setText (i18n ("Automatically select filename e&xtension"));
            whatsThisExtension = i18n ("a suitable extension");

            d->autoSelectExtCheckBox->setChecked (false);
            d->autoSelectExtCheckBox->setEnabled (false);
        }

        const QString locationLabelText = stripUndisplayable (d->locationLabel->text ());
        const QString filterLabelText = stripUndisplayable (d->filterLabel->text ());
        d->autoSelectExtCheckBox->setWhatsThis(            "<qt>" +
                i18n (
                  "This option enables some convenient features for "
                  "saving files with extensions:<br>"
                  "<ol>"
                    "<li>Any extension specified in the <b>%1</b> text "
                    "area will be updated if you change the file type "
                    "to save in.<br>"
                    "<br></li>"
                    "<li>If no extension is specified in the <b>%2</b> "
                    "text area when you click "
                    "<b>Save</b>, %3 will be added to the end of the "
                    "filename (if the filename does not already exist). "
                    "This extension is based on the file type that you "
                    "have chosen to save in.<br>"
                    "<br>"
                    "If you do not want KDE to supply an extension for the "
                    "filename, you can either turn this option off or you "
                    "can suppress it by adding a period (.) to the end of "
                    "the filename (the period will be automatically "
                    "removed)."
                    "</li>"
                  "</ol>"
                  "If unsure, keep this option enabled as it makes your "
                  "files more manageable."
                    )
                .arg (locationLabelText)
                .arg (locationLabelText)
                .arg (whatsThisExtension)
            + "</qt>"
            );

        d->autoSelectExtCheckBox->show ();


        // update the current filename's extension
        updateLocationEditExtension (lastExtension);
    }
    // Automatically Select Extension not valid
    else
    {
        d->autoSelectExtCheckBox->setChecked (false);
        d->autoSelectExtCheckBox->hide ();
    }
}

// Updates the extension of the filename specified in locationEdit if the
// Automatically Select Extension feature is enabled.
// (this prevents you from accidently saving "file.kwd" as RTF, for example)
void KFileDialog::updateLocationEditExtension (const QString &lastExtension)
{
    if (!d->autoSelectExtCheckBox->isChecked () || d->extension.isEmpty ())
        return;

    QString urlStr = locationEdit->currentText ();
    if (urlStr.isEmpty ())
        return;

    KURL url = getCompleteURL (urlStr);
    kdDebug (kfile_area) << "updateLocationEditExtension (" << url << ")" << endl;

    const int fileNameOffset = urlStr.lastIndexOf ('/') + 1;
    QString fileName = urlStr.mid (fileNameOffset);

    const int dot = fileName.lastIndexOf ('.');
    const int len = fileName.length ();
    if (dot > 0 && // has an extension already and it's not a hidden file
                   // like ".hidden" (but we do accept ".hidden.ext")
        dot != len - 1 // and not deliberately suppressing extension
    )
    {
        // exists?
        KIO::UDSEntry t;
        if (KIO::NetAccess::stat (url, t, topLevelWidget()))
        {
            kdDebug (kfile_area) << "\tfile exists" << endl;

            if (t.isDir())
            {
                kdDebug (kfile_area) << "\tisDir - won't alter extension" << endl;
                return;
            }

            // --- fall through ---
        }


        //
        // try to get rid of the current extension
        //

        // catch "double extensions" like ".tar.gz"
        if (lastExtension.length () && fileName.endsWith (lastExtension))
            fileName.truncate (len - lastExtension.length ());
        // can only handle "single extensions"
        else
            fileName.truncate (dot);

        // add extension
        const QString newText = urlStr.left (fileNameOffset) + fileName + d->extension;
        if ( newText != locationEdit->currentText() )
        {
            locationEdit->setCurrentText (urlStr.left (fileNameOffset) + fileName + d->extension);
            locationEdit->lineEdit()->setEdited (true);
        }
    }
}

// Updates the filter if the extension of the filename specified in locationEdit is changed
// (this prevents you from accidently saving "file.kwd" as RTF, for example)
void KFileDialog::updateFilter ()
{
    if ((operationMode() == Saving) && (mode() & KFile::File) ) {
        const QString urlStr = locationEdit->currentText ();
        if (urlStr.isEmpty ())
            return;

        KMimeType::Ptr mime = KMimeType::findByPath(urlStr, 0, true);
        if (mime && mime->name() != KMimeType::defaultMimeType()) {
            if (filterWidget->currentFilter() != mime->name() &&
                filterWidget->filters().findIndex(mime->name()) != -1)
                filterWidget->setCurrentFilter(mime->name());
        }
    }
}

// applies only to a file that doesn't already exist
void KFileDialog::appendExtension (KURL &url)
{
    if (!d->autoSelectExtCheckBox->isChecked () || d->extension.isEmpty ())
        return;

    QString fileName = url.fileName ();
    if (fileName.isEmpty ())
        return;

    kdDebug (kfile_area) << "appendExtension(" << url << ")" << endl;

    const int len = fileName.length ();
    const int dot = fileName.lastIndexOf ('.');

    const bool suppressExtension = (dot == len - 1);
    const bool unspecifiedExtension = (dot <= 0);

    // don't KIO::NetAccess::Stat if unnecessary
    if (!(suppressExtension || unspecifiedExtension))
        return;

    // exists?
    KIO::UDSEntry t;
    if (KIO::NetAccess::stat (url, t, topLevelWidget()))
    {
        kdDebug (kfile_area) << "\tfile exists - won't append extension" << endl;
        return;
    }

    // suppress automatically append extension?
    if (suppressExtension)
    {
        //
        // Strip trailing dot
        // This allows lazy people to have autoSelectExtCheckBox->isChecked
        // but don't want a file extension to be appended
        // e.g. "README." will make a file called "README"
        //
        // If you really want a name like "README.", then type "README.."
        // and the trailing dot will be removed (or just stop being lazy and
        // turn off this feature so that you can type "README.")
        //
        kdDebug (kfile_area) << "\tstrip trailing dot" << endl;
        url.setFileName (fileName.left (len - 1));
    }
    // evilmatically append extension :) if the user hasn't specified one
    else if (unspecifiedExtension)
    {
        kdDebug (kfile_area) << "\tappending extension \'" << d->extension << "\'..." << endl;
        url.setFileName (fileName + d->extension);
        kdDebug (kfile_area) << "\tsaving as \'" << url << "\'" << endl;
    }
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

void KFileDialog::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Escape )
    {
        e->accept();
        d->cancelButton->animateClick();
    }
    else
        KDialogBase::keyPressEvent( e );
}

void KFileDialog::toggleSpeedbar( bool show )
{
    if ( show )
    {
        if ( !d->urlBar )
            initSpeedbar();

        d->urlBar->show();

        // check to see if they have a home item defined, if not show the home button
        KURLBarItem *urlItem = static_cast<KURLBarItem*>( d->urlBar->listBox()->firstItem() );
        KURL homeURL;
        homeURL.setPath( QDir::homePath() );
        while ( urlItem )
        {
            if ( homeURL.equals( urlItem->url(), true ) )
            {
                ops->actionCollection()->action( "home" )->unplug( toolbar );
                break;
            }

            urlItem = static_cast<KURLBarItem*>( urlItem->next() );
        }
    }
    else
    {
        if (d->urlBar)
            d->urlBar->hide();

        if ( !ops->actionCollection()->action( "home" )->isPlugged( toolbar ) )
            ops->actionCollection()->action( "home" )->plug( toolbar, 3 );
    }

    static_cast<KToggleAction *>(actionCollection()->action("toggleSpeedbar"))->setChecked( show );
}

void KFileDialog::toggleBookmarks(bool show)
{
    if (show)
    {
        if (d->bookmarkHandler)
        {
            return;
        }

        d->bookmarkHandler = new KFileBookmarkHandler( this );
        connect( d->bookmarkHandler, SIGNAL( openURL( const QString& )),
                    SLOT( enterURL( const QString& )));

        toolbar->insertButton(QLatin1String("bookmark"),
                              (int)HOTLIST_BUTTON, true,
                              i18n("Bookmarks"), 5);
        toolbar->getButton(HOTLIST_BUTTON)->setPopup(d->bookmarkHandler->menu(),
                                                     true);
        toolbar->getButton(HOTLIST_BUTTON)->setWhatsThis(                        i18n("<qt>This button allows you to bookmark specific locations. "
                                "Click on this button to open the bookmark menu where you may add, "
                                "edit or select a bookmark.<p>"
                                "These bookmarks are specific to the file dialog, but otherwise operate "
                                "like bookmarks elsewhere in KDE.</qt>"));
    }
    else if (d->bookmarkHandler)
    {
        delete d->bookmarkHandler;
        d->bookmarkHandler = 0;
        toolbar->removeItem(HOTLIST_BUTTON);
    }

    static_cast<KToggleAction *>(actionCollection()->action("toggleBookmarks"))->setChecked( show );
}

int KFileDialog::pathComboIndex()
{
    return d->m_pathComboIndex;
}

// static
void KFileDialog::initStatic()
{
    if ( lastDirectory )
        return;

    lastDirectory = ldd.setObject(lastDirectory, new KURL());
}

// static
KURL KFileDialog::getStartURL( const QString& startDir,
                               QString& recentDirClass )
{
    initStatic();

    recentDirClass = QString::null;
    KURL ret;

    bool useDefaultStartDir = startDir.isEmpty();
    if ( !useDefaultStartDir )
    {
        if (startDir[0] == ':')
        {
            recentDirClass = startDir;
            ret = KURL::fromPathOrURL( KRecentDirs::dir(recentDirClass) );
        }
        else
        {
            ret = KCmdLineArgs::makeURL( QFile::encodeName(startDir) );
            // If we won't be able to list it (e.g. http), then use default
            if ( !KProtocolInfo::supportsListing( ret ) )
                useDefaultStartDir = true;
        }
    }

    if ( useDefaultStartDir )
    {
        if (lastDirectory->isEmpty()) {
            lastDirectory->setPath(KGlobalSettings::documentPath());
            KURL home;
            home.setPath( QDir::homePath() );
            // if there is no docpath set (== home dir), we prefer the current
            // directory over it. We also prefer the homedir when our CWD is
            // different from our homedirectory or when the document dir
            // does not exist
            if ( lastDirectory->path(+1) == home.path(+1) ||
                 QDir::currentPath() != QDir::homePath() ||
                 !QDir(lastDirectory->path(+1)).exists() )
                lastDirectory->setPath(QDir::currentPath());
        }
        ret = *lastDirectory;
    }

    return ret;
}

void KFileDialog::setStartDir( const KURL& directory )
{
    initStatic();
    if ( directory.isValid() )
        *lastDirectory = directory;
}

void KFileDialog::setNonExtSelection()
{
    // Enhanced rename: Don't highlight the file extension.
    QString pattern, filename = locationEdit->currentText().trimmed();
    KServiceTypeFactory::self()->findFromPattern( filename, &pattern );

    if ( !pattern.isEmpty() && pattern.at( 0 ) == '*' && pattern.find( '*' , 1 ) == -1 )
       locationEdit->lineEdit()->setSelection( 0, filename.length() - pattern.trimmed().length()+1 );
    else
    {
       int lastDot = filename.lastIndexOf( '.' );
       if ( lastDot > 0 )
          locationEdit->lineEdit()->setSelection( 0, lastDot );
    }
}

void KFileDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }


#include "kfiledialog.moc"
