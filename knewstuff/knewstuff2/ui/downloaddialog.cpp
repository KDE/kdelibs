/*
    This file is part of KNewStuff2.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// own include
#include "downloaddialog.h"

// qt/kde includes
#include <QtCore/QTimer>
#include <QtGui/QPixmap>
#include <QtGui/QSortFilterProxyModel>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

#include <kdebug.h>

#include "knewstuff2/core/provider.h"
#include "knewstuff2/core/providerhandler.h"
#include "knewstuff2/core/entry.h"
#include "knewstuff2/core/entryhandler.h"
#include "knewstuff2/core/category.h"

#include "knewstuff2/dxs/dxs.h"

// local includes
#include "ui_DownloadDialog.h"
#include "itemsmodel.h"
#include "itemsviewdelegate.h"
#include "kdxsrating.h"
#include "kdxscomment.h"
#include "kdxscomments.h"

const char * ConfigGroup = "DownloadDialog Settings";

using namespace KNS;

DownloadDialog::DownloadDialog(DxsEngine* _engine, QWidget * _parent)
        : KDialog(_parent)
{
    setButtons(0);

    m_engine = _engine;
    connect(m_engine, SIGNAL(signalProgress(QString, int)), SLOT(slotProgress(QString, int)));
    connect(m_engine, SIGNAL(signalEntryChanged(KNS::Entry*)), SLOT(slotEntryChanged(KNS::Entry*)));
    connect(m_engine, SIGNAL(signalPayloadFailed(KNS::Entry*)), SLOT(slotPayloadFailed(KNS::Entry*)));
    connect(m_engine, SIGNAL(signalPayloadLoaded(KUrl)), SLOT(slotPayloadLoaded(KUrl)));
    connect(m_engine, SIGNAL(signalProvidersFailed()), SLOT(slotProvidersFailed()));
    connect(m_engine, SIGNAL(signalEntriesFailed()), SLOT(slotEntriesFailed()));

    connect(m_engine, SIGNAL(signalEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)),
            this, SLOT(slotEntryLoaded(KNS::Entry*, const KNS::Feed*, const KNS::Provider*)));
    connect(m_engine, SIGNAL(signalEntryRemoved(KNS::Entry*, const KNS::Feed*)),
            this, SLOT(slotEntryRemoved(KNS::Entry *, const KNS::Feed *)));

    // initialize the private classes
    messageTimer = new QTimer(this);
    messageTimer->setSingleShot(true);
    connect(messageTimer, SIGNAL(timeout()), SLOT(slotResetMessage()));

    networkTimer = new QTimer(this);
    connect(networkTimer, SIGNAL(timeout()), SLOT(slotNetworkTimeout()));

    m_searchTimer = new QTimer(this);
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(1000);   // timeout after 30 seconds
    connect(m_searchTimer, SIGNAL(timeout()), SLOT(slotUpdateSearch()));

    // popuplate dialog with stuff
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    setupUi(_mainWidget);

    // create the delegate
    mDelegate = new ItemsViewDelegate(m_listView, this);
    m_listView->setItemDelegate(mDelegate);
    connect(mDelegate, SIGNAL(performAction(DownloadDialog::EntryAction, KNS::Entry *)),
            SLOT(slotPerformAction(DownloadDialog::EntryAction, KNS::Entry *)));

    // create the filter model
    m_filteredModel = new QSortFilterProxyModel(this);
    m_filteredModel->setFilterRole(ItemsModel::kNameRole);
    m_filteredModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_listView->setModel(m_filteredModel);
    connect(m_listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotListIndexChanged(const QModelIndex &, const QModelIndex &)));

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "knewstuff/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way


    connect(m_sourceCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotLoadProviderDXS()));
    connect(m_sortCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSortingSelected(int)));
    connect(m_searchEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSearchTextChanged()));
    connect(m_searchEdit, SIGNAL(editingFinished()), SLOT(slotUpdateSearch()));

    // FIXME: not sure if this is better, or setting openExternalLinks
    //connect( m_providerLinkLabel, SIGNAL( linkActivated(const QString &)),
    //        KToolInvocation::self(), SLOT(invokeBrowser(const QString &)));

    // load the last size from config
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
    m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                 "%1 Add-On Installer",
                                 KGlobal::activeComponent().aboutData()->programName()));
    m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(accept()));

    KMenu * collabMenu = new KMenu(m_collaborationButton);
    QAction * action_collabrating = collabMenu->addAction(i18n("Add Rating"));
    action_collabrating->setData(DownloadDialog::kCollabRate);

    QAction * action_collabcomment = collabMenu->addAction(i18n("Add Comment"));
    action_collabcomment->setData(DownloadDialog::kCollabComment);

    QAction * action_comment = collabMenu->addAction(SmallIcon("help-about"), i18n("View Comments"));
    action_comment->setData(DownloadDialog::kComments);

/* TODO: Re-enable when implemented
    QAction * action_collabtranslation = collabMenu->addAction(i18n("Translate"));
    action_collabtranslation->setData(DownloadDialog::kCollabTranslate);

    QAction * action_collabsubscribe = collabMenu->addAction(i18n("Subscribe"));
    action_collabsubscribe->setData(DownloadDialog::kCollabSubscribe);

    QAction * action_collabremoval = collabMenu->addAction(i18n("Report bad entry"));
    action_collabremoval->setData(DownloadDialog::kCollabRemoval);
*/

    m_collaborationButton->setMenu(collabMenu);
    connect(m_collaborationButton, SIGNAL(triggered(QAction*)), this, SLOT(slotCollabAction(QAction*)));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
}

void DownloadDialog::slotPerformAction(DownloadDialog::EntryAction action, KNS::Entry * entry)
{
    kDebug(551) << "perform action: " << action;
    const Provider * provider = m_providers.contains(entry) ? m_providers[entry] : NULL;
    Dxs * dxs = m_engine->dxsObject(provider);
    switch (action) {
    case kViewInfo:
        if (provider && dxs) {
            if (provider->webService().isValid()) {
                dxs->call_info();
            } else {
                slotInfo(provider->name().representation(),
                         provider->webAccess().pathOrUrl(),
                         QString());
            }
        }
        break;
    case kComments:
        // show the entry's comments
        if (provider && dxs) {
            connect(dxs, SIGNAL(signalComments(QStringList)), this, SLOT(slotComments(QStringList)));
            dxs->call_comments(entry->idNumber());
        }
        break;
    case kChanges:
        // show the entry's changelog
        break;
    case kContactEmail:
        // invoke mail with the address of the author
        KToolInvocation::invokeMailer(entry->author().email(), i18n("Re: %1", entry->name().representation()));
        break;
    case kContactJabber:
        // start jabber with author's info
        break;
    case kCollabTranslate:
        // open translation dialog
        break;
    case kCollabRemoval:
        // verify removal, maybe authenticate?
        break;
    case kCollabSubscribe:
        // subscribe to changes
        break;
    case kUninstall:
        // uninstall
        setCursor(Qt::WaitCursor);
        m_engine->uninstall(entry);
        setCursor(Qt::ArrowCursor);
        break;
    case kInstall:
        // install
        setCursor(Qt::WaitCursor);
        m_engine->downloadPayload(entry);
        break;
    case kCollabComment: {
        // open comment dialog
        QPointer<KDXSComment> commentDialog = new KDXSComment(this);
        int ret = commentDialog->exec();
        if (ret == QDialog::Accepted) {
            QString s = commentDialog->comment();
            if (dxs && !s.isEmpty()) {
                dxs->call_comment(entry->idNumber(), s);
            }
        }
    }
    break;
    case kCollabRate: {
        // prompt for rating, and send to provider
        QPointer<KDXSRating> ratingDialog = new KDXSRating(this);
        int ret = ratingDialog->exec();
        if (ret == QDialog::Accepted) {
            int rating = ratingDialog->rating();
            if (dxs) {
                dxs->call_rating(entry->idNumber(), rating);
            }
        }
    }
    break;
    }
}

void DownloadDialog::slotCollabAction(QAction * action)
{
    DownloadDialog::EntryAction entryAction = (DownloadDialog::EntryAction)action->data().toInt();
    QModelIndex currentIndex = m_listView->currentIndex();
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(m_filteredModel->sourceModel());
    QModelIndex index = m_filteredModel->mapToSource(currentIndex);
    KNS::Entry * entry = realmodel->entryForIndex(index);
    slotPerformAction(entryAction, entry);
}

void DownloadDialog::slotListIndexChanged(const QModelIndex &index, const QModelIndex &/*old */)
{
    //kDebug() << "slotListIndexChanged called";

    m_collaborationButton->setEnabled(m_hasDxs && index.isValid());
}

void DownloadDialog::hideEvent(QHideEvent * event)
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    KDialog::hideEvent(event);
}

void DownloadDialog::displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs)
{
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    m_titleWidget->setComment(msg, type);

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        //kDebug(551) << "starting the message timer for " << timeOutMs;
        messageTimer->start(timeOutMs);
    }
}

void DownloadDialog::installItem(Entry *entry)
{
    // safety check
//    if ( item->url().isEmpty() || item->destinationPath().isEmpty() )
//    {
//        displayMessage( i18n("I don't know how to install this. Sorry, my fault."), Info );
//        return;
//    }

    //TODO check for AvailableItem deletion! (avoid broken pointers) -> cancel old jobs
    slotEntryChanged(entry);
}

void DownloadDialog::removeItem(Entry *entry)
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
    displayMessage(i18n("Timeout. Check Internet connection."), KTitleWidget::ErrorMessage);
}

void DownloadDialog::slotSortingSelected(int sortType)   // SLOT
{
    if (sortType >= 0) {
        //kDebug(551) << "sorting Selected, setting the sourcemodel for the view";
        QString feedName = m_sortCombo->currentText();
        QString feedType = m_sortCombo->itemData(sortType).toString();

        const Provider * currentProvider = m_entriesByProvider.keys()[m_sourceCombo->currentIndex()];
        Feed * selectedFeed = currentProvider->downloadUrlFeed(feedType);
        m_filteredModel->setSourceModel(m_models.value(selectedFeed));
        m_collaborationButton->setEnabled(false);
    }
}


///////////////// DXS ////////////////////

void DownloadDialog::slotLoadProviderDXS()
{
    kDebug(551) << "slotLoadProviderDXS called";
    //QString category = m_sourceCombo->currentText();
    //QString categoryname = categorymap[category];
    QString providerName = m_sourceCombo->currentText();

    QList<const Provider*> providers = m_entriesByProvider.keys();
    const Provider * provider = 0;

    for (int i = 0; i < providers.size(); ++i) {
        if (providers[i]->name().representation() == providerName) {
            provider = providers[i];
            // update the sortCombo with this provider's feeds
            populateSortCombo(providers[i]);

            Feed * selectedFeed = providers[i]->downloadUrlFeed(m_sortCombo->itemData(m_sortCombo->currentIndex()).toString());
            m_filteredModel->setSourceModel(m_models.value(selectedFeed));
            //m_list->setProvider(providers[i],
            //    providers[i]->downloadUrlFeed(m_sortCombo->itemData(m_sortCombo->currentIndex()).toString()));
            break;
        }
    }
    m_hasDxs = (provider && m_engine->dxsObject(provider) != NULL);
    m_collaborationButton->setEnabled(m_hasDxs);
}

void DownloadDialog::slotUpdateSearch()
{
    m_searchTimer->stop();
    m_filteredModel->setFilterFixedString(m_searchEdit->text());
    m_filteredModel->invalidate();
}

void DownloadDialog::slotLoadProvidersListDXS()
{
}

void DownloadDialog::slotSearchTextChanged()
{
    m_searchTimer->start();
}

void DownloadDialog::slotCategories(QList<KNS::Category*> categories)
{
    categorymap.clear();

    for (QList<KNS::Category*>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        KNS::Category *category = (*it);
        //kDebug(551) << "Category: " << category->name().representation();
        QPixmap icon = DesktopIcon(category->icon().url(), 16);
        // FIXME: use icon from remote URLs (see non-DXS providers as well)
        m_sourceCombo->addItem(icon, category->name().representation());
        categorymap[category->name().representation()] = category->id();
        // FIXME: better use global id, since names are not guaranteed
        //        to be unique
    }

    //m_sourceCombo->setEnabled(true);

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

    if (!m_entriesByProvider.contains(provider)) {
        kDebug(551) << "adding provider " << provider->name().representation() << " to combobox";
        m_sourceCombo->addItem(provider->name().representation());
    }
    m_entriesByProvider[provider].append(entry);

    // FIXME: what if entry belongs to more than one provider at once?
    m_providers[entry] = provider;

    mMutex.lock();

    if (!m_models.value(feed)) {
        // new feed
        kDebug(551) << "making a new model for this feed" << feed;
        m_models[feed] = new KNS::ItemsModel(this, provider->webService().isValid());
        connect(m_engine, SIGNAL(signalEntryChanged(KNS::Entry*)),
                m_models[feed], SLOT(slotEntryChanged(KNS::Entry*)));
        if (provider->name().representation() == m_sourceCombo->currentText()) {
            // this provider is selected, so refresh the feed combobox
            populateSortCombo(provider);
        }
    }
    mMutex.unlock();

    KNS::ItemsModel* thisModel = m_models.value(feed);

    Q_ASSERT(thisModel != NULL);
    thisModel->addEntry(entry);
}

void DownloadDialog::slotEntryRemoved(KNS::Entry *entry, const KNS::Feed *feed)
{
    Q_ASSERT(m_models[feed] != NULL);

    m_models[feed]->removeEntry(entry);
}

void DownloadDialog::refresh()
{
    m_sourceCombo->clear();

    Q_ASSERT(m_entriesByProvider.keys().size() > 0);

    for (int i = 0; i < m_entriesByProvider.keys().count(); i++) {
        const Provider *provider = m_entriesByProvider.keys().at(i);
        if (!provider) {
            //kDebug(551) << "INVALID FEED?!";
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

    //m_sourceCombo->setEnabled(true);
    //m_sortCombo->setEnabled(true);
    //m_searchEdit->setEnabled(true);
}

void DownloadDialog::populateSortCombo(const Provider * provider)
{
    QString url = provider->webAccess().pathOrUrl();
    if (url.isEmpty()) {
        m_providerLinkLabel->hide();
    } else {
        m_providerLinkLabel->setText(QString("<a href=\"%1\">?</a>").arg(url));
    }

    QStringList feeds = provider->feeds();
    m_sortCombo->clear();
    for (int i = 0; i < feeds.size(); ++i) {
        QString feedName = provider->downloadUrlFeed(feeds[i])->name().representation();
        kDebug(551) << "adding feed " << feeds[i] << " to combobox";
        m_sortCombo->addItem(feedName, feeds[i]); // put in the name for the text, and feeds[i] for the userData
    }
}

void DownloadDialog::slotInfo(QString provider, QString server, QString version)
{
    QString link = QString("<a href=\"%1\">%1</a>").arg(server);
    QString infostring = i18n("Server: %1", link);
    infostring += i18n("<br />Provider: %1", provider);
    infostring += i18n("<br />Version: %1", version);

    KMessageBox::information(this,
                             infostring,
                             i18n("Provider information"));
}

void DownloadDialog::slotComments(QStringList comments)
{
    QPointer<KDXSComments> commentsdlg = new KDXSComments(this);

    for (QStringList::const_iterator it = comments.constBegin(); it != comments.constEnd(); ++it) {
        //kDebug() << "Comment: " << (*it);
        commentsdlg->addComment("foo", (*it));
    }

    commentsdlg->exec();
}

///////////////// DXS ////////////////////

void DownloadDialog::slotEntryChanged(KNS::Entry * entry)
{
    Q_UNUSED(entry)
    setCursor(Qt::ArrowCursor);
}

void DownloadDialog::slotPayloadFailed(KNS::Entry * entry)
{
    setCursor(Qt::ArrowCursor);
    KMessageBox::error(this, i18n("Could not install %1", entry->name().representation()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadDialog::slotPayloadLoaded(KUrl url)
{
    Q_UNUSED(url)
    setCursor(Qt::ArrowCursor);
}

void DownloadDialog::slotProgress(const QString & text, int percentage)
{
    m_progress->addProgress(text, percentage);
}

void DownloadDialog::slotProvidersFailed()
{
    kDebug(551) << "slotProvidersFailed";
    KMessageBox::error(this,
                       i18n("There was an error loading data providers."),
                       i18n("Get Hot New Stuff"));
}

/*void DownloadDialog::slotItemMessage( KJob * job, const QString & message )
{
    AvailableItem * item = d->transferJobs[ job ].item;
    kDebug(551) << "Name: " << item->name().representation() << " msg: '" << message << "'.";
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
