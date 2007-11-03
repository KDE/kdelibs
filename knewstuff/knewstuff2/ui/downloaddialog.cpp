/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *             (C) 2007 Dirk Mueller <mueller@kde.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

// own include
#include "downloaddialog.h"

// qt/kde includes
#include <QtCore/QFile>
#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QFont>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtCore/QMutableVectorIterator>
#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QScrollArea>
#include <QtGui/QApplication>
#include <QtGui/QTextDocument>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcomponentdata.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <ktitlewidget.h>
#include <ktoolinvocation.h>

#include "knewstuff2/core/provider.h"
#include "knewstuff2/core/providerhandler.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"

#include "knewstuff2/dxs/dxs.h"

#include "knewstuff2/ui/qprogressindicator.h"

// local includes
#include "kdxsbutton.h"
#include "qasyncpixmap.h"

using namespace KNS;

ItemsView::ItemsView( DownloadDialog * newStuffDialog, QWidget* _parent )
    : QScrollArea( _parent ),
    m_newStuffDialog( newStuffDialog ), m_root( 0 ), m_sorting( 0 )
{
    setFrameStyle(QFrame::Plain | QFrame::StyledPanel);
    setWidgetResizable(true);
}

ItemsView::~ItemsView()
{
    clear();
}

void ItemsView::setEngine(DxsEngine *engine)
{
    m_engine = engine;
}

void ItemsView::setItems( /*const Entry::List & itemList*/ QMap<const Feed*, KNS::Entry::List> itemList )
{
    clear();
    m_entries = itemList;
    buildContents();
}

void ItemsView::setProviders( QMap<Entry*, const Provider*> providers )
{
    m_providers = providers;
}

void ItemsView::setSorting( int sortType )
{
    m_sorting = sortType;
    buildContents();
}

void ItemsView::updateItem( Entry *entry )
{
    Q_UNUSED(entry);
    // XXX ???
    // TODO: proxy over
}

void ItemsView::buildContents()
{
    if(m_root)
    {
        m_root = takeWidget();
        delete m_root;
    }

    if(m_entries.keys().count() == 0)
    {
        // FIXME: warning?
        return;
    }

    m_root = new QWidget(this);
    m_root->setBackgroundRole(QPalette::Base);
    QGridLayout* _layout = new QGridLayout(m_root);
    _layout->setVerticalSpacing (10);

    Entry::List entries = m_entries[m_entries.keys().first()];
    Entry::List::iterator it = entries.begin(), iEnd = entries.end();
    for ( unsigned row = 0; it != iEnd; ++it, ++row )
    {
        Entry* entry = (*it);

        EntryView *part = new EntryView(m_root);
        part->setBackgroundRole(row & 1 ? QPalette::AlternateBase : QPalette::Base);
        _layout->addWidget(part, row*2, 1, 2, 1);

        KDXSButton *dxsbutton = new KDXSButton(m_root);
        dxsbutton->setEntry(entry);
        if(m_providers.contains(entry))
            dxsbutton->setProvider(m_providers[entry]);
        dxsbutton->setEngine(m_engine);

        QString imageurl = entry->preview().representation();
        if(!imageurl.isEmpty())
        {
            QLabel *f = new QLabel(this);
            f->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            QAsyncPixmap *pix = new QAsyncPixmap(imageurl, this);
            f->setFixedSize(64, 64);
            connect(pix, SIGNAL(signalLoaded(const QPixmap&)),
                    f, SLOT(setPixmap(const QPixmap&)));
            _layout->addWidget(f, row*2, 0);
        }
        _layout->setRowStretch(row*2, 1);
        _layout->addWidget(dxsbutton, row*2+1, 0);

        part->setEntry(entry);
    }

    setWidget(m_root);
}

void ItemsView::clear()
{
    m_entries.clear();
    //m_providers.clear();
    m_pixmaps.clear();
}

EntryView::EntryView( QWidget * _parent )
    : QLabel( _parent )
{
    connect(this, SIGNAL(linkActivated(const QString&)), SLOT(urlSelected(const QString&)));
}

void EntryView::setEntry( Entry *entry )
{
    m_entry = entry;
    buildContents();
}

void EntryView::updateEntry( Entry *entry )
{
    // get item id string and iformations
    QString idString = QString::number( (unsigned long)entry );
    //            AvailableItem::State state = item->state();
    //            bool showProgress = state != AvailableItem::Normal;
    //            int pixelProgress = showProgress ? (int)(item->progress() * 80.0) : 0;

    // perform internal scripting operations over the element
    //            executeScript( "document.getElementById('" + idString + "').style.color='red'" );
    //            executeScript( "document.getElementById('bar" + idString + "').style.width='" +
    //                           QString::number( pixelProgress ) + "px'" );
    //            executeScript( "document.getElementById('bc" + idString + "').style.backgroundColor='" +
    //                           (showProgress ? "gray" : "transparent") + "'" );
    //            executeScript( "document.getElementById('btn" + idString + "').value='" +
    //                           (item->installed() ? i18n( "Uninstall" ) : i18n( "Install" )) + "'" );
}

void EntryView::buildContents()
{
    setTheAaronnesqueStyle();
    // write the html header and contents manipulation scripts
    QString t;

    t += "<qt>";

    // precalc the status icon
    Entry::Status status = m_entry->status();
    QString statusIcon;
    KIconLoader *loader = KIconLoader::global();

    switch (status)
    {
        case Entry::Invalid:
            statusIcon = "<img src='" + loader->iconPath("dialog-error", -KIconLoader::SizeSmall) + "' />";
            break;
        case Entry::Downloadable:
            // find a good icon to represent downloadable data
            //statusIcon = "<img src='" + loader->iconPath("network-server", -KIconLoader::SizeSmall) + "' />";
            break;
        case Entry::Installed:
            statusIcon = "<img src='" + loader->iconPath("ok", -KIconLoader::SizeSmall) + "' />";
            break;
        case Entry::Updateable:
            statusIcon = "<img src='" + loader->iconPath("xclock", -KIconLoader::SizeSmall) + "' />";
            break;
        case Entry::Deleted:
            statusIcon = "<img src='" + loader->iconPath("user-trash", -KIconLoader::SizeSmall) + "' />";
            break;
    }

    // precalc the image string
    QString imageString = m_entry->preview().representation();
    if ( !imageString.isEmpty() )
        imageString = "<img class='leftImage' src='" + Qt::escape(imageString) + "' border='0' />";

    // precalc the title string
    QString titleString = m_entry->name().representation();
    if ( !m_entry->version().isEmpty() ) titleString += " v." + Qt::escape(m_entry->version());

    // precalc the string for displaying stars (normal+grayed)
    int starPixels = 11 + 11 * (m_entry->rating() / 10);
    QString starsString = "<div class='star' style='width: " + QString::number( starPixels ) + "px;'></div>";
    int grayPixels = 22 + 22 * (m_entry->rating() / 20);
    starsString = "<div class='starbg' style='width: " + QString::number( grayPixels ) + "px;'>" + starsString + "</div>";

    // precalc the string for displaying author (parsing email)
    KNS::Author author = m_entry->author();
    QString authorString = author.name();

    QString emailString = author.email();
    if (!emailString.isEmpty())
    {
        authorString = "<a href='mailto:" + Qt::escape( emailString) + "'>"
            + Qt::escape(authorString) + "</a>";
    }

    // write the HTML code for the current item
    t += QLatin1String("<table class='contentsHeader' cellspacing='2' cellpadding='0'>")
         + "<tr>"
         +   "<td>" + statusIcon + Qt::escape(titleString) + "</td>"
         +   "<td align='right'>" + starsString +  "</td>"
         + "</tr>"
         + "<tr>"
         +   "<td colspan='2' class='contentsBody'>"
         +      Qt::escape(m_entry->summary().representation())
         +   "</td>"
         + "</tr>"
         + "<tr>"
         +   "<td colspan='2' class='contentsFooter'>"
         +     "<em>" + authorString + "</em>, "
         +       KGlobal::locale()->formatDate( m_entry->releaseDate(), KLocale::ShortDate )
         +   "</td>"
         + "</tr>"
         + "</table>";

    t += "</qt>";
    setText(t);
}

void EntryView::setTheAaronnesqueStyle()
{
    QString hoverColor = "#000000"; //QApplication::palette().active().highlightedText().name();
    QString hoverBackground = "#f8f8f8"; //QApplication::palette().active().highlight().name();
    QString starIconPath = KStandardDirs::locate( "data", "kpdf/pics/ghns_star.png" );
    QString starBgIconPath = KStandardDirs::locate( "data", "kpdf/pics/ghns_star_gray.png" );

    // default elements style
    QString s;
    s += "body { background-color: white; color: black; padding: 0; margin: 0; }";
    s += "table, td, th { padding: 0; margin: 0; text-align: left; }";
    s += "input { color: #000080; font-size:120%; }";

    // the main item container (custom element)
    s += ".itemBox { background-color: white; color: black; width: 100%;  border-bottom: 1px solid gray; margin: 0px 0px; }";
    s += ".itemBox:hover { background-color: " + hoverBackground + "; color: " + hoverColor + "; }";

    // s of the item elements (4 cells with multiple containers)
    s += ".leftColumn { width: 100px; height:100%; text-align: center; }";
    s += ".leftImage {}";
    s += ".leftButton {}";
    s += ".leftProgressContainer { width: 82px; height: 10px; background-color: transparent; }";
    s += ".leftProgressBar { left: 1px; width: 0px; top: 1px; height: 8px; background-color: red; }";
    s += ".contentsColumn { vertical-align: top; }";
    s += ".contentsHeader { width: 100%; font-size: 120%; font-weight: bold; border-bottom: 1px solid #c8c8c8; }";
    s += ".contentsBody {}";
    s += ".contentsFooter {}";
    s += ".star { width: 0px; height: 24px; background-image: url(" + starIconPath + "); background-repeat: repeat-x; }";
    s += ".starbg { width: 110px; height: 24px; background-image: url(" + starBgIconPath + "); background-repeat: repeat-x; }";
    setStyleSheet(s);
}

void EntryView::urlSelected( const QString &link)
{
    //kDebug() << "Clicked on URL " << link;

    KUrl url( link );
    QString urlProtocol = url.protocol();
    QString urlPath = url.path();

    if ( urlProtocol == "mailto" )
    {
        // clicked over a mail address
        // FIXME: if clicked with MRB, show context menu with IM etc.
        // FIXME: but RMB never reaches this method?!
        KToolInvocation::invokeMailer( url );
    }
    else if ( urlProtocol == "item" )
    {
        // clicked over an item
        bool ok;
        unsigned long itemPointer = urlPath.toULong( &ok );
        if ( !ok )
        {
            kWarning() << "ItemsView: error converting item pointer.";
            return;
        }

        // I love to cast pointers
        Entry *entry = (Entry*)itemPointer;
        if ( entry != m_entry )
        {
            kWarning() << "ItemsView: error retrieving item pointer.";
            return;
        }

        // XXX ???
        // install/uninstall the item
        //                if ( item->installed() )
        //                    m_newStuffDialog->removeItem( item );   // synchronous
        //                else
        //                    m_newStuffDialog->installItem( item );  // asynchronous
    }
}

DownloadDialog::DownloadDialog( DxsEngine* _engine, QWidget * _parent )
    : KDialog( _parent )
{
    m_engine = _engine;

    setButtons(KDialog::Close);

    // initialize the private classes
    messageTimer = new QTimer( this );
    messageTimer->setSingleShot( true );
    connect( messageTimer, SIGNAL( timeout() ), SLOT( slotResetMessage() ) );

    networkTimer = new QTimer( this );
    connect( networkTimer, SIGNAL( timeout() ), SLOT( slotNetworkTimeout() ) );

    // popuplate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    QBoxLayout * mainLayout = new QVBoxLayout(_mainWidget);
    mainLayout->setMargin(0);

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "kpdf/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way


    // create title widget
    titleWidget = new KTitleWidget(_mainWidget);

    // create the control panel
    QFrame * panelFrame = new QFrame(_mainWidget);
    panelFrame->setFrameStyle( QFrame::NoFrame );
    QGridLayout * panelLayout = new QGridLayout( panelFrame);
    panelLayout->setMargin(0);
    // add widgets to the control panel
    QLabel * label1 = new QLabel( i18n("Show:")/*, panelFrame*/ );
    panelLayout->addWidget( label1, 0, 0 );
    typeCombo = new QComboBox( false/*, panelFrame*/ );
    panelLayout->addWidget( typeCombo, 0, 1 );
    typeCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    typeCombo->setMinimumWidth( 150 );
    typeCombo->setEnabled( false );
    connect( typeCombo, SIGNAL( activated(int) ), SLOT( slotLoadProviderDXS(int) ) );

    QLabel * label2 = new QLabel( i18n("Order by:")/*, panelFrame*/ );
    panelLayout->addWidget( label2, 0, 2 );
    sortCombo = new QComboBox( false/*, panelFrame*/ );
    panelLayout->addWidget( sortCombo, 0, 3 );
    sortCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    sortCombo->setMinimumWidth( 100 );
    sortCombo->setEnabled( false );
    sortCombo->addItem( SmallIcon( "text" ), i18n("Name") );
    sortCombo->addItem( SmallIcon( "favorites" ), i18n("Rating") );
    sortCombo->addItem( SmallIcon( "go-jump-today" ), i18n("Most recent") );
    sortCombo->addItem( SmallIcon( "cdcopy" ), i18n("Most downloads") );
    connect( sortCombo, SIGNAL( activated(int) ), SLOT( slotSortingSelected(int) ) );

    QLabel * label3 = new QLabel( i18n("Search:"), panelFrame );
    panelLayout->addWidget( label3, 1, 0 );
    searchLine = new KLineEdit( panelFrame );
    panelLayout->addWidget( searchLine, 1, 1, 1, 3 );
    searchLine->setEnabled( false );
    searchLine->setClearButtonShown(true);

    m_progress = new QProgressIndicator( this );
    panelLayout->addWidget( m_progress, 2, 0, 2, 4 );

    // create the ItemsView used to display available items
    itemsView = new ItemsView( this, _mainWidget );
    itemsView->setEngine(m_engine);

    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(panelFrame);
    mainLayout->addWidget(itemsView);

    statusLabel = new QLabel(_mainWidget);
    mainLayout->addWidget(statusLabel);

    // start with a nice size
    // FIXME: this should load the last size the user set it to.
    resize( 700, 400 );

    setWindowTitle(i18n("Get Hot New Stuff!"));
    titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                  "%1 Add On Installer",
                                  KGlobal::mainComponent().aboutData()->programName()));
    titleWidget->setPixmap(KGlobal::mainComponent().aboutData()->appName());

    connect( this, SIGNAL( closeClicked() ), SLOT( accept() ) );
    connect( m_engine, SIGNAL( signalPayloadProgress( KUrl, int ) ), SLOT( slotPayloadProgress( KUrl, int ) ) );
}

DownloadDialog::~DownloadDialog()
{
}

void DownloadDialog::displayMessage( const QString & msg, KTitleWidget::MessageType type, int timeOutMs )
{
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    titleWidget->setComment( msg, type );

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        //kDebug() << "starting the message timer for " << timeOutMs;
        messageTimer->start( timeOutMs );
    }
}

void DownloadDialog::installItem( Entry *entry )
{
    // safety check
//    if ( item->url().isEmpty() || item->destinationPath().isEmpty() )
//    {
//        displayMessage( i18n("I don't know how to install this. Sorry, my fault."), Info );
//        return;
//    }

    //TODO check for AvailableItem deletion! (avoid broken pointers) -> cancel old jobs
    slotDownloadItem( entry );
}

void DownloadDialog::removeItem( Entry *entry )
{
    Q_UNUSED(entry);
//    displayMessage( i18n("%1 is no more installed.").arg( item->name().representation() ) );
}

void DownloadDialog::slotResetMessage() // SLOT
{
    titleWidget->setComment(QString());
}

void DownloadDialog::slotNetworkTimeout() // SLOT
{
    displayMessage( i18n("Timeout. Check internet connection!"), KTitleWidget::ErrorMessage );
}

void DownloadDialog::slotSortingSelected( int sortType ) // SLOT
{
    itemsView->setSorting( sortType );
}


///////////////// DXS ////////////////////

void DownloadDialog::slotLoadProviderDXS(int index)
{
    Q_UNUSED(index);

    QString category = typeCombo->currentText();
    QString categoryname = categorymap[category];

    //m_dxs->call_entries(categoryname, QString());
    // FIXME: use d->engine
}

void DownloadDialog::slotLoadProvidersListDXS()
{
}

void DownloadDialog::slotCategories(QList<KNS::Category*> categories)
{
    categorymap.clear();

    for(QList<KNS::Category*>::Iterator it = categories.begin(); it != categories.end(); ++it)
    {
        KNS::Category *category = (*it);
        //kDebug() << "Category: " << category->name().representation();
        QPixmap icon = DesktopIcon(category->icon().url(), 16);
        // FIXME: use icon from remote URLs (see non-DXS providers as well)
        typeCombo->addItem(icon, category->name().representation());
        categorymap[category->name().representation()] = category->id();
        // FIXME: better use global id, since names are not guaranteed
        //        to be unique
    }

    typeCombo->setEnabled(true);

    slotLoadProviderDXS(0);
}

void DownloadDialog::slotEntries(QList<KNS::Entry*> _entries)
{
	Q_UNUSED(_entries);

	//d->itemsView->setItems( entries );
	// FIXME: old API here
}

// FIXME: below here, those are for traditional GHNS

void DownloadDialog::addEntry(Entry *entry, const Feed *feed, const Provider *provider)
{
	Q_UNUSED(provider);

        Entry::List e = entries[feed];
        e.append(entry);
	entries[feed] = e;

	// FIXME: what if entry belongs to more than one provider at once?
	providers[entry] = provider;

	//kDebug() << "downloaddialog: addEntry to list of size " << entries.size();
}

void DownloadDialog::refresh()
{
	itemsView->setProviders(providers);
	itemsView->setItems(entries);
	// FIXME: this method has side effects (rebuilding HTML!!!)

	for(int i = 0; i < entries.keys().count(); i++)
	{
		const Feed *feed = entries.keys().at(i);
		if(!feed)
		{
			//kDebug() << "INVALID FEED?!";
			continue;
		}
		//QPixmap icon = DesktopIcon(QString(), 16);
		//d->typeCombo->addItem(icon, feed->name().representation());
		typeCombo->addItem(feed->name().representation());
		// FIXME: see DXS categories
	}
	typeCombo->setEnabled(true);
	sortCombo->setEnabled(true);
	// FIXME: sort combo must be filled with feeds instead of being prefilled
}

///////////////// DXS ////////////////////

//BEGIN File(s) Transferring
void DownloadDialog::slotDownloadItem( Entry *entry )
{
//Q_UNUSED(entry);
    itemsView->updateItem(entry);
/*
XXX update item status
    item->setState( AvailableItem::Installing );
    item->setProgress( 0.0 );
    d->itemsView->updateItem( item );

XXX inform the user
    displayMessage( i18n("Installing '%1', this could take some time ...").arg( item->name().representation() ), 3000 );
*/
}

void DownloadDialog::slotPayloadProgress(KUrl payload, int percentage)
{
    m_progress->addProgress(payload, percentage);
}

/*void DownloadDialog::slotItemMessage( KJob * job, const QString & message )
{
    AvailableItem * item = d->transferJobs[ job ].item;
    kDebug() << "Name: " << item->name().representation() << " msg: '" << message << "'.";
    d->itemsView->updateItem( item );
}

void DownloadDialog::slotItemPercentage( KJob * job, unsigned long percent )
{
    AvailableItem * item = d->transferJobs[ job ].item;
    item->setProgress( (float)percent / 100.0 );
    d->itemsView->updateItem( item );
}

void DownloadDialog::slotItemResult( KJob * job )
{
    item->setState( AvailableItem::Normal );
    item->setProgress( 100.0 );
    d->itemsView->updateItem( item );

}*/
//END File(s) Transferring

// fault/error from kdxsbutton
void DownloadDialog::slotFault()
{
	KMessageBox::error(this,
		i18n("A protocol fault has occurred. The request has failed."),
		i18n("Desktop Exchange Service"));
}

void DownloadDialog::slotError()
{
	KMessageBox::error(this,
		i18n("A network error has occurred. The request has failed."),
		i18n("Desktop Exchange Service"));
}

#include "downloaddialog.moc"
