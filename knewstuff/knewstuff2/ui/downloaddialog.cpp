/***************************************************************************
 *   Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>                  *
 *   Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>           *
 *             (C) 2007 Dirk Mueller <mueller@kde.org>                     *
 *   Copyright (C) 2007 Jeremy Whiting <jeremy@scitools.com>               *
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
#include <QtCore/QTimer>
#include <QtGui/QPixmap>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "knewstuff2/core/provider.h"
#include "knewstuff2/core/providerhandler.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"

#include "knewstuff2/dxs/dxs.h"

// local includes
#include "ui_DownloadDialog.h"

const char * ConfigGroup = "DownloadDialog Settings";

using namespace KNS;

DownloadDialog::DownloadDialog( DxsEngine* _engine, QWidget * _parent )
    : KDialog( _parent )
{
    setButtons(0);

    m_engine = _engine;

    // initialize the private classes
    messageTimer = new QTimer( this );
    messageTimer->setSingleShot( true );
    connect( messageTimer, SIGNAL( timeout() ), SLOT( slotResetMessage() ) );

    networkTimer = new QTimer( this );
    connect( networkTimer, SIGNAL( timeout() ), SLOT( slotNetworkTimeout() ) );

    searchTimer = new QTimer( this );
    searchTimer->setSingleShot( true );
    searchTimer->setInterval( 1000 ); // timeout after 30 seconds
    connect( searchTimer, SIGNAL( timeout() ), SLOT( slotUpdateSearch() ) );

    // popuplate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    setupUi(_mainWidget);

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "knewstuff/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way


    connect( m_sourceCombo, SIGNAL( activated(int) ), SLOT( slotLoadProviderDXS() ) );
    connect( m_sortCombo, SIGNAL( activated(int) ), SLOT( slotSortingSelected(int) ) );
    m_searchEdit->setClearButtonShown(true);
    connect( m_searchEdit, SIGNAL( textChanged( const QString &) ), SLOT( slotSearchTextChanged() ));
    connect( m_searchEdit, SIGNAL( editingFinished() ), SLOT( slotUpdateSearch() ));

    m_list->setEngine(m_engine);

    // load the last size from config
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
    m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                  "%1 Add-On Installer",
                                  KGlobal::activeComponent().aboutData()->programName()));
    m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    connect( m_buttonBox, SIGNAL( rejected() ), this, SLOT( accept() ) );
    connect( m_engine, SIGNAL( signalProgress( QString, int ) ), SLOT( slotProgress( QString, int ) ) );
    connect( m_engine, SIGNAL( signalEntryChanged( KNS::Entry* ) ), SLOT( slotEntryChanged( KNS::Entry* ) ));

    connect( m_engine, SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
            this, SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
    connect( m_engine, SIGNAL(signalEntriesFailed()),
            this, SLOT(slotEntriesFailed()));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
}

void DownloadDialog::hideEvent(QHideEvent * event)
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    KDialog::hideEvent(event);
}

void DownloadDialog::displayMessage( const QString & msg, KTitleWidget::MessageType type, int timeOutMs )
{
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    m_titleWidget->setComment( msg, type );

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
    slotEntryChanged( entry );
}

void DownloadDialog::removeItem( Entry *entry )
{
    Q_UNUSED(entry);
//    displayMessage( i18n("%1 is no more installed.").arg( item->name().representation() ) );
}

void DownloadDialog::slotResetMessage() // SLOT
{
    m_titleWidget->setComment(QString());
}

void DownloadDialog::slotNetworkTimeout() // SLOT
{
    displayMessage( i18n("Timeout. Check Internet connection!"), KTitleWidget::ErrorMessage );
}

void DownloadDialog::slotSortingSelected( int sortType ) // SLOT
{
    QString feedName = m_sortCombo->currentText();
    QString feedType = m_sortCombo->itemData(sortType).toString();

    const Provider * currentProvider = m_entriesByProvider.keys()[m_sourceCombo->currentIndex()];
    m_list->setFeed(currentProvider->downloadUrlFeed(feedType));
}


///////////////// DXS ////////////////////

void DownloadDialog::slotLoadProviderDXS()
{
    //QString category = m_sourceCombo->currentText();
    //QString categoryname = categorymap[category];
    QString providerName = m_sourceCombo->currentText();

    QList<const Provider*> providers = m_entriesByProvider.keys();

    for (int i = 0; i < providers.size(); ++i) {
        if (providers[i]->name().representation() == providerName) {
            // update the sortCombo with this provider's feeds
            populateSortCombo(providers[i]);

            m_list->setProvider(providers[i], 
                providers[i]->downloadUrlFeed(m_sortCombo->itemData(m_sortCombo->currentIndex()).toString()));
            break;
        }
    }
}

void DownloadDialog::slotUpdateSearch()
{
    if (searchTimer->isActive()) {
        searchTimer->stop();
        m_list->setSearchText(m_searchEdit->text());
    }
}

void DownloadDialog::slotLoadProvidersListDXS()
{
}

void DownloadDialog::slotSearchTextChanged()
{
    searchTimer->start();
}

void DownloadDialog::slotCategories(QList<KNS::Category*> categories)
{
    categorymap.clear();

    for(QList<KNS::Category*>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        KNS::Category *category = (*it);
        //kDebug() << "Category: " << category->name().representation();
        QPixmap icon = DesktopIcon(category->icon().url(), 16);
        // FIXME: use icon from remote URLs (see non-DXS providers as well)
        m_sourceCombo->addItem(icon, category->name().representation());
        categorymap[category->name().representation()] = category->id();
        // FIXME: better use global id, since names are not guaranteed
        //        to be unique
    }

    m_sourceCombo->setEnabled(true);

    slotLoadProviderDXS();
}

void DownloadDialog::slotEntries(QList<KNS::Entry*> _entries)
{
    Q_UNUSED(_entries);

    //d->itemsView->setItems( entries );
    // FIXME: old API here
}

void DownloadDialog::slotEntriesFailed()
{
    displayMessage(i18n("Entries failed to load"));
}
// FIXME: below here, those are for traditional GHNS

void DownloadDialog::slotEntryLoaded(Entry *entry, const Feed *feed, const Provider *provider)
{
    Entry::List e = entries[feed];
    e.append(entry);
    entries[feed] = e;

    m_entriesByProvider[provider].append(entry);

    // FIXME: what if entry belongs to more than one provider at once?
    providers[entry] = provider;

    //kDebug() << "downloaddialog: addEntry to list of size " << entries.size();
}

void DownloadDialog::refresh()
{
    Q_ASSERT(m_entriesByProvider.keys().size() > 0);
    for (int i = 0; i < m_entriesByProvider.keys().count(); i++) {
        const Provider *provider = m_entriesByProvider.keys().at(i);
        if(!provider) {
            //kDebug() << "INVALID FEED?!";
            continue;
        }
        //QPixmap icon = DesktopIcon(QString(), 16);
        //d->m_typeCombo->addItem(icon, feed->name().representation());
        m_sourceCombo->addItem(provider->name().representation());
        // FIXME: see DXS categories
    }

    slotLoadProviderDXS();
    
    //// get the current provider
    //const Provider * selectedProvider = m_entriesByProvider.keys()[0];

    //populateSortCombo(selectedProvider);
    
    m_sourceCombo->setEnabled(true);
    m_sortCombo->setEnabled(true);
    m_searchEdit->setEnabled(true);
}

void DownloadDialog::populateSortCombo(const Provider * provider)
{
    QStringList feeds = provider->feeds();
    m_sortCombo->clear();
    for (int i = 0; i < feeds.size(); ++i) {
        QString feedName = provider->downloadUrlFeed(feeds[i])->name().representation();
        m_sortCombo->addItem(feedName, feeds[i]); // put in the name for the text, and feeds[i] for the userData
    }
}

///////////////// DXS ////////////////////

void DownloadDialog::slotEntryChanged( KNS::Entry * entry )
{
    m_list->updateItem(entry);
}

void DownloadDialog::slotProgress(const QString & text, int percentage)
{
    m_progress->addProgress(text, percentage);
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
