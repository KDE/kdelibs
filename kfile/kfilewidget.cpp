// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>
                  1998 Daniel Grana <grana@ie.iwi.unibe.ch>
                  1999,2000,2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>
                  2003 Clarence Dang <dang@kde.org>
                  2007 David Faure <faure@kde.org>
                  2008 Rafael Fernández López <ereslibre@kde.org>

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
#include "kfilebookmarkhandler_p.h"
#include "kurlcombobox.h"
#include "kurlnavigator.h"
#include <config-kfile.h>

#include <kactioncollection.h>
#include <kdiroperator.h>
#include <kdirselectdialog.h>
#include <kfilefiltercombo.h>
#include <kimagefilepreview.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kpushbutton.h>
#include <krecentdocument.h>
#include <ktoolbar.h>
#include <kurlcompletion.h>
#include <kuser.h>
#include <kprotocolmanager.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <krecentdirs.h>
#include <kdebug.h>

#include <QtGui/QCheckBox>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSplitter>
#include <QtCore/QFSFileEngine>
#include <kshell.h>
#include <kmessagebox.h>
#include <kauthorized.h>

class KFileWidgetPrivate
{
public:
    KFileWidgetPrivate( KFileWidget* q )
        : boxLayout(0),
          labeledCustomWidget(0),
          bottomCustomWidget(0),
          speedBarWidth(-1),
          inAccept(false),
          dummyAdded(false),
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
    void setLocationText(const KUrl&);
    void setLocationText(const KUrl::List&);
    void appendExtension(KUrl &url);
    void updateLocationEditExtension(const QString &);
    void updateFilter();
    void updateSplitterSize();
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

    /**
     * Returns the absolute version of the URL specified in locationEdit.
     */
    KUrl getCompleteUrl(const QString&) const;

    /**
     * Sets the dummy entry on the history combo box. If the dummy entry
     * already exists, it is overwritten with this information.
     */
    void setDummyHistoryEntry(const QString& text, const QPixmap& icon = QPixmap(),
                              bool usePreviousPixmapIfNull = true);

    /**
     * Removes the dummy entry of the history combo box.
     */
    void removeDummyHistoryEntry();

    // private slots
    void _k_slotLocationChanged( const QString& );
    void _k_urlEntered( const KUrl& );
    void _k_enterUrl( const KUrl& );
    void _k_enterUrl( const QString& );
    void _k_locationAccepted( const QString& );
    void _k_locationActivated( const QString& );
    void _k_slotFilterChanged();
    void _k_fileHighlighted( const KFileItem& );
    void _k_fileSelected( const KFileItem& );
    void _k_slotStatResult( KJob* );
    void _k_slotLoadingFinished();
    void _k_fileCompletion( const QString& );
    void _k_toggleSpeedbar( bool );
    void _k_toggleBookmarks( bool );
    void _k_slotAutoSelectExtClicked();
    void _k_placesViewSplitterMoved();

    void addToRecentDocuments();

    QString locationEditCurrentText() const;

    // the last selected url
    KUrl url;

    // the selected filenames in multiselection mode -- FIXME
    QString filenames;

    // the name of the filename set by setSelection
    QString selection;

    // now following all kind of widgets, that I need to rebuild
    // the geometry management
    QBoxLayout *boxLayout;
    QGridLayout *lafBox;
    QVBoxLayout *vbox;

    QLabel *locationLabel;

    // @deprecated remove in KDE4 -- err, remove what?
    QLabel *filterLabel;
    KUrlNavigator *urlNavigator;
    KPushButton *okButton, *cancelButton;
    KFilePlacesView *placesView;
    QSplitter *placesViewSplitter;
    QWidget *labeledCustomWidget;
    QWidget *bottomCustomWidget;

    // Automatically Select Extension stuff
    QCheckBox *autoSelectExtCheckBox;
    bool autoSelectExtChecked; // whether or not the _user_ has checked the above box
    QString extension; // current extension for this filter

    QList<KIO::StatJob*> statJobs;

    KUrl::List urlList; //the list of selected urls

    // caches the speed bar width. This value will be updated when the splitter
    // is moved. This allows us to properly set a value when the dialog itself
    // is resized
    int speedBarWidth;

    // indicates if the location edit should be kept or cleared when changing
    // directories
    bool keepLocation;

    // the KDirOperators view is set in KFileWidget::show(), so to avoid
    // setting it again and again, we have this nice little boolean :)
    bool hasView;

    bool hasDefaultFilter; // necessary for the operationMode
    bool autoDirectoryFollowing;
    bool inAccept; // true between beginning and end of accept()
    bool dummyAdded; // if the dummy item has been added. This prevents the combo from having a
                     // blank item added when loaded

    KFileWidget::OperationMode operationMode;

    // The file class used for KRecentDirs
    QString fileClass;

    KFileBookmarkHandler *bookmarkHandler;

    KActionMenu* bookmarkButton;
    KConfigGroup *viewConfigGroup;

    KToolBar *toolbar;
    KUrlComboBox *locationEdit;
    KDirOperator *ops;
    KFileFilterCombo *filterWidget;
    KFileWidget* q;

    KFilePlacesModel *model;
};

K_GLOBAL_STATIC(KUrl, lastDirectory) // to set the start path

static const char autocompletionWhatsThisText[] = I18N_NOOP("<qt>While typing in the text area, you may be presented "
                                                  "with possible matches. "
                                                  "This feature can be controlled by clicking with the right mouse button "
                                                  "and selecting a preferred mode from the <b>Text Completion</b> menu.")  "</qt>";

// returns true if the string contains "<a>:/" sequence, where <a> is at least 2 alpha chars
static bool containsProtocolSection( const QString& string )
{
    int len = string.length();
    static const char prot[] = ":/";
    for (int i=0; i < len;) {
        i = string.indexOf( QLatin1String(prot), i );
        if (i == -1)
            return false;
        int j=i-1;
        for (; j >= 0; j--) {
            const QChar& ch( string[j] );
            if (ch.toAscii() == 0 || !ch.isLetter())
                break;
            if (ch.isSpace() && (i-j-1) >= 2)
                return true;
        }
        if (j < 0 && i >= 2)
            return true; // at least two letters before ":/"
        i += 3; // skip : and / and one char
    }
    return false;
}

KFileWidget::KFileWidget( const KUrl& startDir, QWidget *parent )
    : QWidget(parent), KAbstractFileWidget(), d(new KFileWidgetPrivate(this))
{
    // TODO move most of this code for the KFileWidgetPrivate constructor
    d->keepLocation = false;
    d->operationMode = Opening;
    d->bookmarkHandler = 0;
    d->hasDefaultFilter = false;
    d->hasView = false;
    d->placesViewSplitter = 0;

    d->okButton = new KPushButton(KStandardGuiItem::ok(), this);
    d->okButton->setDefault( true );
    d->cancelButton = new KPushButton(KStandardGuiItem::cancel(), this);
    // The dialog shows them
    d->okButton->hide();
    d->cancelButton->hide();

    d->autoSelectExtCheckBox = 0; // delayed loading
    d->autoSelectExtChecked = false;
    d->placesView = 0; // delayed loading

    d->toolbar = new KToolBar(this, true);
    d->toolbar->setObjectName("KFileWidget::toolbar");
    d->toolbar->setMovable(false);

    d->model = new KFilePlacesModel(this);
    d->urlNavigator = new KUrlNavigator(d->model, startDir, d->toolbar);
    d->urlNavigator->setPlacesSelectorVisible(false);

    KUrl u;
    KUrlComboBox *pathCombo = d->urlNavigator->editor();
#ifdef Q_WS_WIN
    foreach( const QFileInfo &drive,QFSFileEngine::drives() )
    {
        u.setPath( drive.filePath() );
        pathCombo->addDefaultUrl(u,
                                 KIO::pixmapForUrl( u, 0, KIconLoader::Small ),
                                 i18n("Drive: %1",  u.toLocalFile()));
    }
#else
    u.setPath(QDir::rootPath());
    pathCombo->addDefaultUrl(u,
                             KIO::pixmapForUrl(u, 0, KIconLoader::Small),
                             u.toLocalFile());
#endif

    u.setPath(QDir::homePath());
    pathCombo->addDefaultUrl(u, KIO::pixmapForUrl(u, 0, KIconLoader::Small),
                             u.path(KUrl::AddTrailingSlash));

    KUrl docPath;
    docPath.setPath( KGlobalSettings::documentPath() );
    if ( (u.path(KUrl::AddTrailingSlash) != docPath.path(KUrl::AddTrailingSlash)) &&
          QDir(docPath.path(KUrl::AddTrailingSlash)).exists() )
    {
        pathCombo->addDefaultUrl( docPath,
                                  KIO::pixmapForUrl( docPath, 0, KIconLoader::Small ),
                                  docPath.path(KUrl::AddTrailingSlash));
    }

    u.setPath( KGlobalSettings::desktopPath() );
    pathCombo->addDefaultUrl(u,
                             KIO::pixmapForUrl(u, 0, KIconLoader::Small),
                             u.path(KUrl::AddTrailingSlash));

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
            SLOT(_k_urlEntered(const KUrl&)));
    connect(d->ops, SIGNAL(fileHighlighted(const KFileItem &)),
            SLOT(_k_fileHighlighted(const KFileItem &)));
    connect(d->ops, SIGNAL(fileSelected(const KFileItem &)),
            SLOT(_k_fileSelected(const KFileItem &)));
    connect(d->ops, SIGNAL(finishedLoading()),
            SLOT(_k_slotLoadingFinished()));

    d->ops->setupMenu(KDirOperator::SortActions |
                   KDirOperator::FileActions |
                   KDirOperator::ViewActions);
    KActionCollection *coll = d->ops->actionCollection();

    // add nav items to the toolbar
    //
    // NOTE:  The order of the button icons here differs from that
    // found in the file manager and web browser, but has been discussed
    // and agreed upon on the kde-core-devel mailing list:
    //
    // http://lists.kde.org/?l=kde-core-devel&m=116888382514090&w=2
    //
    d->toolbar->addAction( coll->action( "up" ) );
    coll->action( "up" )->setWhatsThis(i18n("<qt>Click this button to enter the parent folder.<br /><br />"
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
        new KToggleAction(i18n("Show Places Navigation Panel"), this);
    coll->addAction("toggleSpeedbar", showSidebarAction);
    showSidebarAction->setShortcut( QKeySequence(Qt::Key_F9) );
    connect( showSidebarAction, SIGNAL( toggled( bool ) ),
             SLOT( _k_toggleSpeedbar( bool )) );

    KToggleAction *showBookmarksAction =
        new KToggleAction(i18n("Show Bookmarks"), this);
    coll->addAction("toggleBookmarks", showBookmarksAction);
    connect( showBookmarksAction, SIGNAL( toggled( bool ) ),
             SLOT( _k_toggleBookmarks( bool )) );

    KActionMenu *menu = new KActionMenu( KIcon("configure"), i18n("Options"), this);
    coll->addAction("extra menu", menu);
    menu->setWhatsThis(i18n("<qt>This is the preferences menu for the file dialog. "
                            "Various options can be accessed from this menu including: <ul>"
                            "<li>how files are sorted in the list</li>"
                            "<li>types of view, including icon and list</li>"
                            "<li>showing of hidden files</li>"
                            "<li>the Places navigation panel</li>"
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

    menu->setDelayed( false );
    connect( menu->menu(), SIGNAL( aboutToShow() ),
             d->ops, SLOT( updateSelectionDependentActions() ));
    d->toolbar->addAction( menu );

    d->toolbar->addWidget(d->urlNavigator);

    // FIXME KAction port - add capability
    //d->toolbar->setItemAutoSized (PATH_COMBO);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d->toolbar->setMovable(false);

    KUrlCompletion *pathCompletionObj = new KUrlCompletion( KUrlCompletion::DirCompletion );
    pathCombo->setCompletionObject( pathCompletionObj );
    pathCombo->setAutoDeleteCompletionObject( true );

    connect( d->urlNavigator, SIGNAL( urlChanged( const KUrl&  )),
             this,  SLOT( _k_enterUrl( const KUrl& ) ));

    QString whatsThisText;

    // the Location label/edit
    d->locationLabel = new QLabel(i18n("&Location:"), this);
    d->locationEdit = new KUrlComboBox(KUrlComboBox::Files, true, this);
    // Properly let the dialog be resized (to smaller). Otherwise we could have
    // huge dialogs that can't be resized to smaller (it would be as big as the longest
    // item in this combo box). (ereslibre)
    d->locationEdit->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    connect( d->locationEdit, SIGNAL( editTextChanged( const QString& ) ),
             SLOT( _k_slotLocationChanged( const QString& )) );

    d->updateLocationWhatsThis();
    d->locationLabel->setBuddy(d->locationEdit);

    KUrlCompletion *fileCompletionObj = new KUrlCompletion( KUrlCompletion::FileCompletion );
    QString dir = d->url.url(KUrl::AddTrailingSlash);

    d->urlNavigator->setUrl( dir );

    fileCompletionObj->setDir( dir );
    d->locationEdit->setCompletionObject( fileCompletionObj );
    d->locationEdit->setAutoDeleteCompletionObject( true );
    connect( fileCompletionObj, SIGNAL( match( const QString& ) ),
             SLOT( _k_fileCompletion( const QString& )) );

    connect(d->locationEdit, SIGNAL( returnPressed( const QString&  )),
            this,  SLOT( _k_locationAccepted( const QString& ) ));
    connect(d->locationEdit, SIGNAL( activated( const QString& )),
            this,  SLOT( _k_locationActivated( const QString& ) ));

    // the Filter label/edit
    whatsThisText = i18n("<qt>This is the filter to apply to the file list. "
                         "File names that do not match the filter will not be shown.<p>"
                         "You may select from one of the preset filters in the "
                         "drop down menu, or you may enter a custom filter "
                         "directly into the text area.</p><p>"
                         "Wildcards such as * and ? are allowed.</p></qt>");
    d->filterLabel = new QLabel(i18n("&Filter:"), this);
    d->filterLabel->setWhatsThis(whatsThisText);
    d->filterWidget = new KFileFilterCombo(this);
    // Properly let the dialog be resized (to smaller). Otherwise we could have
    // huge dialogs that can't be resized to smaller (it would be as big as the longest
    // item in this combo box). (ereslibre)
    d->filterWidget->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    d->filterWidget->setWhatsThis(whatsThisText);
    d->filterLabel->setBuddy(d->filterWidget);
    connect(d->filterWidget, SIGNAL(filterChanged()), SLOT(_k_slotFilterChanged()));

    // the Automatically Select Extension checkbox
    // (the text, visibility etc. is set in updateAutoSelectExtension(), which is called by readConfig())
    d->autoSelectExtCheckBox = new QCheckBox (this);
    d->autoSelectExtCheckBox->setStyleSheet(QString("QCheckBox { padding-top: %1px; }").arg(KDialog::spacingHint()));
    connect(d->autoSelectExtCheckBox, SIGNAL(clicked()), SLOT(_k_slotAutoSelectExtClicked()));

    d->initGUI(); // activate GM

    KSharedConfig::Ptr config = KGlobal::config();
    d->readRecentFiles(config.data());

    d->viewConfigGroup=new KConfigGroup(config,ConfigGroup);
    d->ops->setViewConfig(*d->viewConfigGroup);
    d->readConfig(* d->viewConfigGroup);
    setSelection(d->selection);
    d->locationEdit->setFocus();
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
    d->filterWidget->setFilter( QString() );
    d->ops->clearFilter();
    d->hasDefaultFilter = false;
    d->filterWidget->setEditable( true );

    d->updateAutoSelectExtension ();
}

QString KFileWidget::currentMimeFilter() const
{
    int i = d->filterWidget->currentIndex();
    if (d->filterWidget->showsAllTypes() && i == 0)
        return QString(); // The "all types" item has no mimetype

    return d->filterWidget->filters()[i];
}

KMimeType::Ptr KFileWidget::currentFilterMimeType()
{
    return KMimeType::mimeType( currentMimeFilter() );
}

void KFileWidget::setPreviewWidget(KPreviewWidgetBase *w) {
    d->ops->setPreviewWidget(w);
    d->ops->clearHistory();
    d->hasView = true;
}

KUrl KFileWidgetPrivate::getCompleteUrl(const QString &_url) const
{
    QString url = KShell::tildeExpand(_url);
    KUrl u;

    if ( KUrl::isRelativeUrl(url) ) // only a full URL isn't relative. Even /path is.
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

// Called by KFileDialog
void KFileWidget::slotOk()
{
    kDebug(kfile_area) << "slotOk\n";

    // a list of all selected files/directories (if any)
    // can only be used if the user didn't type any filenames/urls himself
    const KFileItemList items = d->ops->selectedItems();

    const QString locationEditCurrentText( d->locationEditCurrentText() );

    if ( (mode() & KFile::Directory) != KFile::Directory ) {
        if ( locationEditCurrentText.isEmpty() ) {
            // allow directory navigation by entering a path and pressing
            // enter, by simply returning we will browse the new path
            if (items.isEmpty())
                return;

            // weird case: the location edit is empty, but there are
            // highlighted files
            bool multi = (mode() & KFile::Files) != 0;
            QString endQuote = QLatin1String("\" ");
            QString name;
            KUrl::List urlList;
            foreach (const KFileItem &fileItem, items) {
                name = fileItem.name();
                if ( multi ) {
                    name.prepend( QLatin1Char( '"' ) );
                    name.append( endQuote );
                }

                urlList << fileItem.url();
            }
            d->setLocationText( urlList );
            return;
        }
    }

    bool dirOnly = d->ops->dirOnlyMode();

    // we can use our kfileitems, no need to parse anything
    if ( !d->locationEdit->lineEdit()->isModified() &&
         !(items.isEmpty() && !dirOnly) ) {

        d->urlList.clear();
        d->filenames.clear();

        if ( dirOnly ) {
            d->url = d->ops->url();
        }
        else {
            if ( !(mode() & KFile::Files) ) {// single selection
                d->url = items.first().url();
            }

            else { // multi (dirs and/or files)
                d->url = d->ops->url();
                KUrl::List urlList;
                foreach (const KFileItem &item, items) {
                    urlList.append(item.url());
                }
                d->urlList = urlList;
            }
        }

        KUrl url = KIO::NetAccess::mostLocalUrl(d->url,topLevelWidget());
        if ( ( (mode() & KFile::LocalOnly) == KFile::LocalOnly ) &&
             !url.isLocalFile() )
        {
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
        if ( locationEditCurrentText.contains( '/' ) ) {
            // relative path? -> prepend the current directory
            KUrl u( d->ops->url(), KShell::tildeExpand( locationEditCurrentText ));
            if ( u.isValid() )
                selectedUrl = u;
            else
                selectedUrl = d->ops->url();
        }
        else // simple filename -> just use the current URL
            selectedUrl = d->ops->url();
    }

    else {
        selectedUrl = d->getCompleteUrl( locationEditCurrentText );

        // appendExtension() may change selectedUrl
        d->appendExtension (selectedUrl);
    }

    if ( !selectedUrl.isValid() ) {
       KMessageBox::sorry( this, i18n("%1\ndoes not appear to be a valid URL.\n", d->url.url()), i18n("Invalid URL") );
       return;
    }

    KUrl url = KIO::NetAccess::mostLocalUrl(selectedUrl,topLevelWidget());
    if ( ( (mode() & KFile::LocalOnly) == KFile::LocalOnly ) &&
         !url.isLocalFile() )
    {
        KMessageBox::sorry( this,
                            i18n("You can only select local files."),
                            i18n("Remote Files Not Accepted") );
        return;
    }

    d->url = url;

    // d->url is a correct URL now

    if ( (mode() & KFile::Directory) == KFile::Directory ) {
        kDebug(kfile_area) << "Directory";
        bool done = true;
        if ( d->url.isLocalFile() ) {
            if ( locationEditCurrentText.isEmpty() ) {
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
                        KUrl fullURL(d->url, locationEditCurrentText);
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

                d->filenames = locationEditCurrentText;
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
    else { // we don't want dir
        KUrl::List urls = d->tokenize( locationEditCurrentText );
        if ( urls.count()==1 && urls.first().isLocalFile() ) {
            QFileInfo info( urls.first().toLocalFile() );
            if ( info.isDir() && this->selectedUrl().isValid() && !this->selectedUrl().equals( urls.first(), KUrl::CompareWithoutTrailingSlash ) ) {
                setSelection( info.absolutePath() );
                slotOk();
                return;
            }
        }
    }

    if (!KAuthorized::authorizeUrlAction("open", KUrl(), d->url))
    {
        QString msg = KIO::buildErrorString(KIO::ERR_ACCESS_DENIED, d->url.prettyUrl());
        KMessageBox::error( this, msg);
        return;
    }

    KIO::StatJob *job = 0L;
    d->statJobs.clear();
    d->filenames = KShell::tildeExpand( locationEditCurrentText );

    if ( (mode() & KFile::Files) == KFile::Files &&
         !locationEditCurrentText.contains( '/' ) ) {
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
            KIO::JobFlags flags = !(*it).isLocalFile() ? KIO::DefaultFlags : KIO::HideProgressInfo;
            job = KIO::stat( *it, flags );
            job->ui()->setWindow (topLevelWidget());
            KIO::Scheduler::scheduleJob( job );
            d->statJobs.append( job );
            connect( job, SIGNAL( result(KJob *) ),
                     SLOT( _k_slotStatResult( KJob *) ));
        }
        return;
    }

    KIO::JobFlags flags = !d->url.isLocalFile() ? KIO::DefaultFlags : KIO::HideProgressInfo;
    job = KIO::stat(d->url,flags);
    job->ui()->setWindow (topLevelWidget());
    d->statJobs.append( job );
    connect(job, SIGNAL(result(KJob*)), SLOT(_k_slotStatResult(KJob*)));
}

// FIXME : count all errors and show messagebox when d->statJobs.count() == 0
// in case of an error, we cancel the whole operation (clear d->statJobs and
// don't call accept)
void KFileWidgetPrivate::_k_slotStatResult(KJob* job)
{
    kDebug(kfile_area) << "slotStatResult";
    KIO::StatJob *sJob = static_cast<KIO::StatJob *>( job );

    if ( !statJobs.removeAll( sJob ) ) {
        return;
    }

    int count = statJobs.count();

    // errors mean in general, the location is no directory ;/
    // Can we be sure that it is exististant at all? (pfeiffer)
    if (sJob->error() && count == 0 && !ops->dirOnlyMode())
    {
        emit q->accepted();
        return;
    }

    KIO::UDSEntry t = sJob->statResult();
    if (t.isDir())
    {
        if ( ops->dirOnlyMode() )
        {
            filenames.clear();
            urlList.clear();
            emit q->accepted();
        }
        else // in File[s] mode, directory means error -> cd into it
        {
            if ( count == 0 ) {
                locationEdit->clearEditText();
                locationEdit->lineEdit()->setModified( false );
                q->setUrl( sJob->url() );
            }
        }
        statJobs.clear();
        return;
    }
    else if ( ops->dirOnlyMode() )
    {
        return; // ### error message?
    }

    kDebug(kfile_area) << "filename " << sJob->url().url();

    if ( count == 0 )
        emit q->accepted();
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
    int atmost = d->locationEdit->maxItems(); //don't add more items than necessary
    for ( ; it != list.end() && atmost > 0; ++it ) {
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
        //FIXME I don't think this works correctly when the KUrlComboBox has some default urls.
        //KUrlComboBox should provide a function to add an url and rotate the existing ones, keeping
        //track of maxItems, and we shouldn't be able to insert items as we please.
        d->locationEdit->insertItem( 1,file);
        atmost--;
    }

    KSharedConfig::Ptr config = KGlobal::config();
    config->setForceGlobal( true );
    KConfigGroup grp(config,ConfigGroup);
    d->writeConfig(grp);
    config->setForceGlobal( false );

    d->saveRecentFiles(config.data());
    config->sync();

    d->addToRecentDocuments();

    if ( (mode() & KFile::Files) != KFile::Files ) // single selection
        emit fileSelected(d->url.url());

    d->ops->close();
}


void KFileWidgetPrivate::_k_fileHighlighted(const KFileItem &i)
{
    const bool modified = locationEdit->lineEdit()->isModified();
    locationEdit->lineEdit()->setModified( false );

    if ( ( !i.isNull() && i.isDir() ) ||
         ( locationEdit->hasFocus() && !locationEdit->currentText().isEmpty() ) ) // don't disturb
        return;

    if ( (ops->mode() & KFile::Files) != KFile::Files ) {
        if ( i.isNull() ) {
            if ( !modified ) {
                setLocationText( KUrl() );
            }
            return;
        }

        url = i.url();

        if ( !locationEdit->hasFocus() ) { // don't disturb while editing
            setLocationText( url );
        }
        emit q->fileHighlighted(url.url());
    }

    else {
        multiSelectionChanged();
        emit q->selectionChanged();
    }

    locationEdit->lineEdit()->selectAll();
}

void KFileWidgetPrivate::_k_fileSelected(const KFileItem &i)
{
    if (!i.isNull() && i.isDir())
        return;

    if ( (ops->mode() & KFile::Files) != KFile::Files ) {
        if ( i.isNull() ) {
            setLocationText( KUrl() );
            return;
        }

        setLocationText( i.url() );
    }
    else {
        multiSelectionChanged();
        emit q->selectionChanged();
    }
    q->slotOk();
}


// I know it's slow to always iterate thru the whole filelist
// (d->ops->selectedItems()), but what can we do?
void KFileWidgetPrivate::multiSelectionChanged()
{
    if ( locationEdit->hasFocus() && !locationEdit->currentText().isEmpty() ) // don't disturb
        return;

    const KFileItemList list = ops->selectedItems();

    if ( list.isEmpty() ) {
        setLocationText( KUrl() );
        return;
    }

    static const QString &begin = KGlobal::staticQString(" \"");
    KUrl::List urlList;
    foreach (const KFileItem &fileItem, list) {
        urlList << fileItem.url();
    }

    setLocationText( urlList );
}

void KFileWidgetPrivate::setDummyHistoryEntry( const QString& text, const QPixmap& icon,
                                               bool usePreviousPixmapIfNull )
{
    // setCurrentItem() will cause textChanged() being emitted,
    // so slotLocationChanged() will be called. Make sure we don't clear
    // the KDirOperator's view-selection in there
    QObject::disconnect( locationEdit, SIGNAL( editTextChanged( const QString& ) ),
                        q, SLOT( _k_slotLocationChanged( const QString& ) ) );

    bool dummyExists = dummyAdded;

    int cursorPosition = locationEdit->lineEdit()->cursorPosition();

    if ( dummyAdded ) {
        if ( !icon.isNull() ) {
            locationEdit->setItemIcon( 0, icon );
            locationEdit->setItemText( 0, text );
        } else {
            if ( !usePreviousPixmapIfNull ) {
                locationEdit->setItemIcon( 0, QPixmap() );
            }
            locationEdit->setItemText( 0, text );
        }
    } else {
        if ( !text.isEmpty() ) {
            if ( !icon.isNull() ) {
                locationEdit->insertItem( 0, icon, text );
            } else {
                if ( !usePreviousPixmapIfNull ) {
                    locationEdit->insertItem( 0, QPixmap(), text );
                } else {
                    locationEdit->insertItem( 0, text );
                }
            }
            dummyAdded = true;
            dummyExists = true;
        }
    }

    if ( dummyExists && !text.isEmpty() ) {
        locationEdit->setCurrentIndex( 0 );
    }

    locationEdit->lineEdit()->setCursorPosition( cursorPosition );

    QObject::connect( locationEdit, SIGNAL( editTextChanged ( const QString& ) ),
                    q, SLOT( _k_slotLocationChanged( const QString& )) );
}

void KFileWidgetPrivate::removeDummyHistoryEntry()
{
    if ( !dummyAdded || locationEdit->lineEdit()->isModified() ) {
        return;
    }

    // setCurrentItem() will cause textChanged() being emitted,
    // so slotLocationChanged() will be called. Make sure we don't clear
    // the KDirOperator's view-selection in there
    QObject::disconnect( locationEdit, SIGNAL( editTextChanged( const QString& ) ),
                        q, SLOT( _k_slotLocationChanged( const QString& ) ) );

    locationEdit->removeItem( 0 );
    locationEdit->setCurrentIndex( -1 );
    dummyAdded = false;

    QObject::connect( locationEdit, SIGNAL( editTextChanged ( const QString& ) ),
                    q, SLOT( _k_slotLocationChanged( const QString& )) );
}

void KFileWidgetPrivate::setLocationText( const KUrl& url )
{
    if ( !url.isEmpty() ) {
        QPixmap mimeTypeIcon = KIconLoader::global()->loadMimeTypeIcon( KMimeType::iconNameForUrl( url ), KIconLoader::Small );
        setDummyHistoryEntry( url.fileName(), mimeTypeIcon );
    } else {
        removeDummyHistoryEntry();
    }

    // don't change selection when user has clicked on an item
    if ( operationMode == KFileWidget::Saving && !locationEdit->isVisible())
       setNonExtSelection();
}

void KFileWidgetPrivate::setLocationText( const KUrl::List& urlList )
{
    if ( urlList.count() > 1 ) {
        QString urls;
        foreach (const KUrl &url, urlList) {
            urls += QString( "\"%1\"" ).arg( url.fileName() ) + ' ';
        }
        urls = urls.left( urls.size() - 1 );

        setDummyHistoryEntry( urls, QPixmap(), false );
    } else if ( urlList.count() ) {
        const QPixmap mimeTypeIcon = KIconLoader::global()->loadMimeTypeIcon( KMimeType::iconNameForUrl( urlList[0] ),  KIconLoader::Small );
        setDummyHistoryEntry( urlList[0].fileName(), mimeTypeIcon );
    } else {
        removeDummyHistoryEntry();
    }

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
    placesView->setModel(model);
    placesView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    placesView->setObjectName( QLatin1String( "url bar" ) );
    QObject::connect( placesView, SIGNAL( urlChanged( const KUrl& )),
                      q, SLOT( _k_enterUrl( const KUrl& )) );

    // need to set the current url of the urlbar manually (not via urlEntered()
    // here, because the initial url of KDirOperator might be the same as the
    // one that will be set later (and then urlEntered() won't be emitted).
    // ### REMOVE THIS when KDirOperator's initial URL (in the c'tor) is gone.
    placesView->setUrl( url );

    placesViewSplitter->insertWidget( 0, placesView );
}

void KFileWidgetPrivate::initGUI()
{
    delete boxLayout; // deletes all sub layouts

    boxLayout = new QVBoxLayout( q);
    boxLayout->setMargin(0); // no additional margin to the already existing
    boxLayout->setSpacing(0);
    boxLayout->addWidget(toolbar, 0, Qt::AlignTop);

    placesViewSplitter = new QSplitter(q);
    placesViewSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    placesViewSplitter->setChildrenCollapsible(false);
    boxLayout->addWidget(placesViewSplitter);

    QObject::connect(placesViewSplitter, SIGNAL(splitterMoved(int,int)), q, SLOT(_k_placesViewSplitterMoved()));

    vbox = new QVBoxLayout();
    vbox->setMargin(0);
    QWidget *vboxWidget = new QWidget();
    vboxWidget->setLayout(vbox);
    placesViewSplitter->insertWidget(0, vboxWidget);

    vbox->addWidget(ops, 4);
    vbox->addSpacing(KDialog::spacingHint());

    lafBox = new QGridLayout();

    lafBox->setSpacing(KDialog::spacingHint());
    lafBox->addWidget(locationLabel, 0, 0, Qt::AlignVCenter);
    lafBox->addWidget(locationEdit, 0, 1, Qt::AlignVCenter);
    lafBox->addWidget(okButton, 0, 2, Qt::AlignVCenter);

    lafBox->addWidget(filterLabel, 1, 0, Qt::AlignVCenter);
    lafBox->addWidget(filterWidget, 1, 1, Qt::AlignVCenter);
    lafBox->addWidget(cancelButton, 1, 2, Qt::AlignVCenter);

    lafBox->setColumnStretch(1, 4);

    vbox->addLayout(lafBox);

    // add the Automatically Select Extension checkbox
    vbox->addWidget(autoSelectExtCheckBox);

    q->setTabOrder(ops, autoSelectExtCheckBox);
    q->setTabOrder(autoSelectExtCheckBox, locationEdit);
    q->setTabOrder(locationEdit, filterWidget);
    q->setTabOrder(filterWidget, okButton);
    q->setTabOrder(okButton, cancelButton);
    q->setTabOrder(cancelButton, urlNavigator);
    q->setTabOrder(urlNavigator, ops);
    q->setTabOrder(cancelButton, urlNavigator);
    q->setTabOrder(urlNavigator, ops);
}

void KFileWidgetPrivate::_k_slotFilterChanged()
{
    QString filter = filterWidget->currentFilter();
    ops->clearFilter();

    if ( filter.indexOf( '/' ) > -1 ) {
        QStringList types = filter.split(" ",QString::SkipEmptyParts); //QStringList::split( " ", filter );
        types.prepend( "inode/directory" );
        ops->setMimeFilter( types );
    }
    else
        ops->setNameFilter( filter );

    ops->updateDir();

    updateAutoSelectExtension();

    emit q->filterChanged( filter );
}


void KFileWidget::setUrl(const KUrl& url, bool clearforward)
{
    d->selection.clear();
    d->ops->setUrl( url, clearforward);
}

// Protected
void KFileWidgetPrivate::_k_urlEntered(const KUrl& url)
{
    QString filename = locationEditCurrentText();
    selection.clear();

    KUrlComboBox* pathCombo = urlNavigator->editor();
    if ( pathCombo->count() != 0 ) { // little hack
        pathCombo->setUrl( url );
    }

    bool blocked = locationEdit->blockSignals( true );
    if ( keepLocation ) {
        locationEdit->changeUrl( 0, KIcon( KMimeType::iconNameForUrl( filename ) ), filename );
        locationEdit->lineEdit()->setModified( true );
    }

    locationEdit->blockSignals( blocked );

    urlNavigator->setUrl(  url );

    QString dir = url.url(KUrl::AddTrailingSlash);
    // is trigged in ctor before completion object is set
    KUrlCompletion *completion = dynamic_cast<KUrlCompletion*>( locationEdit->completionObject() );
    if( completion )
        completion->setDir( dir );

    if ( placesView )
        placesView->setUrl( url );
}

void KFileWidgetPrivate::_k_locationAccepted( const QString& url )
{
    ops->setCurrentItem( url );
    q->slotOk();
}

void KFileWidgetPrivate::_k_locationActivated( const QString& url )
{
    // the location has been activated by selecting an entry
    // from the combo box of the location bar
    q->setUrl( url );
}

void KFileWidgetPrivate::_k_enterUrl( const KUrl& url )
{
    KUrl fixedUrl( url );
    // append '/' if needed: url combo does not add it
    // tokenize() expects it because uses KUrl::setFileName()
    fixedUrl.adjustPath( KUrl::AddTrailingSlash );
    q->setUrl( fixedUrl );
}

void KFileWidgetPrivate::_k_enterUrl( const QString& url )
{
    _k_enterUrl( KUrl( KUrlCompletion::replacedPath( url, true, true )) );
}


void KFileWidget::setSelection(const QString& url)
{
    kDebug(kfile_area) << "setSelection " << url;

    if (url.isEmpty()) {
        d->selection.clear();
        return;
    }

    KUrl u = d->getCompleteUrl(url);
    if (!u.isValid()) { // if it still is
        kWarning() << url << " is not a correct argument for setSelection!";
        return;
    }

    // Honor protocols that do not support directory listing
    if (!KProtocolManager::supportsListing(u))
        return;

    /* we strip the first / from the path to avoid file://usr which means
     *  / on host usr
     */
    KIO::UDSEntry entry;
    bool res = KIO::NetAccess::stat(u, entry, this);
    KFileItem i(entry, u);
    //    KFileItem i(u.path());
    kDebug(kfile_area) << "KFileItem " << u.path() << " " << i.isDir() << " " << u.isLocalFile() << " " << QFile::exists( u.path() );
    if ( res && i.isDir() && u.isLocalFile() && QFile::exists( u.path() ) ) {
        // trust isDir() only if the file is
        // local (we cannot stat non-local urls) and if it exists!
        // (as KFileItem does not check if the file exists or not
        // -> the statbuffer is undefined -> isDir() is unreliable) (Simon)
        setUrl(u, true);
    }
    else if ( res ) {
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
            kDebug(kfile_area) << "filename " << filename;
            d->selection = filename;
            d->setLocationText( u );

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
    else {
        d->setLocationText(url);
        d->locationEdit->lineEdit()->setModified( true );
    }
}

void KFileWidgetPrivate::_k_slotLoadingFinished()
{
    if ( !selection.isEmpty() )
        ops->setCurrentItem( selection );
}

void KFileWidgetPrivate::_k_fileCompletion( const QString& match )
{
    if ( match.isEmpty() && ops->view() ) {
        ops->view()->clearSelection();
    } else {
        ops->setCurrentItem( match );
        setDummyHistoryEntry( locationEdit->currentText(), KIconLoader::global()->loadMimeTypeIcon( KMimeType::iconNameForUrl( match ), KIconLoader::Small ), !locationEdit->currentText().isEmpty() );
        locationEdit->setCompletedText( match );
    }
}

void KFileWidgetPrivate::_k_slotLocationChanged( const QString& text )
{
    locationEdit->lineEdit()->setModified( true );

    if ( text.isEmpty() && ops->view() )
        ops->view()->clearSelection();

    if ( text.isEmpty() ) {
        removeDummyHistoryEntry();
    } else {
        setDummyHistoryEntry( text );
    }

    ops->setCurrentItem( text );

    updateFilter();
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
        KUrl u;
        if ( containsProtocolSection( filenames ) )
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

void KFileWidget::resizeEvent(QResizeEvent* event)
{
    d->updateSplitterSize();

    QWidget::resizeEvent(event);
}

void KFileWidget::showEvent(QShowEvent* event)
{
    if ( !d->hasView ) { // delayed view-creation
        d->ops->setView(KFile::Default);
        d->ops->view()->setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum ) );
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

    KUrlComboBox *combo = urlNavigator->editor();
    combo->setUrls( configGroup.readPathEntry( RecentURLs, QStringList() ), KUrlComboBox::RemoveTop );
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
    _k_toggleSpeedbar( configGroup.readEntry( ShowSpeedbar, true ) );

    // show or don't show the bookmarks
    _k_toggleBookmarks( configGroup.readEntry(ShowBookmarks, false) );

    // does the user want Automatically Select Extension?
    autoSelectExtChecked = configGroup.readEntry (AutoSelectExtChecked, DefaultAutoSelectExtChecked);
    updateAutoSelectExtension();

    // should the URL navigator use the breadcrumb navigation?
    urlNavigator->setUrlEditable( !configGroup.readEntry(BreadcrumbNavigation, true) );

    int w1 = q->minimumSize().width();
    int w2 = toolbar->sizeHint().width();
    if (w1 < w2)
        q->setMinimumWidth(w2);
}

void KFileWidgetPrivate::writeConfig(KConfigGroup &configGroup)
{
    KUrlComboBox *pathCombo = urlNavigator->editor();
    configGroup.writePathEntry( RecentURLs, pathCombo->urls() );
    //saveDialogSize( configGroup, KConfigGroup::Persistent | KConfigGroup::Global );
    configGroup.writeEntry( PathComboCompletionMode, static_cast<int>(pathCombo->completionMode()) );
    configGroup.writeEntry( LocationComboCompletionMode, static_cast<int>(locationEdit->completionMode()) );

    const bool showSpeedbar = placesView && !placesView->isHidden();
    configGroup.writeEntry( ShowSpeedbar, showSpeedbar );
    if (showSpeedbar) {
        const QList<int> sizes = placesViewSplitter->sizes();
        Q_ASSERT( sizes.count() > 0 );
        configGroup.writeEntry( SpeedbarWidth, sizes[0] );
    }

    configGroup.writeEntry( ShowBookmarks, bookmarkHandler != 0 );
    configGroup.writeEntry( AutoSelectExtChecked, autoSelectExtChecked );
    configGroup.writeEntry( BreadcrumbNavigation, !urlNavigator->isUrlEditable() );

    ops->writeConfig(configGroup);
}


void KFileWidgetPrivate::readRecentFiles( KConfig *kc )
{
    KConfigGroup cg( kc, ConfigGroup );

    locationEdit->setMaxItems( cg.readEntry( RecentFilesNumber,
                                             DefaultRecentURLsNumber ) );
    locationEdit->setUrls( cg.readPathEntry( RecentFiles, QStringList() ),
                           KUrlComboBox::RemoveBottom );
    locationEdit->setCurrentIndex( -1 );
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
       // don't use KStandardGuiItem::open() here which has trailing ellipsis!
       d->okButton->setGuiItem( KGuiItem( i18n( "&Open" ), "document-open") );
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

void KFileWidgetPrivate::_k_slotAutoSelectExtClicked()
{
    kDebug (kfile_area) << "slotAutoSelectExtClicked(): "
                         << autoSelectExtCheckBox->isChecked() << endl;

    // whether the _user_ wants it on/off
    autoSelectExtChecked = autoSelectExtCheckBox->isChecked();

    // update the current filename's extension
    updateLocationEditExtension (extension /* extension hasn't changed */);
}

void KFileWidgetPrivate::_k_placesViewSplitterMoved()
{
    const QList<int> sizes = placesViewSplitter->sizes();
    speedBarWidth = sizes[0];
}

static QString getExtensionFromPatternList(const QStringList &patternList)
{
    QString ret;
    kDebug (kfile_area) << "\tgetExtension " << patternList;

    QStringList::ConstIterator patternListEnd = patternList.end();
    for (QStringList::ConstIterator it = patternList.begin();
         it != patternListEnd;
         ++it)
    {
        kDebug (kfile_area) << "\t\ttry: \'" << (*it) << "\'";

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

    kDebug (kfile_area) << "Figure out an extension: ";
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

                if (mime)
                {
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
                  "saving files with extensions:<br />"
                  "<ol>"
                    "<li>Any extension specified in the <b>%1</b> text "
                    "area will be updated if you change the file type "
                    "to save in.<br />"
                    "<br /></li>"
                    "<li>If no extension is specified in the <b>%2</b> "
                    "text area when you click "
                    "<b>Save</b>, %3 will be added to the end of the "
                    "filename (if the filename does not already exist). "
                    "This extension is based on the file type that you "
                    "have chosen to save in.<br />"
                    "<br />"
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

    QString urlStr = locationEditCurrentText();
    if (urlStr.isEmpty())
        return;

    KUrl url = getCompleteUrl(urlStr);
    kDebug (kfile_area) << "updateLocationEditExtension (" << url << ")";

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
            kDebug (kfile_area) << "\tfile exists";

            if (t.isDir())
            {
                kDebug (kfile_area) << "\tisDir - won't alter extension";
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
        else if (extension.length() && fileName.endsWith (extension))
            fileName.truncate (len - extension.length());
        // can only handle "single extensions"
        else
            fileName.truncate (dot);

        // add extension
        const QString newText = urlStr.left (fileNameOffset) + fileName + extension;
        if ( newText != locationEditCurrentText() )
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
        const QString urlStr = locationEditCurrentText();
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

// Updates the splitter size. This is necessary since we call to this method when the widget is
// shown as well as when it is resized. This is also very important, because this will be
// contained in other widget, which can try to resize this one, and make the places view be
// wider than what the user wanted.
void KFileWidgetPrivate::updateSplitterSize()
{
    if (!placesViewSplitter) {
        return;
    }

    QList<int> sizes = placesViewSplitter->sizes();
    if (sizes.count() == 2) {
        // restore width of speedbar
        KConfigGroup configGroup( KGlobal::config(), ConfigGroup );
        const int speedbarWidth = speedBarWidth == -1 ? configGroup.readEntry( SpeedbarWidth, placesView->sizeHintForColumn(0) )
                                                      : speedBarWidth;
        const int availableWidth = q->width();
        sizes[0] = speedbarWidth + 1; // without this pixel, our places view is reduced 1 pixel each time is shown.
        sizes[1] = availableWidth - speedbarWidth - 1;
        placesViewSplitter->setSizes( sizes );
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

    kDebug (kfile_area) << "appendExtension(" << url << ")";

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
        kDebug (kfile_area) << "\tfile exists - won't append extension";
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
        kDebug (kfile_area) << "\tstrip trailing dot";
        url.setFileName (fileName.left (len - 1));
    }
    // evilmatically append extension :) if the user hasn't specified one
    else if (unspecifiedExtension)
    {
        kDebug (kfile_area) << "\tappending extension \'" << extension << "\'...";
        url.setFileName (fileName + extension);
        kDebug (kfile_area) << "\tsaving as \'" << url << "\'";
    }
}


// adds the selected files/urls to 'recent documents'
void KFileWidgetPrivate::addToRecentDocuments()
{
    int m = ops->mode();
    int atmost = KRecentDocument::maximumItems();
    //don't add more than we need. KRecentDocument::add() is pretty slow

    if ( m & KFile::LocalOnly ) {
        const QStringList files = q->selectedFiles();
        QStringList::ConstIterator it = files.begin();
        for ( ; it != files.end() && atmost > 0; ++it ) {
            KRecentDocument::add( *it );
            atmost--;
        }
    }

    else { // urls
        KUrl::List urls = q->selectedUrls();
        KUrl::List::ConstIterator it = urls.begin();
        for ( ; it != urls.end() && atmost > 0; ++it ) {
            if ( (*it).isValid() ) {
                KRecentDocument::add( *it );
                atmost--;
            }
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

void KFileWidgetPrivate::_k_toggleSpeedbar( bool show )
{
    if ( show )
    {
        if ( !placesView )
            initSpeedbar();

        placesView->show();

        // check to see if they have a home item defined, if not show the home button
        KUrl homeURL;
        homeURL.setPath( QDir::homePath() );
        KFilePlacesModel *model = static_cast<KFilePlacesModel*>(placesView->model());
        for ( int rowIndex = 0 ; rowIndex < placesView->model()->rowCount() ; rowIndex++ )
        {
            QModelIndex index = model->index(rowIndex, 0);
            KUrl url = model->url(index);

            if ( homeURL.equals( url, KUrl::CompareWithoutTrailingSlash ) ) {
                toolbar->removeAction( ops->actionCollection()->action( "home" ) );
                break;
            }
        }
    }
    else
    {
        if (placesView)
            placesView->hide();

        QAction* homeAction = ops->actionCollection()->action( "home" );
        QAction* reloadAction = ops->actionCollection()->action( "reload" );
        if ( !toolbar->actions().contains(homeAction) )
            toolbar->insertAction( reloadAction, homeAction );
    }

    static_cast<KToggleAction *>(q->actionCollection()->action("toggleSpeedbar"))->setChecked( show );
}

void KFileWidgetPrivate::_k_toggleBookmarks(bool show)
{
    if (show)
    {
        if (bookmarkHandler)
        {
            return;
        }

        bookmarkHandler = new KFileBookmarkHandler( q );
        q->connect( bookmarkHandler, SIGNAL( openUrl( const QString& )),
                    SLOT( _k_enterUrl( const QString& )));

        bookmarkButton = new KActionMenu(KIcon("bookmarks"),i18n("Bookmarks"), q);
        bookmarkButton->setDelayed(false);
        q->actionCollection()->addAction("bookmark", bookmarkButton);
        bookmarkButton->setMenu(bookmarkHandler->menu());
        bookmarkButton->setWhatsThis(i18n("<qt>This button allows you to bookmark specific locations. "
                                "Click on this button to open the bookmark menu where you may add, "
                                "edit or select a bookmark.<br /><br />"
                                "These bookmarks are specific to the file dialog, but otherwise operate "
                                "like bookmarks elsewhere in KDE.</qt>"));
        toolbar->addAction(bookmarkButton);
    }
    else if (bookmarkHandler)
    {
        delete bookmarkHandler;
        bookmarkHandler = 0;
        delete bookmarkButton;
        bookmarkButton = 0;
    }

    static_cast<KToggleAction *>(q->actionCollection()->action("toggleBookmarks"))->setChecked( show );
}

// static
KUrl KFileWidget::getStartUrl( const KUrl& startDir,
                               QString& recentDirClass )
{
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
    if ( directory.isValid() )
        *lastDirectory = directory;
}

void KFileWidgetPrivate::setNonExtSelection()
{
    // Enhanced rename: Don't highlight the file extension.
    QString filename = locationEditCurrentText();
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
    delete d->bottomCustomWidget;
    d->bottomCustomWidget = widget;

    // add it to the dialog, below the filter list box.

    // Change the parent so that this widget is a child of the main widget
    d->bottomCustomWidget->setParent( this );

    d->vbox->addWidget( d->bottomCustomWidget );
    //d->vbox->addSpacing(3); // can't do this every time...

    // FIXME: This should adjust the tab orders so that the custom widget
    // comes after the Cancel button. The code appears to do this, but the result
    // somehow screws up the tab order of the file path combo box. Not a major
    // problem, but ideally the tab order with a custom widget should be
    // the same as the order without one.
    setTabOrder(d->cancelButton, d->bottomCustomWidget);
    setTabOrder(d->bottomCustomWidget, d->urlNavigator);
}

void KFileWidget::setCustomWidget(const QString& text, QWidget* widget)
{
    delete d->labeledCustomWidget;
    d->labeledCustomWidget = widget;

    QLabel* label = new QLabel(text, this);
    d->lafBox->addWidget(label, 2, 0, Qt::AlignVCenter);
    d->lafBox->addWidget(widget, 2, 1, Qt::AlignVCenter);
}

void KFileWidget::virtual_hook( int id, void* data )
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}

QString KFileWidgetPrivate::locationEditCurrentText() const
{
    return QDir::fromNativeSeparators(locationEdit->currentText().trimmed());
}

#include "kfilewidget.moc"
