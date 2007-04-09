/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

// qt/kde includes
#include <QtCore/QFile>
#include <QtGui/QWidget>
#include <QtCore/QTimer>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QFont>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtCore/QMutableVectorIterator>
#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QScrollArea>
#include <QtGui/QApplication>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <khtml_part.h>
#include <khtmlview.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <ktoolinvocation.h>

#include "knewstuff2/core/provider.h"
#include "knewstuff2/core/providerhandler.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"

#include "knewstuff2/dxs/dxs.h"

// local includes
#include "downloaddialog.h"

#include "kdxsbutton.h"
#include "qasyncpixmap.h"
#include "qasyncframe.h"

using namespace KNS;

class MyEventFilter : public QObject
{
    public:
        MyEventFilter(QObject *parent)
        : QObject(parent)
        {
        }

    protected:
        bool eventFilter(QObject *o, QEvent *e)
        {
            if(e->type() == QEvent::Wheel)
            {
                kapp->sendEvent((dynamic_cast<QWidget*>(o))->parentWidget(), e);
                return true;
            }
	    else if(e->type() == QEvent::KeyPress)
	    {
                kapp->sendEvent((dynamic_cast<QWidget*>(o))->parentWidget(), e);
	    	return true;
	    }
            return false;
        }
};


/** GUI/CORE: HTML Widget for exactly one AvailableItem::List */
class ItemsViewPart : public KHTMLPart
{
    public:
        ItemsViewPart( QWidget * parentWidget = 0 )
            : KHTMLPart( parentWidget )
        {
            // customize functionality
            setJScriptEnabled( true );
            setJavaEnabled( false );
            setMetaRefreshEnabled( false );
            setPluginsEnabled( false );
            setStandardFont( QApplication::font().family() );
            // 100% is too large! less is better
            setZoomFactor( 70 );

            MyEventFilter *f = new MyEventFilter(this);
            view()->installEventFilter(f);
        }

        void setEntry( Entry *entry )
        {
            m_entry = entry;
            buildContents();
        }

        void updateEntry( Entry *entry )
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

    private:
        // generate the HTML contents to be displayed by the class itself
        void buildContents()
        {
            begin();
            setTheAaronnesqueStyle();
            // write the html header and contents manipulation scripts
            write( "<html><body>" );

// XXX ???
//            write( "<div style='font-size:2em; border-bottom: 1px solid blue; background-color: #808080;'>Updated</div>" );

            Entry *entry = m_entry;

            // precalc the image string
            QString imageString = entry->preview().representation();
            if ( imageString.length() > 1 )
                imageString = "<div class='leftImage'><img src='" + imageString + "' border='0'></div>";

            // precalc the title string
            QString titleString = entry->name().representation();
            if ( entry->version().length() > 0 )
                titleString += " v." + entry->version();

            // precalc button's text
///            QString buttonText = item->installed() ? i18n( "Uninstall" ) : i18n( "Install" );

            // precalc item's dynamic strings
            QString idString = QString::number( (unsigned long)entry );
            QString clickString = "window.location.href=\"item:" + idString + "\";";

            // precalc the string for displaying stars (normal+grayed)
            int starPixels = 11 + 11 * (entry->rating() / 10);
            QString starsString = "<div class='star' style='width: " + QString::number( starPixels ) + "px;'></div>";
            int grayPixels = 22 + 22 * (entry->rating() / 20);
            starsString = "<div class='starbg' style='width: " + QString::number( grayPixels ) + "px;'>" + starsString + "</div>";

            // precalc the string for displaying author (parsing email)
	    KNS::Author author = entry->author();
            QString authorString = author.name();
            QString emailString = author.email();
            if (!emailString.isEmpty())
            {
                authorString = "<a href='mailto:" + emailString + "'>" + authorString + "</a>";
            }

            // write the HTML code for the current item
            write( QString(
                  "<table class='itemBox' id='" + idString + "'><tr>"
// XXX ???
//                     "<td class='leftColumn'>"
//                        // image
//                          + imageString +
//                        // progress bar (initially transparent)
//                        "<div class='leftProgressContainer' id='bc" + idString + "'>"
//                          "<div id='bar" + idString + "' class='leftProgressBar'></div>"
//                        "</div>"
//                        // button
//                        "<div class='leftButton'>"
//                          "<input type='button' id='btn" + idString + "' onClick='" + clickString + "' value='" + buttonText + "'>"
//                        "</div>"
//                     "</td>"
                     "<td class='contentsColumn'>"
                        // contents header: item name/score
                        "<table class='contentsHeader' cellspacing='2' cellpadding='0'><tr>"
                          "<td>" + titleString + "</td>"
                          "<td align='right'>" + starsString +  "</td>"
                        "</tr></table>"
                        // contents body: item description
                        "<div class='contentsBody'>"
                          + entry->summary().representation() +
                        "</div>"
                        // contents footer: author's name/date
                        "<div class='contentsFooter'>"
                          "<em>" + authorString + "</em>, "
                          + KGlobal::locale()->formatDate( entry->releaseDate(), KLocale::ShortDate ) +
                        "</div>"
                     "</td>"
                  "</tr></table>" )
            );

            write( "</body></html>" );
            end();
        }

        // this is the stylesheet we use
        void setTheAaronnesqueStyle()
        {
            QString hoverColor = "#000000"; //QApplication::palette().active().highlightedText().name();
            QString hoverBackground = "#f8f8f8"; //QApplication::palette().active().highlight().name();
            QString starIconPath = KStandardDirs::locate( "data", "kpdf/pics/ghns_star.png" );
            QString starBgIconPath = KStandardDirs::locate( "data", "kpdf/pics/ghns_star_gray.png" );

            // default elements style
            QString style;
            style += "body { background-color: white; color: black; padding: 0; margin: 0; }";
            style += "table, td, th { padding: 0; margin: 0; text-align: left; }";
            style += "input { color: #000080; font-size:120%; }";

            // the main item container (custom element)
            style += ".itemBox { background-color: white; color: black; width: 100%;  border-bottom: 1px solid gray; margin: 0px 0px; }";
            style += ".itemBox:hover { background-color: " + hoverBackground + "; color: " + hoverColor + "; }";

            // style of the item elements (4 cells with multiple containers)
            style += ".leftColumn { width: 100px; height:100%; text-align: center; }";
            style += ".leftImage {}";
            style += ".leftButton {}";
            style += ".leftProgressContainer { width: 82px; height: 10px; background-color: transparent; }";
            style += ".leftProgressBar { left: 1px; width: 0px; top: 1px; height: 8px; background-color: red; }";
            style += ".contentsColumn { vertical-align: top; }";
            style += ".contentsHeader { width: 100%; font-size: 120%; font-weight: bold; border-bottom: 1px solid #c8c8c8; }";
            style += ".contentsBody {}";
            style += ".contentsFooter {}";
            style += ".star { width: 0px; height: 24px; background-image: url(" + starIconPath + "); background-repeat: repeat-x; }";
            style += ".starbg { width: 110px; height: 24px; background-image: url(" + starBgIconPath + "); background-repeat: repeat-x; }";
            setUserStyleSheet( style );
        }

        // handle clicks on page links/buttons
        void urlSelected( const QString &link, int button, int state, const QString &target, KParts::URLArgs args)
        {
	    kDebug() << "Clicked on URL " << link << endl;

	    Q_UNUSED(button);
	    Q_UNUSED(state);
	    Q_UNUSED(target);
	    Q_UNUSED(args);

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
                    kWarning() << "ItemsView: error converting item pointer." << endl;
                    return;
                }

                // I love to cast pointers
                Entry *entry = (Entry*)itemPointer;
                if ( entry != m_entry )
                {
                    kWarning() << "ItemsView: error retrieving item pointer." << endl;
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

    private:
        Entry *m_entry;
};


/** GUI/CORE: HTML Widget to operate on AvailableItem::List */
class ItemsView : public QScrollArea
{
    //Q_OBJECT
    public:
        ItemsView( DownloadDialog * newStuffDialog, QWidget * parentWidget )
            : QScrollArea( parentWidget ),
            m_newStuffDialog( newStuffDialog ), m_root( 0 ), m_sorting( 0 )
        {
        }

        ~ItemsView()
        {
            clear();
        }

	void setEngine(DxsEngine *engine)
	{
	    m_engine = engine;
	}

        void setItems( /*const Entry::List & itemList*/ QMap<const Feed*, KNS::Entry::List> itemList )
        {
            clear();
            m_entries = itemList;
            buildContents();
        }

        void setSorting( int sortType )
        {
            m_sorting = sortType;
            buildContents();
        }

        void updateItem( Entry *entry )
        {
            Q_UNUSED(entry);
// XXX ???
// TODO: proxy over
        }

    private:
        void buildContents()
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

            m_root = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(m_root);

            Entry::List entries = m_entries[m_entries.keys().first()];
            Entry::List::iterator it = entries.begin(), iEnd = entries.end();
            for ( ; it != iEnd; ++it )
            {
                Entry* entry = (*it);

                QFrame *itemview = new QFrame();
                QWidget *nav = new QWidget();
                ItemsViewPart *part = new ItemsViewPart();
                QHBoxLayout *l = new QHBoxLayout(itemview);
                l->addWidget(nav);
                l->addWidget(part->view());
                layout->addWidget(itemview);
		//part->view()->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
		part->view()->setFixedHeight(100);

                QVBoxLayout *l2 = new QVBoxLayout(nav);
                KDXSButton *dxsbutton = new KDXSButton();
                dxsbutton->setEntry(entry);
                dxsbutton->setProvider(0);
		dxsbutton->setEngine(m_engine);

                QString imageurl = entry->preview().representation();
		if(!imageurl.isEmpty())
		{
                    QAsyncFrame *f = new QAsyncFrame();
                    f->setFrameStyle(QFrame::Panel | QFrame::Sunken);
                    QAsyncPixmap *pix = new QAsyncPixmap(imageurl);
                    f->setFixedSize(64, 64);
                    connect(pix, SIGNAL(signalLoaded(QPixmap*)), f, SLOT(slotLoaded(QPixmap*)));
                    l2->addWidget(f);
		}
                l2->addStretch(1);
                l2->addWidget(dxsbutton);

                part->setEntry(entry);
            }

            setWidget(m_root);
        }

        // delete all the classes we own
        void clear()
        {
            m_entries.clear();
            m_pixmaps.clear();
        }

    private:
        DownloadDialog * m_newStuffDialog;
        QMap<const Feed*, Entry::List> m_entries;
        QWidget *m_root;
        int m_sorting;
        QMap<QPixmap*, QWidget*> m_pixmaps;
	DxsEngine *m_engine;
};


struct KNS::DownloadDialogPrivate
{
    // Contents
    QList< Provider * > providers;

    // gui related vars
    QWidget * parentWidget;
    QLineEdit * searchLine;
    QComboBox * typeCombo;
    QComboBox * sortCombo;
    ItemsView * itemsView;
    QLabel * messageLabel;

    // other classes
    QTimer * messageTimer;
    QTimer * networkTimer;
};

DownloadDialog::DownloadDialog( QWidget * parentWidget )
    : QDialog( parentWidget ), d( new DownloadDialogPrivate )
{
    // initialize the private classes
    d->parentWidget = parentWidget;

    d->messageTimer = new QTimer( this );
    connect( d->messageTimer, SIGNAL( timeout() ),
             this, SLOT( slotResetMessageColors() ) );

    d->networkTimer = new QTimer( this );
    connect( d->networkTimer, SIGNAL( timeout() ),
             this, SLOT( slotNetworkTimeout() ) );

    // popuplate dialog with stuff
    QBoxLayout * horLay = new QHBoxLayout( this/*, 11*/ ); // FIXME KDE4PORT

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "kpdf/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way

    // create right 'main' widget
    QWidget * rightLayouterWidget = new QWidget( this );
    QVBoxLayout * rightLayouter = new QVBoxLayout();
    rightLayouterWidget->setLayout( rightLayouter );
    rightLayouter->setSpacing( 6 );
    horLay->addWidget( rightLayouterWidget );

      // create upper label
      QLabel * mainLabel = new QLabel( i18n("All you ever wanted, in one click!") );
      QFont mainFont = mainLabel->font();
      mainFont.setBold( true );
      mainLabel->setFont( mainFont );

      // create the control panel
      QFrame * panelFrame = new QFrame();
      panelFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
      //QGridLayout * panelLayout = new QGridLayout( panelFrame, 2, 4, 11, 6 );
      //2, 4, 11, 6 ); // FIXME KDE4PORT
      QGridLayout * panelLayout = new QGridLayout( panelFrame);
        // add widgets to the control panel
        QLabel * label1 = new QLabel( i18n("Show:")/*, panelFrame*/ );
        panelLayout->addWidget( label1, 0, 0 );
        d->typeCombo = new QComboBox( false/*, panelFrame*/ );
        panelLayout->addWidget( d->typeCombo, 0, 1 );
        d->typeCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
        d->typeCombo->setMinimumWidth( 150 );
        d->typeCombo->setEnabled( false );
        connect( d->typeCombo, SIGNAL( activated(int) ),
                 this, SLOT( slotLoadProviderDXS(int) ) );

        QLabel * label2 = new QLabel( i18n("Order by:")/*, panelFrame*/ );
        panelLayout->addWidget( label2, 0, 2 );
        d->sortCombo = new QComboBox( false/*, panelFrame*/ );
        panelLayout->addWidget( d->sortCombo, 0, 3 );
        d->sortCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
        d->sortCombo->setMinimumWidth( 100 );
        d->sortCombo->setEnabled( false );
        d->sortCombo->addItem( SmallIcon( "text" ), i18n("Name") );
        d->sortCombo->addItem( SmallIcon( "favorites" ), i18n("Rating") );
        d->sortCombo->addItem( SmallIcon( "calendar-today" ), i18n("Most recent") );
        d->sortCombo->addItem( SmallIcon( "cdcopy" ), i18n("Most downloads") );
        connect( d->sortCombo, SIGNAL( activated(int) ),
                 this, SLOT( slotSortingSelected(int) ) );

        QLabel * label3 = new QLabel( i18n("Search:"), panelFrame );
        panelLayout->addWidget( label3, 1, 0 );
        d->searchLine = new QLineEdit( panelFrame );
        panelLayout->addWidget( d->searchLine, 1, 1, 1, 3 );
        d->searchLine->setEnabled( false );

      // create the ItemsView used to display available items
      d->itemsView = new ItemsView( this, this ); // FIXME KDE4PORT

    rightLayouter->addWidget(mainLabel);
    rightLayouter->addWidget(panelFrame);
    rightLayouter->addWidget(d->itemsView);

      // create bottom buttons
      QHBoxLayout * bottomLine = new QHBoxLayout();
      // FIXME KDE4PORT
      //QWidget * bottomLineWidget = new QWidget();
      //bottomLineWidget->setLayout( bottomLine );
      rightLayouter->addLayout(bottomLine);
        // create info label
        d->messageLabel = new QLabel();
        d->messageLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
        d->messageLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
        // close button
        QPushButton * closeButton = new QPushButton( i18n("Close") );
        //closeButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum )
        connect( closeButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
      bottomLine->addWidget(d->messageLabel);
      bottomLine->addWidget(closeButton);

    // start with a nice size
    resize( 700, 400 );
    slotResetMessageColors();

    m_engine = NULL;

    setWindowTitle(i18n("Get Hot New Stuff!"));
}

DownloadDialog::~DownloadDialog()
{
    // delete the private storage structure
    delete d;
}

void DownloadDialog::displayMessage( const QString & msg, MessageType type, int timeOutMs )
{
    // stop the pending timer if present
    if ( d->messageTimer )
        d->messageTimer->stop();

    // set background color based on message type
    switch ( type )
    {
        case Info:
	    // FIXME KDE4PORT
            //d->messageLabel->setPaletteForegroundColor( palette().active().highlightedText() );
            //d->messageLabel->setPaletteBackgroundColor( palette().active().highlight() );
            break;
        case Error:
	    // FIXME KDE4PORT
            //d->messageLabel->setPaletteForegroundColor( Qt::white );
            //d->messageLabel->setPaletteBackgroundColor( Qt::red );
            break;
        default:
            slotResetMessageColors();
            break;
    }

    // set text to messageLabel
    d->messageLabel->setText( msg );

    // single shot the resetColors timer (and create it if null)
    d->messageTimer->start( timeOutMs );
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

void DownloadDialog::slotResetMessageColors() // SLOT
{
    // FIXME KDE4PORT
    //d->messageLabel->setPaletteForegroundColor( palette().active().text() );
    //d->messageLabel->setPaletteBackgroundColor( palette().active().background() );
}

void DownloadDialog::slotNetworkTimeout() // SLOT
{
    displayMessage( i18n("Timeout. Check internet connection!"), Error );
}

void DownloadDialog::slotSortingSelected( int sortType ) // SLOT
{
    d->itemsView->setSorting( sortType );
}


///////////////// DXS ////////////////////

void DownloadDialog::setEngine(DxsEngine *engine)
{
	m_engine = engine;

	d->itemsView->setEngine(m_engine);

	// FIXME: adapt to DxsEngine from Dxs!
	connect(m_engine,
		SIGNAL(signalCategories(QList<KNS::Category*>)),
		SLOT(slotCategories(QList<KNS::Category*>)));
	connect(m_engine,
		SIGNAL(signalEntries(QList<KNS::Entry*>)),
		SLOT(slotEntries(QList<KNS::Entry*>)));
	connect(m_engine,
		SIGNAL(signalError()),
		SLOT(slotError()));
	connect(m_engine,
		SIGNAL(signalFault()),
		SLOT(slotFault()));
}

void DownloadDialog::slotLoadProviderDXS(int index)
{
	Q_UNUSED(index);

	QString category = d->typeCombo->currentText();
	QString categoryname = m_categorymap[category];

	//m_dxs->call_entries(categoryname, QString());
	// FIXME: use m_engine
}

void DownloadDialog::slotLoadProvidersListDXS()
{
}

void DownloadDialog::slotCategories(QList<KNS::Category*> categories)
{
	m_categorymap.clear();

	for(QList<KNS::Category*>::Iterator it = categories.begin(); it != categories.end(); it++)
	{
		KNS::Category *category = (*it);
		kDebug() << "Category: " << category->name().representation() << endl;
		QPixmap icon = DesktopIcon(category->icon().url(), 16);
		// FIXME: use icon from remote URLs (see non-DXS providers as well)
		d->typeCombo->addItem(icon, category->name().representation());
		m_categorymap[category->name().representation()] = category->id();
		// FIXME: better use global id, since names are not guaranteed
		//        to be unique
	}

	d->typeCombo->setEnabled(true);

	slotLoadProviderDXS(0);
}

void DownloadDialog::slotEntries(QList<KNS::Entry*> entries)
{
	Q_UNUSED(entries);

	//d->itemsView->setItems( entries );
	// FIXME: old API here
}

// FIXME: below here, those are for traditional GHNS

void DownloadDialog::addEntry(Entry *entry, const Feed *feed, const Provider *provider)
{
	Q_UNUSED(provider);

	Entry::List entries = m_entries[feed];
	entries.append(entry);
	m_entries[feed] = entries;

	kDebug() << "downloaddialog: addEntry to list of size " << entries.size() << endl;
}

void DownloadDialog::refresh()
{
	d->itemsView->setItems(m_entries);

	for(int i = 0; i < m_entries.keys().count(); i++)
	{
		const Feed *feed = m_entries.keys().at(i);
		if(!feed)
		{
			kDebug() << "INVALID FEED?!" << endl;
			continue;
		}
		//QPixmap icon = DesktopIcon(QString(), 16);
		//d->typeCombo->addItem(icon, feed->name().representation());
		d->typeCombo->addItem(feed->name().representation());
		// FIXME: see DXS categories
	}
	d->typeCombo->setEnabled(true);
	d->sortCombo->setEnabled(true);
	// FIXME: sort combo must be filled with feeds instead of being prefilled
}

///////////////// DXS ////////////////////

//BEGIN File(s) Transferring
void DownloadDialog::slotDownloadItem( Entry *entry )
{
Q_UNUSED(entry);
/*
XXX update item status
    item->setState( AvailableItem::Installing );
    item->setProgress( 0.0 );
    d->itemsView->updateItem( item );

XXX inform the user
    displayMessage( i18n("Installing '%1', this could take some time ...").arg( item->name().representation() ) );
*/
}

/*void DownloadDialog::slotItemMessage( KJob * job, const QString & message )
{
    AvailableItem * item = d->transferJobs[ job ].item;
    kDebug() << "Name: " << item->name().representation() << " msg: '" << message << "'." << endl;
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
