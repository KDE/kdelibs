// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>
                  2007 David Faure <faure@kde.org>

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

#include "kfilewidget.h"

#include "kfileplacesview.h"
#include "kfileplacesmodel.h"
#include "kfilebookmarkhandler.h"
#include "kurlcombobox.h"
#include "config-kfile.h"

#include <kactioncollection.h>
#include <kdiroperator.h>
#include <kdirselectdialog.h>
#include <kfilefiltercombo.h>
#include <kfileview.h>
#include <kimagefilepreview.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kpushbutton.h>
#include <krecentdocument.h>
#include <kstaticdeleter.h>
#include <ktoolbar.h>
#include <kurlcompletion.h>
#include <kuser.h>
#include <kprotocolmanager.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <krecentdirs.h>

#include <QCheckBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QFSFileEngine>
#include <kshell.h>
#include <kmessagebox.h>
#include <kauthorized.h>

namespace {
    static void silenceQToolBar(QtMsgType, const char *)
    {
    }
}

class KFileWidgetPrivate
{
public:
    KFileWidgetPrivate( KFileWidget* q )
        : boxLayout(0),
          customWidget(0),
          inAccept(false),
          q(q)
    {
    }
    void updateLocationWhatsThis();
    void updateAutoSelectExtension();
    void initSpeedbar();
    void initGUI();
    void readConfig(const KConfigGroup &configGroup);
    void writeConfig(KConfigGroup &configGroup);
    void setNonExtSelection();
    void setLocationText(const QString&);
    void appendExtension(KUrl &url);
    void updateLocationEditExtension(const QString &);
    void updateFilter();
    KUrl::List& parseSelectedUrls();
    /**
     * Parses the string "line" for files. If line doesn't contain any ", the
     * whole line will be interpreted as one file. If the number of " is odd,
     * an empty list will be returned. Otherwise, all items enclosed in " "
     * will be returned as correct urls.
     */
    KUrl::List tokenize(const QString& line) const;
    /**
     * Reads the recent used files and inserts them into the location combobox
     */
    void readRecentFiles( KConfig * );
    /**
     * Saves the entries from the location combobox.
     */
    void saveRecentFiles( KConfig * );
    /**
     * called when an item is highlighted/selected in multiselection mode.
     * handles setting the locationEdit.
     */
    void multiSelectionChanged();

    // the last selected url
    KUrl url;

    // the selected filenames in multiselection mode -- FIXME
    QString filenames;

    // the name of the filename set by setSelection
    QString selection;

    // now following all kind of widgets, that I need to rebuild
    // the geometry management
    QBoxLayout *boxLayout;
    QVBoxLayout *vbox;

    QLabel *locationLabel;

    // @deprecated remove in KDE4 -- err, remove what?
    QLabel *filterLabel;
    KUrlComboBox *pathCombo;
    KPushButton *okButton, *cancelButton;
    KFilePlacesView *placesView;
    QHBoxLayout *placesViewLayout;
    QWidget *customWidget;

    // Automatically Select Extension stuff
    QCheckBox *autoSelectExtCheckBox;
    bool autoSelectExtChecked; // whether or not the _user_ has checked the above box
    QString extension; // current extension for this filter

    QList<KIO::StatJob*> statJobs;

    KUrl::List urlList; //the list of selected urls

    QStringList mimetypes; //the list of possible mimetypes to save as

    // indicates if the location edit should be kept or cleared when changing
    // directories
    bool keepLocation;

    // the KDirOperators view is set in KFileWidget::show(), so to avoid
    // setting it again and again, we have this nice little boolean :)
    bool hasView;

    bool hasDefaultFilter; // necessary for the operationMode
    bool autoDirectoryFollowing;
    bool inAccept; // true between beginning and end of accept()

    KFileWidget::OperationMode operationMode;

    // The file class used for KRecentDirs
    QString fileClass;

    KFileBookmarkHandler *bookmarkHandler;

    // the QAction before of the path drop down so subclasses can place their custom widgets properly
    QAction* m_pathComboIndex;

    KActionMenu* bookmarkButton;
    KConfigGroup *viewConfigGroup;

    KToolBar *toolbar;
    KUrlComboBox *locationEdit;
    KDirOperator *ops;
    KFileFilterCombo *filterWidget;
    KFileWidget* q;
};

static KUrl *lastDirectory; // to set the start path

static KStaticDeleter<KUrl> ldd;
static void initStatic()
{
    if ( !lastDirectory )
        lastDirectory = ldd.setObject(lastDirectory, new KUrl());
}

static const char autocompletionWhatsThisText[] = I18N_NOOP("<p>While typing in the text area, you may be presented "
                                                  "with possible matches. "
                                                  "This feature can be controlled by clicking with the right mouse button "
                                                  "and selecting a preferred mode from the <b>Text Completion</b> menu.")  "</qt>";

KFileWidget::KFileWidget( const KUrl& startDir, QWidget *parent )
    : QWidget(parent), KAbstractFileWidget(), d(new KFileWidgetPrivate(this))
{
    initStatic();

    // TODO move most of this code for the KFileWidgetPrivate constructor
    d->keepLocation = false;
    d->operationMode = Opening;
    d->bookmarkHandler = 0;
    d->hasDefaultFilter = false;
    d->hasView = false;

    d->okButton = new KPushButton(KStandardGuiItem::ok(), this);
    d->okButton->setDefault( true );
    d->cancelButton = new KPushButton(KStandardGuiItem::cancel(), this);
    // The dialog shows them
    d->okButton->hide();
    d->cancelButton->hide();

    d->autoSelectExtCheckBox = 0; // delayed loading
    d->autoSelectExtChecked = false;
    d->placesView = 0; // delayed loading

    QtMsgHandler oldHandler = qInstallMsgHandler( silenceQToolBar );
    d->toolbar = new KToolBar( this, "KFileWidget::toolbar", true);
    d->toolbar->setMovable(false);

    qInstallMsgHandler( oldHandler );

    d->pathCombo = new KUrlComboBox( KUrlComboBox::Directories, true,
                                     d->toolbar);
    d->pathCombo->setToolTip( i18n("Current location") );
    d->pathCombo->setWhatsThis("<qt>" + i18n("This is the currently listed location. "
                                                 "The drop-down list also lists commonly used locations. "
                                                 "This includes standard locations, such as your home folder, as well as "
                                                 "locations that have been visited recently.") + i18n (autocompletionWhatsThisText));
    KUrl u;
    QString text;
#ifdef Q_WS_WIN
    foreach( const QFileInfo &drive,QFSFileEngine::drives() )
    {
        u.setPath( drive.filePath() );
        text = i18n("Drive: %1",  u.toLocalFile() );
        d->pathCombo->addDefaultUrl( u,
                                 KIO::pixmapForUrl( u, 0, K3Icon::Small ),
                                 text );
    }
#else
    u.setPath( QDir::rootPath() );
    text = i18n("Root Folder: %1",  u.toLocalFile() );
    d->pathCombo->addDefaultUrl( u,
                                 KIO::pixmapForUrl( u, 0, K3Icon::Small ),
                                 text );
#endif

    u.setPath( QDir::homePath() );
    text = i18n("Home Folder: %1",  u.path( KUrl::AddTrailingSlash ) );
    d->pathCombo->addDefaultUrl( u, KIO::pixmapForUrl( u, 0, K3Icon::Small ),
                                 text );

    KUrl docPath;
    docPath.setPath( KGlobalSettings::documentPath() );
    if ( (u.path(KUrl::AddTrailingSlash) != docPath.path(KUrl::AddTrailingSlash)) &&
          QDir(docPath.path(KUrl::AddTrailingSlash)).exists() )
    {
      text = i18n("Documents: %1",  docPath.path( KUrl::AddTrailingSlash ) );
        d->pathCombo->addDefaultUrl( docPath,
                                     KIO::pixmapForUrl( docPath, 0, K3Icon::Small ),
                                     text );
    }

    u.setPath( KGlobalSettings::desktopPath() );
    text = i18n("Desktop: %1",  u.path( KUrl::AddTrailingSlash ) );
    d->pathCombo->addDefaultUrl( u,
                                 KIO::pixmapForUrl( u, 0, K3Icon::Small ),
                                 text );

    d->url = getStartUrl( startDir, d->fileClass );
    d->selection = d->url.url();

    // If local, check it exists. If not, go up until it exists.
    if ( d->url.isLocalFile() )
    {
        if ( !QFile::exists( d->url.toLocalFile() ) )
        {
            d->url = d->url.upUrl();
            QDir dir( d->url.toLocalFile() );
            while ( !dir.exists() )
            {
                d->url = d->url.upUrl();
                dir.setPath( d->url.toLocalFile() );
            }
        }
    }

    d->ops = new KDirOperator(d->url, this );
    d->ops->setObjectName( "KFileWidget::ops" );
    d->ops->setOnlyDoubleClickSelectsFiles( true );
    connect(d->ops, SIGNAL(urlEntered(const KUrl&)),
            SLOT(urlEntered(const KUrl&)));
    connect(d->ops, SIGNAL(fileHighlighted(const KFileItem *)),
            SLOT(fileHighlighted(const KFileItem *)));
    connect(d->ops, SIGNAL(fileSelected(const KFileItem *)),
            SLOT(fileSelected(const KFileItem *)));
    connect(d->ops, SIGNAL(finishedLoading()),
            SLOT(slotLoadingFinished()));

    d->ops->setupMenu(KDirOperator::SortActions |
                   KDirOperator::FileActions |
                   KDirOperator::ViewActions);
    KActionCollection *coll = d->ops->actionCollection();

    // add nav items to the toolbar
    d->toolbar->addAction( coll->action( "up" ) );
    coll->action( "up" )->setWhatsThis(i18n("<qt>Click this button to enter the parent folder.<p>"
                                            "For instance, if the current location is file:/home/%1 clicking this "
                                            "button will take you to file:/home.</qt>",  KUser().loginName() ));
    d->toolbar->addAction( coll->action( "back" ) );
    coll->action( "back" )->setWhatsThis(i18n("Click this button to move backwards one step in the browsing history."));
    d->toolbar->addAction( coll->action( "forward" ) );
    coll->action( "forward" )->setWhatsThis(i18n("Click this button to move forward one step in the browsing history."));
    d->toolbar->addAction( coll->action( "reload" ) );
    coll->action( "reload" )->setWhatsThis(i18n("Click this button to reload the contents of the current location."));
    coll->action( "mkdir" )->setShortcut( QKeySequence(Qt::Key_F10) );
    d->toolbar->addAction( coll->action( "mkdir" ) );
    coll->action( "mkdir" )->setWhatsThis(i18n("Click this button to create a new folder."));

    KToggleAction *showSidebarAction =
        new KToggleAction(i18n("Show Quick Access Navigation Panel"), this);
    coll->addAction("toggleSpeedbar", showSidebarAction);
    showSidebarAction->setShortcut( QKeySequence(Qt::Key_F9) );
    showSidebarAction->setCheckedState(KGuiItem(i18n("Hide Quick Access Navigation Panel")));
    connect( showSidebarAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleSpeedbar( bool )) );

    KToggleAction *showBookmarksAction =
        new KToggleAction(i18n("Show Bookmarks"), this);
    coll->addAction("toggleBookmarks", showBookmarksAction);
    showBookmarksAction->setCheckedState(KGuiItem(i18n("Hide Bookmarks")));
    connect( showBookmarksAction, SIGNAL( toggled( bool ) ),
             SLOT( toggleBookmarks( bool )) );

    KActionMenu *menu = new KActionMenu( KIcon("configure"), i18n("Configure"), this);
    coll->addAction("extra menu", menu);
    menu->setWhatsThis(i18n("<qt>This is the configuration menu for the file dialog. "
                            "Various options can be accessed from this menu including: <ul>"
                            "<li>how files are sorted in the list</li>"
                            "<li>types of view, including icon and list</li>"
                            "<li>showing of hidden files</li>"
                            "<li>the Quick Access navigation panel</li>"
                            "<li>file previews</li>"
                            "<li>separating folders from files</li></ul></qt>"));
    menu->addAction( coll->action( "sorting menu" ));
    menu->addSeparator();
    coll->action( "short view" )->setShortcut( QKeySequence(Qt::Key_F6) );
    menu->addAction( coll->action( "short view" ));
    coll->action( "detailed view" )->setShortcut( QKeySequence(Qt::Key_F7) );
    menu->addAction( coll->action( "detailed view" ));
    menu->addSeparator();
    coll->action( "show hidden" )->setShortcut( QKeySequence(Qt::Key_F8) );
    menu->addAction( coll->action( "show hidden" ));
    menu->addAction( showSidebarAction );
    menu->addAction( showBookmarksAction );
    coll->action( "preview" )->setShortcut( QKeySequence(Qt::Key_F11) );
    menu->addAction( coll->action( "preview" ));
    coll->action( "separate dirs" )->setShortcut( QKeySequence(Qt::Key_F12) );
    menu->addAction( coll->action( "separate dirs" ));

    menu->setDelayed( false );
    connect( menu->menu(), SIGNAL( aboutToShow() ),
             d->ops, SLOT( updateSelectionDependentActions() ));
    d->toolbar->addAction( menu );

    //Insert a separator.
    d->m_pathComboIndex = d->toolbar->addSeparator();

    d->toolbar->addWidget(d->pathCombo);

    // FIXME KAction port - add capability
    //d->toolbar->setItemAutoSized (PATH_COMBO);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d->toolbar->setMovable(false);

    KUrlCompletion *pathCompletionObj = new KUrlCompletion( KUrlCompletion::DirCompletion );
    d->pathCombo->setCompletionObject( pathCompletionObj );
    d->pathCombo->setAutoDeleteCompletionObject( true );

    connect( d->pathCombo, SIGNAL( urlActivated( const KUrl&  )),
             this,  SLOT( enterUrl( const KUrl& ) ));
    connect( d->pathCombo, SIGNAL( returnPressed( const QString&  )),
             this,  SLOT( enterUrl( const QString& ) ));

    QString whatsThisText;

    // the Location label/edit
    d->locationLabel = new QLabel(i18n("&Location:"), this);
    d->locationEdit = new KUrlComboBox(KUrlComboBox::Files, true,
                                    this);
    connect( d->locationEdit, SIGNAL( textChanged( const QString& ) ),
             SLOT( slotLocationChanged( const QString& )) );

    d->updateLocationWhatsThis();
    d->locationLabel->setBuddy(d->locationEdit);

    d->locationEdit->setFocus();
    KUrlCompletion *fileCompletionObj = new KUrlCompletion( KUrlCompletion::FileCompletion );
    QString dir = d->url.url(KUrl::AddTrailingSlash);
    pathCompletionObj->setDir( dir );
    fileCompletionObj->setDir( dir );
    d->locationEdit->setCompletionObject( fileCompletionObj );
    d->locationEdit->setAutoDeleteCompletionObject( true );
    connect( fileCompletionObj, SIGNAL( match( const QString& ) ),
             SLOT( fileCompletion( const QString& )) );

    connect( d->locationEdit, SIGNAL( returnPressed() ),
             this, SLOT( slotOk()));
    connect(d->locationEdit, SIGNAL( activated( const QString&  )),
            this,  SLOT( locationActivated( const QString& ) ));

    // the Filter label/edit
    whatsThisText = i18n("<qt>This is the filter to apply to the file list. "
                         "File names that do not match the filter will not be shown.<p>"
                         "You may select from one of the preset filters in the "
                         "drop down menu, or you may enter a custom filter "
                         "directly into the text area.<p>"
                         "Wildcards such as * and ? are allowed.</qt>");
    d->filterLabel = new QLabel(i18n("&Filter:"), this);
    d->filterLabel->setWhatsThis(whatsThisText);
    d->filterWidget = new KFileFilterCombo(this);
    d->filterWidget->setWhatsThis(whatsThisText);
    d->filterLabel->setBuddy(d->filterWidget);
    connect(d->filterWidget, SIGNAL(filterChanged()), SLOT(slotFilterChanged()));

    // the Automatically Select Extension checkbox
    // (the text, visibility etc. is set in updateAutoSelectExtension(), which is called by readConfig())
    d->autoSelectExtCheckBox = new QCheckBox (this);
    connect(d->autoSelectExtCheckBox, SIGNAL(clicked()), SLOT(slotAutoSelectExtClicked()));

    d->initGUI(); // activate GM

    KSharedConfig::Ptr config = KGlobal::config();
    d->readRecentFiles(config.data());

    d->viewConfigGroup=new KConfigGroup(config,ConfigGroup);
    d->ops->setViewConfig(*d->viewConfigGroup);
    d->readConfig(* d->viewConfigGroup);
    setSelection(d->selection);
}

KFileWidget::~KFileWidget()
{
    KSharedConfig::Ptr config = KGlobal::config();

    config->sync();

    delete d->bookmarkHandler; // Should be deleted before ops!
    delete d->ops;
    delete d->viewConfigGroup;
    delete d;
}

void KFileWidget::setLocationLabel(const QString& text)
{
    d->locationLabel->setText(text);
}

void KFileWidget::setFilter(const QString& filter)
{
    int pos = filter.indexOf('/');

    // Check for an un-escaped '/', if found
    // interpret as a MIME filter.

    if (pos > 0 && filter[pos - 1] != '\\') {
        QStringList filters = filter.split(" ", QString::SkipEmptyParts); //QStringList::split( " ", filter );
        setMimeFilter( filters );
        return;
    }

    // Strip the escape characters from
    // escaped '/' characters.

    QString copy (filter);
    for (pos = 0; (pos = copy.indexOf("\\/", pos)) != -1; ++pos)
        copy.remove(pos, 1);

    d->ops->clearFilter();
    d->filterWidget->setFilter(copy);
    d->ops->setNameFilter(d->filterWidget->currentFilter());
    d->hasDefaultFilter = false;
    d->filterWidget->setEditable( true );

    d->updateAutoSelectExtension ();
}

QString KFileWidget::currentFilter() const
{
    return d->filterWidget->currentFilter();
}

void KFileWidget::setMimeFilter( const QStringList& mimeTypes,
                                 const QString& defaultType )
{
    d->mimetypes = mimeTypes;
    d->filterWidget->setMimeFilter( mimeTypes, defaultType );

    QStringList types = d->filterWidget->currentFilter().split(" ",QString::SkipEmptyParts); //QStringList::split(" ", d->filterWidget->currentFilter());
    types.append( QLatin1String( "inode/directory" ));
    d->ops->clearFilter();
    d->ops->setMimeFilter( types );
    d->hasDefaultFilter = !defaultType.isEmpty();
    d->filterWidget->setEditable( !d->hasDefaultFilter ||
                               d->operationMode != Saving );

    d->updateAutoSelectExtension ();
}

void KFileWidget::clearFilter()
{
    d->mimetypes.clear();
    d->filterWidget->setFilter( QString() );
    d->ops->clearFilter();
    d->hasDefaultFilter = false;
    d->filterWidget->setEditable( true );

    d->updateAutoSelectExtension ();
}

QString KFileWidget::currentMimeFilter() const
{
    int i = d->filterWidget->currentIndex();
    if (d->filterWidget->showsAllTypes())
        i--;

    if ((i >= 0) && (i < (int) d->mimetypes.count()))
        return d->mimetypes[i];
    return QString(); // The "all types" item has no mimetype
}

KMimeType::Ptr KFileWidget::currentFilterMimeType()
{
    return KMimeType::mimeType( currentMimeFilter() );
}

void KFileWidget::setPreviewWidget(const KPreviewWidgetBase *w) {
    d->ops->setPreviewWidget(w);
    d->ops->clearHistory();
    d->hasView = true;
}

KUrl KFileWidget::getCompleteUrl(const QString &_url)
{
    QString url = KShell::tildeExpand(_url);
    KUrl u;

    if ( KUrl::isRelativeUrl(url) ) // only a full URL isn't relative. Even /path is.
    {
        if (!url.isEmpty() && !QDir::isRelativePath(url) ) // absolute path
            u.setPath( url );
        else
        {
            u = d->ops->url();
            u.addPath( url ); // works for filenames and relative paths
            u.cleanPath(); // fix "dir/.."
        }
    }
    else // complete URL
        u = url;

    return u;
}

// Called by KFileDialog
void KFileWidget::slotOk()
{
    kDebug(kfile_area) << "slotOk\n";

    // a list of all selected files/directories (if any)
    // can only be used if the user didn't type any filenames/urls himself
    const KFileItemList *items = d->ops->selectedItems();

    if ( (mode() & KFile::Directory) != KFile::Directory ) {
        if ( d->locationEdit->currentText().trimmed().isEmpty() ) {
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
                KFileItemList::const_iterator kit = items->begin();
                const KFileItemList::const_iterator kend = items->end();
                QString endQuote = QLatin1String("\" ");
                QString name, files;
                for ( ; kit != kend; ++kit ) {
                    name = (*kit)->name();
                    if ( multi ) {
                        name.prepend( QLatin1Char( '"' ) );
                        name.append( endQuote );
                    }

                    files.append( name );
                }
                d->setLocationText( files );
                return;
            }
        }
    }

    bool dirOnly = d->ops->dirOnlyMode();

    // we can use our kfileitems, no need to parse anything
    if ( items && !d->locationEdit->lineEdit()->isModified() &&
         !(items->isEmpty() && !dirOnly) ) {

        d->urlList.clear();
        d->filenames.clear();

        if ( dirOnly ) {
            d->url = d->ops->url();
        }
        else {
            if ( !(mode() & KFile::Files) ) {// single selection
                d->url = items->first()->url();
            }

            else { // multi (dirs and/or files)
                d->url = d->ops->url();
                d->urlList = items->urlList();
            }
        }

        KUrl url = KIO::NetAccess::mostLocalUrl(d->url,topLevelWidget());
        if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
             !url.isLocalFile() ) {
// ### after message freeze, add message for directories!
            KMessageBox::sorry( this,
                                i18n("You can only select local files."),
                                i18n("Remote Files Not Accepted") );
            return;
        }

        d->url = url;
        emit accepted();
        return;
    }


    KUrl selectedUrl;

    if ( (mode() & KFile::Files) == KFile::Files ) {// multiselection mode
        QString locationText = d->locationEdit->currentText();
        if ( locationText.contains( '/' )) {
            // relative path? -> prepend the current directory
            KUrl u( d->ops->url(), KShell::tildeExpand(locationText));
            if ( u.isValid() )
                selectedUrl = u;
            else
                selectedUrl = d->ops->url();
        }
        else // simple filename -> just use the current URL
            selectedUrl = d->ops->url();
    }

    else {
        selectedUrl = getCompleteUrl(d->locationEdit->currentText());

        // appendExtension() may change selectedUrl
        d->appendExtension (selectedUrl);
    }

    if ( !selectedUrl.isValid() ) {
       KMessageBox::sorry( this, i18n("%1\ndoes not appear to be a valid URL.\n", d->url.url()), i18n("Invalid URL") );
       return;
    }

    KUrl url = KIO::NetAccess::mostLocalUrl(selectedUrl,topLevelWidget());
    if ( (mode() & KFile::LocalOnly) == KFile::LocalOnly &&
         !url.isLocalFile() ) {
        KMessageBox::sorry( this,
                            i18n("You can only select local files."),
                            i18n("Remote Files Not Accepted") );
        return;
    }

    d->url = url;

    // d->url is a correct URL now

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
        kDebug(kfile_area) << "Directory" << endl;
        bool done = true;
        if ( d->url.isLocalFile() ) {
            if ( d->locationEdit->currentText().trimmed().isEmpty() ) {
                QFileInfo info( d->url.toLocalFile() );
                if ( info.isDir() ) {
                    d->filenames.clear();
                    d->urlList.clear();
                    d->urlList.append( d->url );
                    emit accepted();
                }
                else if (!info.exists() && (mode() & KFile::File) != KFile::File) {
                    // directory doesn't exist, create and enter it
                    if ( d->ops->mkdir( d->url.url(), true ))
                        return;
                    else
                        emit accepted();
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
                    if ( d->ops->dirOnlyMode() )
                    {
                        KUrl fullURL(d->url, d->locationEdit->currentText());
                        if ( QFile::exists( fullURL.toLocalFile() ) )
                        {
                            d->url = fullURL;
                            d->filenames.clear();
                            d->urlList.clear();
                            emit accepted();
                            return;
                        }
                        else // doesn't exist -> reject
                            return;
                    }
                }

                d->filenames = d->locationEdit->currentText();
                emit accepted(); // what can we do?
            }

        }
        else { // FIXME: remote directory, should we allow that?
//             qDebug( "**** Selected remote directory: %s", d->url.url().toLatin1().constData());
            d->filenames.clear();
            d->urlList.clear();
            d->urlList.append( d->url );

            if ( mode() & KFile::ExistingOnly )
                done = false;
            else
                emit accepted();
        }

        if ( done )
            return;
    }

    if (!KAuthorized::authorizeUrlAction("open", KUrl(), d->url))
    {
        QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, d->url.prettyUrl());
        KMessageBox::error( this, msg);
        return;
    }

    KIO::StatJob *job = 0L;
    d->statJobs.clear();
    d->filenames = KShell::tildeExpand(d->locationEdit->currentText());

    if ( (mode() & KFile::Files) == KFile::Files &&
         !d->locationEdit->currentText().contains( '/' )) {
        kDebug(kfile_area) << "Files\n";
        KUrl::List list = d->parseSelectedUrls();
        for ( KUrl::List::ConstIterator it = list.begin();
              it != list.end(); ++it )
        {
            if (!KAuthorized::authorizeUrlAction("open", KUrl(), *it))
            {
                QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, (*it).prettyUrl());
                KMessageBox::error( this, msg);
                return;
            }
        }
        for ( KUrl::List::ConstIterator it = list.begin();
              it != list.end(); ++it )
        {
            job = KIO::stat( *it, !(*it).isLocalFile() );
            job->ui()->setWindow (topLevelWidget());
            KIO::Scheduler::scheduleJob( job );
            d->statJobs.append( job );
            connect( job, SIGNAL( result(KJob *) ),
                     SLOT( slotStatResult( KJob *) ));
        }
        return;
    }

    job = KIO::stat(d->url,!d->url.isLocalFile());
    job->ui()->setWindow (topLevelWidget());
    d->statJobs.append( job );
    connect(job, SIGNAL(result(KJob*)), SLOT(slotStatResult(KJob*)));
}

// FIXME : count all errors and show messagebox when d->statJobs.count() == 0
// in case of an error, we cancel the whole operation (clear d->statJobs and
// don't call accept)
void KFileWidget::slotStatResult(KJob* job)
{
    kDebug(kfile_area) << "slotStatResult" << endl;
    KIO::StatJob *sJob = static_cast<KIO::StatJob *>( job );

    if ( !d->statJobs.removeAll( sJob ) ) {
        return;
    }

    int count = d->statJobs.count();

    // errors mean in general, the location is no directory ;/
    // Can we be sure that it is exististant at all? (pfeiffer)
    if (sJob->error() && count == 0 && !d->ops->dirOnlyMode())
    {
        emit accepted();
        return;
    }

    KIO::UDSEntry t = sJob->statResult();
    if (t.isDir())
    {
        if ( d->ops->dirOnlyMode() )
        {
            d->filenames.clear();
            d->urlList.clear();
            emit accepted();
        }
        else // in File[s] mode, directory means error -> cd into it
        {
            if ( count == 0 ) {
                d->locationEdit->clearEditText();
                d->locationEdit->lineEdit()->setModified( false );
                setUrl( sJob->url() );
            }
        }
        d->statJobs.clear();
        return;
    }
    else if ( d->ops->dirOnlyMode() )
    {
        return; // ### error message?
    }

    kDebug(kfile_area) << "filename " << sJob->url().url() << endl;

    if ( count == 0 )
        emit accepted();
}

void KFileWidget::accept()
{
    d->inAccept = true; // parseSelectedUrls() checks that

    *lastDirectory = d->ops->url();
    if (!d->fileClass.isEmpty())
       KRecentDirs::add(d->fileClass, d->ops->url().url());

    // clear the topmost item, we insert it as full path later on as item 1
    d->locationEdit->setItemText( 0, QString() );

    KUrl::List list = selectedUrls();
    QList<KUrl>::const_iterator it = list.begin();
    for ( ; it != list.end(); ++it ) {
        const KUrl& url = *it;
        // we strip the last slash (-1) because KUrlComboBox does that as well
        // when operating in file-mode. If we wouldn't , dupe-finding wouldn't
        // work.
        QString file = url.isLocalFile() ? url.path(KUrl::RemoveTrailingSlash) : url.prettyUrl(KUrl::RemoveTrailingSlash);

        // remove dupes
        for ( int i = 1; i < d->locationEdit->count(); i++ ) {
            if ( d->locationEdit->itemText( i ) == file ) {
                d->locationEdit->removeItem( i-- );
                break;
            }
        }
        d->locationEdit->insertItem( 1,file);
    }

    KSharedConfig::Ptr config = KGlobal::config();
    config->setForceGlobal( true );
    KConfigGroup grp(config,ConfigGroup);
    d->writeConfig(grp);
    config->setForceGlobal( false );

    d->saveRecentFiles(config.data());
    config->sync();

    addToRecentDocuments();

    if ( (mode() & KFile::Files) != KFile::Files ) // single selection
        emit fileSelected(d->url.url());

    d->ops->close();
}


void KFileWidget::fileHighlighted(const KFileItem *i)
{
    if (i && i->isDir())
        return;

    if ( (d->ops->mode() & KFile::Files) != KFile::Files ) {
        if ( !i )
            return;

        d->url = i->url();

        if ( !d->locationEdit->hasFocus() ) { // don't disturb while editing
            d->setLocationText( i->name() );
        }
        emit fileHighlighted(d->url.url());
    }

    else {
        d->multiSelectionChanged();
        emit selectionChanged();
    }
}

void KFileWidget::fileSelected(const KFileItem *i)
{
    if (i && i->isDir())
        return;

    if ( (d->ops->mode() & KFile::Files) != KFile::Files ) {
        if ( !i )
            return;

        d->url = i->url();
        d->setLocationText( i->name() );
    }
    else {
        d->multiSelectionChanged();
        emit selectionChanged();
    }
    slotOk();
}


// I know it's slow to always iterate thru the whole filelist
// (d->ops->selectedItems()), but what can we do?
void KFileWidgetPrivate::multiSelectionChanged()
{
    if ( locationEdit->hasFocus() ) // don't disturb
        return;

    locationEdit->lineEdit()->setModified( false );
    const KFileItemList *list = ops->selectedItems();
    if ( !list ) {
        locationEdit->clearEditText();
        return;
    }

    static const QString &begin = KGlobal::staticQString(" \"");
    QString text;
    KFileItemList::const_iterator kit = list->begin();
    const KFileItemList::const_iterator kend = list->end();
    for ( ; kit != kend; ++kit )
    {
        text.append( begin ).append( (*kit)->name() ).append( QLatin1Char( '"' ) );
    }

    setLocationText( text.trimmed() );
}

void KFileWidgetPrivate::setLocationText( const QString& text )
{
    // setCurrentItem() will cause textChanged() being emitted,
    // so slotLocationChanged() will be called. Make sure we don't clear
    // the KDirOperator's view-selection in there
    QObject::disconnect( locationEdit, SIGNAL( textChanged( const QString& ) ),
                         q, SLOT( slotLocationChanged( const QString& ) ) );
    locationEdit->setCurrentIndex( 0 );
    QObject::connect( locationEdit, SIGNAL( textChanged( const QString& ) ),
                      q, SLOT( slotLocationChanged( const QString& )) );
    locationEdit->setEditText( text );

    // don't change selection when user has clicked on an item
    if ( operationMode == KFileWidget::Saving && !locationEdit->isVisible())
       setNonExtSelection();
}

void KFileWidgetPrivate::updateLocationWhatsThis()
{
    QString whatsThisText;
    if (operationMode == KFileWidget::Saving)
    {
        whatsThisText = "<qt>" + i18n("This is the name to save the file as.") +
                             i18n (autocompletionWhatsThisText);
    }
    else if (ops->mode() & KFile::Files)
    {
        whatsThisText = "<qt>" + i18n("This is the list of files to open. More than "
                             "one file can be specified by listing several "
                             "files, separated by spaces.") +
                              i18n (autocompletionWhatsThisText);
    }
    else
    {
        whatsThisText = "<qt>" + i18n("This is the name of the file to open.") +
                             i18n (autocompletionWhatsThisText);
    }

    locationLabel->setWhatsThis(whatsThisText);
    locationEdit->setWhatsThis(whatsThisText);
}

void KFileWidgetPrivate::initSpeedbar()
{
    placesView = new KFilePlacesView( q );
    placesView->setModel(new KFilePlacesModel(placesView));
    placesView->setFrameStyle( QFrame::Box | QFrame::Plain );

    placesView->setObjectName( QLatin1String( "url bar" ) );
    QObject::connect( placesView, SIGNAL( urlChanged( const KUrl& )),
                      q, SLOT( enterUrl( const KUrl& )) );

    // need to set the current url of the urlbar manually (not via urlEntered()
    // here, because the initial url of KDirOperator might be the same as the
    // one that will be set later (and then urlEntered() won't be emitted).
    // ### REMOVE THIS when KDirOperator's initial URL (in the c'tor) is gone.
    placesView->setUrl( url );

    placesViewLayout->insertWidget( 0, placesView );
}

void KFileWidgetPrivate::initGUI()
{
    delete boxLayout; // deletes all sub layouts

    boxLayout = new QVBoxLayout( q);
    boxLayout->setSpacing(KDialog::spacingHint());
    boxLayout->addWidget(toolbar, 0, Qt::AlignTop);
    boxLayout->setMargin(0); // no additional margin to the already existing

    placesViewLayout = new QHBoxLayout();
    boxLayout->addItem(placesViewLayout); // needed for the placesView that may appear
    vbox = new QVBoxLayout();
    vbox->setMargin(KDialog::marginHint());
    placesViewLayout->addItem(vbox);

    vbox->addWidget(ops, 4);
    vbox->addSpacing(KDialog::spacingHint());

    QGridLayout* lafBox = new QGridLayout();

    lafBox->setSpacing(KDialog::spacingHint());
    lafBox->addWidget(locationLabel, 0, 0, Qt::AlignVCenter);
    lafBox->addWidget(locationEdit, 0, 1, Qt::AlignVCenter);
    lafBox->addWidget(okButton, 0, 2, Qt::AlignVCenter);

    lafBox->addWidget(filterLabel, 1, 0, Qt::AlignVCenter);
    lafBox->addWidget(filterWidget, 1, 1, Qt::AlignVCenter);
    lafBox->addWidget(cancelButton, 1, 2, Qt::AlignVCenter);

    lafBox->setColumnStretch(1, 4);

    vbox->addLayout(lafBox);
    vbox->addSpacing(KDialog::spacingHint());

    // add the Automatically Select Extension checkbox
    vbox->addWidget (autoSelectExtCheckBox);
    vbox->addSpacing (3);

    q->setTabOrder(ops, autoSelectExtCheckBox);
    q->setTabOrder(autoSelectExtCheckBox, locationEdit);
    q->setTabOrder(locationEdit, filterWidget);
    q->setTabOrder(filterWidget, okButton);
    q->setTabOrder(okButton, cancelButton);
    q->setTabOrder(cancelButton, pathCombo);
    q->setTabOrder(pathCombo, ops);
    q->setTabOrder(cancelButton, pathCombo);
    q->setTabOrder(pathCombo, ops);
}

void KFileWidget::slotFilterChanged()
{
    QString filter = d->filterWidget->currentFilter();
    d->ops->clearFilter();

    if ( filter.indexOf( '/' ) > -1 ) {
        QStringList types = filter.split(" ",QString::SkipEmptyParts); //QStringList::split( " ", filter );
        types.prepend( "inode/directory" );
        d->ops->setMimeFilter( types );
    }
    else
        d->ops->setNameFilter( filter );

    d->ops->updateDir();

    d->updateAutoSelectExtension();

    emit filterChanged( filter );
}


void KFileWidget::setUrl(const KUrl& url, bool clearforward)
{
    d->selection.clear();
    d->ops->setUrl( url, clearforward);
}

// Protected
void KFileWidget::urlEntered(const KUrl& url)
{
    QString filename = d->locationEdit->currentText();
    d->selection.clear();

    if ( d->pathCombo->count() != 0 ) { // little hack
        d->pathCombo->setUrl( url );
    }

    bool blocked = d->locationEdit->blockSignals( true );
    d->locationEdit->setCurrentIndex( 0 );
    if ( d->keepLocation )
        d->locationEdit->setEditText( filename );

    d->locationEdit->blockSignals( blocked );

    QString dir = url.url(KUrl::AddTrailingSlash);
    static_cast<KUrlCompletion*>( d->pathCombo->completionObject() )->setDir( dir );
    static_cast<KUrlCompletion*>( d->locationEdit->completionObject() )->setDir( dir );

    if ( d->placesView )
        d->placesView->setUrl( url );
}

void KFileWidget::locationActivated( const QString& url )
{
    // This guard prevents any URL _typed_ by the user from being interpreted
    // twice (by returnPressed/slotOk and here, activated/locationActivated)
    // after the user presses Enter.  Without this, _both_ setSelection and
    // slotOk would "u.addPath( url )" ...so instead we leave it up to just
    // slotOk....
    if (!d->locationEdit->lineEdit()->isModified())
        setSelection( url );
}

void KFileWidget::enterUrl( const KUrl& url )
{
    setUrl( url );
}

void KFileWidget::enterUrl( const QString& url )
{
    setUrl( KUrl( KUrlCompletion::replacedPath( url, true, true )) );
}


void KFileWidget::setSelection(const QString& url)
{
    kDebug(kfile_area) << "setSelection " << url << endl;

    if (url.isEmpty()) {
        d->selection.clear();
        return;
    }

    KUrl u = getCompleteUrl(url);
    if (!u.isValid()) { // if it still is
        kWarning() << url << " is not a correct argument for setSelection!" << endl;
        return;
    }

    // Honor protocols that do not support directory listing
    if (!KProtocolManager::supportsListing(u))
        return;

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KFileItem i(KFileItem::Unknown, KFileItem::Unknown, u, true );
    //    KFileItem i(u.path());
    kDebug(kfile_area) << "KFileItem " << u.path() << " " << i.isDir() << " " << u.isLocalFile() << " " << QFile::exists( u.path() ) << endl;
    if ( i.isDir() && u.isLocalFile() && QFile::exists( u.path() ) ) {
        // trust isDir() only if the file is
        // local (we cannot stat non-local urls) and if it exists!
        // (as KFileItem does not check if the file exists or not
        // -> the statbuffer is undefined -> isDir() is unreliable) (Simon)
        setUrl(u, true);
    }
    else {
        QString filename = u.url();
        int sep = filename.lastIndexOf('/');
        if (sep >= 0) { // there is a / in it
            KUrl dir(u);
            dir.setQuery( QString() );
            dir.setFileName( QString() );
            setUrl(dir, true );

            // filename must be decoded, or "name with space" would become
            // "name%20with%20space", so we use KUrl::fileName()
            filename = u.fileName();
            kDebug(kfile_area) << "filename " << filename << endl;
            d->selection = filename;
            d->setLocationText( filename );

            // tell the line edit that it has been edited
            // otherwise we won't know this was set by the user
            // and it will be ignored if there has been an
            // auto completion. this caused bugs where automcompletion
            // would start, the user would pick something from the
            // history and then hit Ok only to get the autocompleted
            // selection. OOD->OPS.
            d->locationEdit->lineEdit()->setModified( true );
        }

        d->url = d->ops->url();
        d->url.addPath(filename);
    }
}

void KFileWidget::slotLoadingFinished()
{
    if ( !d->selection.isNull() )
        d->ops->setCurrentItem( d->selection );
}

void KFileWidget::fileCompletion( const QString& match )
{
    if ( match.isEmpty() && d->ops->view() )
        d->ops->view()->clearSelection();
    else
        d->ops->setCurrentItem( match );
}

void KFileWidget::slotLocationChanged( const QString& text )
{
    if ( text.isEmpty() && d->ops->view() )
        d->ops->view()->clearSelection();

    d->updateFilter();
}

KUrl KFileWidget::selectedUrl() const
{
    if ( d->inAccept )
        return d->url;
    else
        return KUrl();
}

KUrl::List KFileWidget::selectedUrls() const
{
    KUrl::List list;
    if ( d->inAccept ) {
        if ( (d->ops->mode() & KFile::Files) == KFile::Files )
            list = d->parseSelectedUrls();
        else
            list.append( d->url );
    }
    return list;
}


KUrl::List& KFileWidgetPrivate::parseSelectedUrls()
{
    if ( filenames.isEmpty() ) {
        return urlList;
    }

    urlList.clear();
    if ( filenames.contains( '/' )) { // assume _one_ absolute filename
        static const QString &prot = KGlobal::staticQString(":/");
        KUrl u;
        if ( filenames.indexOf( prot ) != -1 )
            u = filenames;
        else
            u.setPath( filenames );

        if ( u.isValid() )
            urlList.append( u );
        else
            KMessageBox::error( q,
                                i18n("The chosen filenames do not\n"
                                     "appear to be valid."),
                                i18n("Invalid Filenames") );
    }

    else
        urlList = tokenize( filenames );

    filenames.clear(); // indicate that we parsed that one

    return urlList;
}


// FIXME: current implementation drawback: a filename can't contain quotes
KUrl::List KFileWidgetPrivate::tokenize( const QString& line ) const
{
    KUrl::List urls;
    KUrl u( ops->url() );
    QString name;

    const int count = line.count( QLatin1Char( '"' ) );
    if ( count == 0 ) { // no " " -> assume one single file
        u.setFileName( line );
        if ( u.isValid() )
            urls.append( u );

        return urls;
    }

    if ( (count % 2) == 1 ) { // odd number of " -> error
        KMessageBox::sorry(q, i18n("The requested filenames\n"
                                   "%1\n"
                                   "do not appear to be valid;\n"
                                   "make sure every filename is enclosed in double quotes.", line),
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


QString KFileWidget::selectedFile() const
{
    if ( d->inAccept ) {
        const KUrl url = KIO::NetAccess::mostLocalUrl(d->url,topLevelWidget());
        if (url.isLocalFile())
            return url.path();
        else {
            KMessageBox::sorry( const_cast<KFileWidget*>(this),
                                i18n("You can only select local files."),
                                i18n("Remote Files Not Accepted") );
        }
    }
    return QString();
}

QStringList KFileWidget::selectedFiles() const
{
    QStringList list;

    if ( d->inAccept ) {
        if ( (d->ops->mode() & KFile::Files) == KFile::Files ) {
            KUrl::List urls = d->parseSelectedUrls();
            QList<KUrl>::const_iterator it = urls.begin();
            while ( it != urls.end() ) {
                KUrl url = KIO::NetAccess::mostLocalUrl(*it,topLevelWidget());
                if ( url.isLocalFile() )
                    list.append( url.path() );
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

KUrl KFileWidget::baseUrl() const
{
    return d->ops->url();
}

void KFileWidget::showEvent(QShowEvent* event)
{
    if ( !d->hasView ) { // delayed view-creation
        d->ops->setView(KFile::Default);
        d->hasView = true;
    }
    d->ops->clearHistory();

    QWidget::showEvent(event);
}

void KFileWidget::setMode( KFile::Modes m )
{
    d->ops->setMode(m);
    if ( d->ops->dirOnlyMode() ) {
        d->filterWidget->setDefaultFilter( i18n("*|All Folders") );
    }
    else {
        d->filterWidget->setDefaultFilter( i18n("*|All Files") );
    }

    d->updateAutoSelectExtension();
}

KFile::Modes KFileWidget::mode() const
{
    return d->ops->mode();
}


void KFileWidgetPrivate::readConfig( const KConfigGroup &configGroup)
{
    ops->readConfig(configGroup);

    KUrlComboBox *combo = pathCombo;
    combo->setUrls( configGroup.readPathListEntry( RecentURLs ), KUrlComboBox::RemoveTop );
    combo->setMaxItems( configGroup.readEntry( RecentURLsNumber,
                                       DefaultRecentURLsNumber ) );
    combo->setUrl( ops->url() );
    autoDirectoryFollowing = configGroup.readEntry( AutoDirectoryFollowing,
                                            DefaultDirectoryFollowing );

    KGlobalSettings::Completion cm = (KGlobalSettings::Completion)
                                      configGroup.readEntry( PathComboCompletionMode,
                                      static_cast<int>( KGlobalSettings::completionMode() ) );
    if ( cm != KGlobalSettings::completionMode() )
        combo->setCompletionMode( cm );

    cm = (KGlobalSettings::Completion)
         configGroup.readEntry( LocationComboCompletionMode,
                        static_cast<int>( KGlobalSettings::completionMode() ) );
    if ( cm != KGlobalSettings::completionMode() )
        locationEdit->setCompletionMode( cm );

    // show or don't show the speedbar
    q->toggleSpeedbar( configGroup.readEntry(ShowSpeedbar, true) );

    // show or don't show the bookmarks
    q->toggleBookmarks( configGroup.readEntry(ShowBookmarks, false) );

    // does the user want Automatically Select Extension?
    autoSelectExtChecked = configGroup.readEntry (AutoSelectExtChecked, DefaultAutoSelectExtChecked);
    updateAutoSelectExtension();

    int w1 = q->minimumSize().width();
    int w2 = toolbar->sizeHint().width() + 10;
    if (w1 < w2)
        q->setMinimumWidth(w2);
    //restoreDialogSize( d->fileWidget->viewConfigGroup() );
}

void KFileWidgetPrivate::writeConfig(KConfigGroup &configGroup)
{
    configGroup.writePathEntry( RecentURLs, pathCombo->urls() );
    //saveDialogSize( configGroup, KConfigBase::Persistent | KConfigBase::Global );
    configGroup.writeEntry( PathComboCompletionMode, static_cast<int>(pathCombo->completionMode()) );
    configGroup.writeEntry( LocationComboCompletionMode, static_cast<int>(locationEdit->completionMode()) );
    configGroup.writeEntry( ShowSpeedbar, placesView && !placesView->isHidden() );
    configGroup.writeEntry( ShowBookmarks, bookmarkHandler != 0 );
    configGroup.writeEntry( AutoSelectExtChecked, autoSelectExtChecked );

    ops->writeConfig(configGroup);
}


void KFileWidgetPrivate::readRecentFiles( KConfig *kc )
{
    KConfigGroup cg( kc, ConfigGroup );

    locationEdit->setMaxItems( cg.readEntry( RecentFilesNumber,
                                             DefaultRecentURLsNumber ) );
    locationEdit->setUrls( cg.readPathListEntry( RecentFiles ),
                           KUrlComboBox::RemoveBottom );
    locationEdit->insertItem(0, QString()); // dummy item without pixmap
    locationEdit->setCurrentIndex( 0 );
}

void KFileWidgetPrivate::saveRecentFiles( KConfig *kc )
{
    KConfigGroup cg(kc, ConfigGroup );
    cg.writePathEntry( RecentFiles, locationEdit->urls() );
}

KPushButton * KFileWidget::okButton() const
{
    return d->okButton;
}

KPushButton * KFileWidget::cancelButton() const
{
    return d->cancelButton;
}

// Called by KFileDialog
void KFileWidget::slotCancel()
{
    d->ops->close();

    KSharedConfig::Ptr config = KGlobal::config();
    config->setForceGlobal( true );
    KConfigGroup grp(config,ConfigGroup);
    d->writeConfig(grp);
    config->setForceGlobal( false );
}

void KFileWidget::setKeepLocation( bool keep )
{
    d->keepLocation = keep;
}

bool KFileWidget::keepsLocation() const
{
    return d->keepLocation;
}

void KFileWidget::setOperationMode( OperationMode mode )
{
    d->operationMode = mode;
    d->keepLocation = (mode == Saving);
    d->filterWidget->setEditable( !d->hasDefaultFilter || mode != Saving );
    if ( mode == Opening )
       d->okButton->setGuiItem( KStandardGuiItem::open() );
    else if ( mode == Saving ) {
       d->okButton->setGuiItem( KStandardGuiItem::save() );
       d->setNonExtSelection();
    }
    else
       d->okButton->setGuiItem( KStandardGuiItem::ok() );
    d->updateLocationWhatsThis();
    d->updateAutoSelectExtension();
}

KFileWidget::OperationMode KFileWidget::operationMode() const
{
    return d->operationMode;
}

void KFileWidget::slotAutoSelectExtClicked()
{
    kDebug (kfile_area) << "slotAutoSelectExtClicked(): "
                         << d->autoSelectExtCheckBox->isChecked() << endl;

    // whether the _user_ wants it on/off
    d->autoSelectExtChecked = d->autoSelectExtCheckBox->isChecked();

    // update the current filename's extension
    d->updateLocationEditExtension (d->extension /* extension hasn't changed */);
}

static QString getExtensionFromPatternList(const QStringList &patternList)
{
    QString ret;
    kDebug (kfile_area) << "\tgetExtension " << patternList << endl;

    QStringList::ConstIterator patternListEnd = patternList.end();
    for (QStringList::ConstIterator it = patternList.begin();
         it != patternListEnd;
         it++)
    {
        kDebug (kfile_area) << "\t\ttry: \'" << (*it) << "\'" << endl;

        // is this pattern like "*.BMP" rather than useless things like:
        //
        // README
        // *.
        // *.*
        // *.JP*G
        // *.JP?
        if ((*it).startsWith ("*.") &&
            (*it).length() > 2 &&
            (*it).indexOf('*', 2) < 0 && (*it).indexOf ('?', 2) < 0)
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


//QString KFileWidget::currentFilterExtension()
//{
//    return d->extension;
//}

void KFileWidgetPrivate::updateAutoSelectExtension()
{
    if (!autoSelectExtCheckBox) return;

    //
    // Figure out an extension for the Automatically Select Extension thing
    // (some Windows users apparently don't know what to do when confronted
    // with a text file called "COPYING" but do know what to do with
    // COPYING.txt ...)
    //

    kDebug (kfile_area) << "Figure out an extension: " << endl;
    QString lastExtension = extension;
    extension.clear();

    // Automatically Select Extension is only valid if the user is _saving_ a _file_
    if ((operationMode == KFileWidget::Saving) && (ops->mode() & KFile::File))
    {
        //
        // Get an extension from the filter
        //

        QString filter = filterWidget->currentFilter();
        if (!filter.isEmpty())
        {
            // e.g. "*.cpp"
            if (filter.indexOf ('/') < 0)
            {
                extension = getExtensionFromPatternList (filter.split(" ",QString::SkipEmptyParts)/*QStringList::split (" ", filter)*/).toLower();
                kDebug (kfile_area) << "\tsetFilter-style: pattern ext=\'"
                                    << extension << "\'" << endl;
            }
            // e.g. "text/html"
            else
            {
                KMimeType::Ptr mime = KMimeType::mimeType (filter);

                // first try X-KDE-NativeExtension
                QString nativeExtension = mime->property ("X-KDE-NativeExtension").toString();
                if (!nativeExtension.isEmpty() && nativeExtension.at (0) == '.')
                {
                    extension = nativeExtension.toLower();
                    kDebug (kfile_area) << "\tsetMimeFilter-style: native ext=\'"
                                         << extension << "\'" << endl;
                }

                // no X-KDE-NativeExtension
                if (extension.isEmpty())
                {
                    extension = getExtensionFromPatternList (mime->patterns()).toLower();
                    kDebug (kfile_area) << "\tsetMimeFilter-style: pattern ext=\'"
                                         << extension << "\'" << endl;
                }
            }
        }


        //
        // GUI: checkbox
        //

        QString whatsThisExtension;
        if (!extension.isEmpty())
        {
            // remember: sync any changes to the string with below
            autoSelectExtCheckBox->setText (i18n ("Automatically select filename e&xtension (%1)",  extension));
            whatsThisExtension = i18n ("the extension <b>%1</b>",  extension);

            autoSelectExtCheckBox->setEnabled (true);
            autoSelectExtCheckBox->setChecked (autoSelectExtChecked);
        }
        else
        {
            // remember: sync any changes to the string with above
            autoSelectExtCheckBox->setText (i18n ("Automatically select filename e&xtension"));
            whatsThisExtension = i18n ("a suitable extension");

            autoSelectExtCheckBox->setChecked (false);
            autoSelectExtCheckBox->setEnabled (false);
        }

        const QString locationLabelText = stripUndisplayable (locationLabel->text());
        const QString filterLabelText = stripUndisplayable (filterLabel->text());
        autoSelectExtCheckBox->setWhatsThis(            "<qt>" +
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
                    ,
                  locationLabelText,
                  locationLabelText,
                  whatsThisExtension)
            + "</qt>"
            );

        autoSelectExtCheckBox->show();


        // update the current filename's extension
        updateLocationEditExtension (lastExtension);
    }
    // Automatically Select Extension not valid
    else
    {
        autoSelectExtCheckBox->setChecked (false);
        autoSelectExtCheckBox->hide();
    }
}

// Updates the extension of the filename specified in d->locationEdit if the
// Automatically Select Extension feature is enabled.
// (this prevents you from accidently saving "file.kwd" as RTF, for example)
void KFileWidgetPrivate::updateLocationEditExtension (const QString &lastExtension)
{
    if (!autoSelectExtCheckBox->isChecked() || extension.isEmpty())
        return;

    QString urlStr = locationEdit->currentText();
    if (urlStr.isEmpty())
        return;

    KUrl url = q->getCompleteUrl(urlStr);
    kDebug (kfile_area) << "updateLocationEditExtension (" << url << ")" << endl;

    const int fileNameOffset = urlStr.lastIndexOf ('/') + 1;
    QString fileName = urlStr.mid (fileNameOffset);

    const int dot = fileName.lastIndexOf ('.');
    const int len = fileName.length();
    if (dot > 0 && // has an extension already and it's not a hidden file
                   // like ".hidden" (but we do accept ".hidden.ext")
        dot != len - 1 // and not deliberately suppressing extension
        )
    {
        // exists?
        KIO::UDSEntry t;
        if (KIO::NetAccess::stat (url, t, q->topLevelWidget()))
        {
            kDebug (kfile_area) << "\tfile exists" << endl;

            if (t.isDir())
            {
                kDebug (kfile_area) << "\tisDir - won't alter extension" << endl;
                return;
            }

            // --- fall through ---
        }


        //
        // try to get rid of the current extension
        //

        // catch "double extensions" like ".tar.gz"
        if (lastExtension.length() && fileName.endsWith (lastExtension))
            fileName.truncate (len - lastExtension.length());
        // can only handle "single extensions"
        else
            fileName.truncate (dot);

        // add extension
        const QString newText = urlStr.left (fileNameOffset) + fileName + extension;
        if ( newText != locationEdit->currentText() )
        {
            locationEdit->setItemText(locationEdit->currentIndex(),urlStr.left (fileNameOffset) + fileName + extension);
            locationEdit->lineEdit()->setModified (true);
        }
    }
}

// Updates the filter if the extension of the filename specified in d->locationEdit is changed
// (this prevents you from accidently saving "file.kwd" as RTF, for example)
void KFileWidgetPrivate::updateFilter()
{
    if ((operationMode == KFileWidget::Saving) && (ops->mode() & KFile::File) ) {
        const QString urlStr = locationEdit->currentText();
        if (urlStr.isEmpty())
            return;

        KMimeType::Ptr mime = KMimeType::findByPath(urlStr, 0, true);
        if (mime && mime->name() != KMimeType::defaultMimeType()) {
            if (filterWidget->currentFilter() != mime->name() &&
                filterWidget->filters().indexOf(mime->name()) != -1)
                filterWidget->setCurrentFilter(mime->name());
        }
    }
}

// applies only to a file that doesn't already exist
void KFileWidgetPrivate::appendExtension (KUrl &url)
{
    if (!autoSelectExtCheckBox->isChecked() || extension.isEmpty())
        return;

    QString fileName = url.fileName();
    if (fileName.isEmpty())
        return;

    kDebug (kfile_area) << "appendExtension(" << url << ")" << endl;

    const int len = fileName.length();
    const int dot = fileName.lastIndexOf ('.');

    const bool suppressExtension = (dot == len - 1);
    const bool unspecifiedExtension = (dot <= 0);

    // don't KIO::NetAccess::Stat if unnecessary
    if (!(suppressExtension || unspecifiedExtension))
        return;

    // exists?
    KIO::UDSEntry t;
    if (KIO::NetAccess::stat (url, t, q->topLevelWidget()))
    {
        kDebug (kfile_area) << "\tfile exists - won't append extension" << endl;
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
        kDebug (kfile_area) << "\tstrip trailing dot" << endl;
        url.setFileName (fileName.left (len - 1));
    }
    // evilmatically append extension :) if the user hasn't specified one
    else if (unspecifiedExtension)
    {
        kDebug (kfile_area) << "\tappending extension \'" << extension << "\'..." << endl;
        url.setFileName (fileName + extension);
        kDebug (kfile_area) << "\tsaving as \'" << url << "\'" << endl;
    }
}


// adds the selected files/urls to 'recent documents'
void KFileWidget::addToRecentDocuments()
{
    int m = d->ops->mode();

    if ( m & KFile::LocalOnly ) {
        QStringList files = selectedFiles();
        QStringList::ConstIterator it = files.begin();
        for ( ; it != files.end(); ++it )
            KRecentDocument::add( *it );
    }

    else { // urls
        KUrl::List urls = selectedUrls();
        KUrl::List::ConstIterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            if ( (*it).isValid() )
                KRecentDocument::add( *it );
        }
    }
}

KUrlComboBox* KFileWidget::locationEdit() const
{
    return d->locationEdit;
}

KFileFilterCombo* KFileWidget::filterWidget() const
{
    return d->filterWidget;
}

KActionCollection * KFileWidget::actionCollection() const
{
    return d->ops->actionCollection();
}

void KFileWidget::toggleSpeedbar( bool show )
{
    if ( show )
    {
        if ( !d->placesView )
            d->initSpeedbar();

        d->placesView->show();

        // check to see if they have a home item defined, if not show the home button
        KUrl homeURL;
        homeURL.setPath( QDir::homePath() );
        KFilePlacesModel *model = static_cast<KFilePlacesModel*>(d->placesView->model());
        int rowCount = model->rowCount();
        for ( int rowIndex = 0 ; rowIndex < d->placesView->model()->rowCount() ; rowIndex++ )
        {
            QModelIndex index = model->index(rowIndex, 0);
            KUrl url = model->url(index);

            if ( homeURL.equals( url, KUrl::CompareWithoutTrailingSlash ) ) {
                d->toolbar->removeAction( d->ops->actionCollection()->action( "home" ) );
                break;
            }
        }
    }
    else
    {
        if (d->placesView)
            d->placesView->hide();

        QAction* homeAction = d->ops->actionCollection()->action( "home" );
        QAction* reloadAction = d->ops->actionCollection()->action( "reload" );
        if ( !d->toolbar->actions().contains(homeAction) )
            d->toolbar->insertAction( reloadAction, homeAction );
    }

    static_cast<KToggleAction *>(actionCollection()->action("toggleSpeedbar"))->setChecked( show );
}

void KFileWidget::toggleBookmarks(bool show)
{
    if (show)
    {
        if (d->bookmarkHandler)
        {
            return;
        }

        d->bookmarkHandler = new KFileBookmarkHandler( this );
        connect( d->bookmarkHandler, SIGNAL( openUrl( const QString& )),
                    SLOT( enterUrl( const QString& )));

        d->bookmarkButton = new KActionMenu(KIcon("bookmark"),i18n("Bookmarks"), this);
        actionCollection()->addAction("bookmark", d->bookmarkButton);
        d->bookmarkButton->setMenu(d->bookmarkHandler->menu());
        d->bookmarkButton->setWhatsThis(i18n("<qt>This button allows you to bookmark specific locations. "
                                "Click on this button to open the bookmark menu where you may add, "
                                "edit or select a bookmark.<p>"
                                "These bookmarks are specific to the file dialog, but otherwise operate "
                                "like bookmarks elsewhere in KDE.</qt>"));
        d->toolbar->addAction(d->bookmarkButton);
    }
    else if (d->bookmarkHandler)
    {
        delete d->bookmarkHandler;
        d->bookmarkHandler = 0;
        delete d->bookmarkButton;
        d->bookmarkButton = 0;
    }

    static_cast<KToggleAction *>(actionCollection()->action("toggleBookmarks"))->setChecked( show );
}

#if 0
// to be re-added once an app needs it so that it can be tested
// (it was added for kedit in kde3, but kedit doesn't exist anymore)
QAction* KFileWidget::pathComboIndex()
{
    return d->m_pathComboIndex;
}
#endif

// static
KUrl KFileWidget::getStartUrl( const KUrl& startDir,
                               QString& recentDirClass )
{
    initStatic();

    recentDirClass.clear();
    KUrl ret;

    bool useDefaultStartDir = startDir.isEmpty();
    if ( !useDefaultStartDir )
    {
        if (startDir.protocol() == "kfiledialog")
        {
            if ( startDir.query() == "?global" )
              recentDirClass = QString( "::%1" ).arg( startDir.path().mid( 1 ) );
            else
              recentDirClass = QString( ":%1" ).arg( startDir.path().mid( 1 ) );

            ret = KUrl( KRecentDirs::dir(recentDirClass) );
        }
        else
        {
            ret = startDir;
            // If we won't be able to list it (e.g. http), then use default
            if ( !KProtocolManager::supportsListing( ret ) )
                useDefaultStartDir = true;
        }
    }

    if ( useDefaultStartDir )
    {
        if (lastDirectory->isEmpty()) {
            lastDirectory->setPath(KGlobalSettings::documentPath());
            KUrl home;
            home.setPath( QDir::homePath() );
            // if there is no docpath set (== home dir), we prefer the current
            // directory over it. We also prefer the homedir when our CWD is
            // different from our homedirectory or when the document dir
            // does not exist
            if ( lastDirectory->path(KUrl::AddTrailingSlash) == home.path(KUrl::AddTrailingSlash) ||
                 QDir::currentPath() != QDir::homePath() ||
                 !QDir(lastDirectory->path(KUrl::AddTrailingSlash)).exists() )
                lastDirectory->setPath(QDir::currentPath());
        }
        ret = *lastDirectory;
    }

    return ret;
}

void KFileWidget::setStartDir( const KUrl& directory )
{
    initStatic();
    if ( directory.isValid() )
        *lastDirectory = directory;
}

void KFileWidgetPrivate::setNonExtSelection()
{
    // Enhanced rename: Don't highlight the file extension.
    QString filename = locationEdit->currentText().trimmed();
    QString extension = KMimeType::extractKnownExtension( filename );

    if ( !extension.isEmpty() )
       locationEdit->lineEdit()->setSelection( 0, filename.length() - extension.length() - 1 );
    else
    {
       int lastDot = filename.lastIndexOf( '.' );
       if ( lastDot > 0 )
          locationEdit->lineEdit()->setSelection( 0, lastDot );
    }
}

KToolBar * KFileWidget::toolBar() const
{
    return d->toolbar;
}

void KFileWidget::setCustomWidget(QWidget* widget)
{
    delete d->customWidget;
    d->customWidget = widget;

    // add it to the dialog, below the filter list box.

    // Change the parent so that this widget is a child of the main widget
    d->customWidget->setParent( this );

    d->vbox->addWidget( d->customWidget );
    //d->vbox->addSpacing(3); // can't do this every time...

    // FIXME: This should adjust the tab orders so that the custom widget
    // comes after the Cancel button. The code appears to do this, but the result
    // somehow screws up the tab order of the file path combo box. Not a major
    // problem, but ideally the tab order with a custom widget should be
    // the same as the order without one.
    setTabOrder(d->cancelButton, d->customWidget);
    setTabOrder(d->customWidget, d->pathCombo);
}

#include "kfilewidget.moc"
