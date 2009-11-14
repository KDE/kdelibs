/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jeremy@scitools.com>

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
#include <QtGui/QScrollBar>

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

#include <kdebug.h>

#include "knewstuff3/core/category.h"

// local includes
#include "ui_DownloadDialog.h"

#include "itemsmodel.h"
#include "itemsviewdelegate.h"
#include "kdxsrating.h"

const char * ConfigGroup = "DownloadDialog Settings";

using namespace KNS3;

class DownloadDialog::Private {
public:
    QTimer* messageTimer;

    Engine *engine;
    QMap<QString, QString> categorymap;

    KNS3::ItemsModel* model;
    // sort items according to sort combo
    QSortFilterProxyModel * sortingProxyModel;
    ItemsViewDelegate * mDelegate;
    
    QString searchTerm;
    
    Private(Engine* _engine)
        : engine(_engine), model(new ItemsModel), sortingProxyModel(new QSortFilterProxyModel)
        , messageTimer(new QTimer)
    {
        messageTimer->setSingleShot(true);

        sortingProxyModel->setFilterRole(ItemsModel::kNameRole);
        sortingProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        sortingProxyModel->setSourceModel(model);
    }
    
    ~Private() {
        delete messageTimer;
        delete mDelegate;
        delete sortingProxyModel;
        delete model;
    }
};

DownloadDialog::DownloadDialog(Engine* engine, QWidget * parent)
        : KDialog(parent)
        , d(new Private(engine))
{
    setButtons(KDialog::None);
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    setupUi(_mainWidget);

    closeButton->setGuiItem(KStandardGuiItem::Close);
    connect(closeButton, SIGNAL(clicked()), SLOT(accept()));
    
    // Entries have been fetched and should be shown:
    connect(d->engine, SIGNAL(signalEntriesLoaded(KNS3::Entry::List)), d->model, SLOT(slotEntriesLoaded(KNS3::Entry::List)));
    
    connect(d->engine, SIGNAL(signalError(const QString&)), SLOT(slotError(const QString&)));

    // An entry has changes - eg because it was installed
    connect(d->engine, SIGNAL(signalEntryChanged(KNS3::Entry)), SLOT(slotEntryChanged(KNS3::Entry)));

    connect(d->engine, SIGNAL(signalResetView()), d->model, SLOT(clearEntries()));
    
    // FIXME show download progress
    connect(d->engine, SIGNAL(signalProgress(QString, int)), SLOT(slotProgress(QString, int)));
    
    connect(d->messageTimer, SIGNAL(timeout()), SLOT(slotResetMessage()));

    d->mDelegate = new ItemsViewDelegate(m_listView);
    m_listView->setItemDelegate(d->mDelegate);
    connect(d->mDelegate, SIGNAL(performAction(DownloadDialog::EntryAction, const KNS3::Entry&)),
            SLOT(slotPerformAction(DownloadDialog::EntryAction, const KNS3::Entry&)));

    m_listView->setModel(d->sortingProxyModel);
    connect(m_listView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotListIndexChanged(const QModelIndex &, const QModelIndex &)));

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "knewstuff/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way

    // FIXME set sorting options in m_sortCombo, make the sortFilterProxyModel use the sorting
    // maybe also clear the list of entries and ask providers to refetch them (either from cache or dynamically)
    m_sortCombo->insertItem(Provider::Rating, i18nc("Sorting order of the list of items in get hot new stuff", "Rating"));
    m_sortCombo->insertItem(Provider::Newest, i18nc("Sorting order of the list of items in get hot new stuff", "Newest"));
    m_sortCombo->insertItem(Provider::Downloads, i18nc("Sorting order of the list of items in get hot new stuff", "Most Downloads"));
    m_sortCombo->insertItem(Provider::Alphabetical, i18nc("Sorting order of the list of items in get hot new stuff", "Alphabetical"));
    m_sortCombo->insertItem(Provider::Installed, i18nc("Sorting order of the list of items in get hot new stuff", "Installed only"));
    
    connect(m_sortCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSortingSelected(int)));
    connect(m_searchEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSearchTextChanged()));
    connect(m_searchEdit, SIGNAL(editingFinished()), SLOT(slotUpdateSearch()));

    /*
    KMenu * collabMenu = new KMenu(m_collaborationButton);
    QAction * action_collabrating = collabMenu->addAction(i18n("Add Rating"));
    action_collabrating->setData(DownloadDialog::kCollabRate);

    QAction * action_collabcomment = collabMenu->addAction(i18n("Add Comment"));
    action_collabcomment->setData(DownloadDialog::kCollabComment);

    QAction * action_comment = collabMenu->addAction(SmallIcon("help-about"), i18n("View Comments"));
    action_comment->setData(DownloadDialog::kComments);
    */

/* TODO: Re-enable when implemented
    QAction * action_collabtranslation = collabMenu->addAction(i18n("Translate"));
    action_collabtranslation->setData(DownloadDialog::kCollabTranslate);

    QAction * action_collabsubscribe = collabMenu->addAction(i18n("Subscribe"));
    action_collabsubscribe->setData(DownloadDialog::kCollabSubscribe);

    QAction * action_collabremoval = collabMenu->addAction(i18n("Report bad entry"));
    action_collabremoval->setData(DownloadDialog::kCollabRemoval);
*/
/*
    m_collaborationButton->setMenu(collabMenu);
    connect(m_collaborationButton, SIGNAL(triggered(QAction*)), this, SLOT(slotCollabAction(QAction*)));
*/

    // load the last size from config
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
    m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                 "%1 Add-On Installer",
                                 KGlobal::activeComponent().aboutData()->programName()));
    m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    connect(m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollbar(int)));
    connect(this, SIGNAL(signalRequestMoreData()), d->engine, SLOT(slotRequestMoreData()));
    
    // FIXME connect(d->engine, SIGNAL(signalJobStarted(KJob*)), progressIndicator, SLOT(addJob(KJob*)));
    connect(d->model, SIGNAL(jobStarted(KJob*, const QString&)), progressIndicator, SLOT(addJob(KJob*, const QString&)));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    delete d;
}

void DownloadDialog::slotPerformAction(DownloadDialog::EntryAction action, Entry entry)
{
    kDebug(551) << "perform action: " << action;
    
    //Dxs * dxs = d->engine->dxsObject(provider);
    switch (action) {
    case kViewInfo:
        //if (provider && dxs) {
            //if (provider->webService().isValid()) {
            //    dxs->call_info();
            //} else {
                //slotInfo(provider->name().representation(),
                //         provider->webAccess().pathOrUrl(),
                //         QString());
            //}
        //}
        break;
    case kComments:
        // show the entry's comments
        //if (provider && dxs) {
        //    connect(dxs, SIGNAL(signalComments(QStringList)), this, SLOT(slotComments(QStringList)));
        //    dxs->call_comments(entry->idNumber());
        //}
        break;
    case kChanges:
        // show the entry's changelog
        break;
    case kContactEmail:
        // invoke mail with the address of the author
        KToolInvocation::invokeMailer(entry.author().email(), i18n("Re: %1", entry.name().representation()));
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
        setCursor(Qt::WaitCursor);
        d->engine->uninstall(entry);
        setCursor(Qt::ArrowCursor);
        break;
    case kInstall:
        setCursor(Qt::WaitCursor);
        d->engine->install(entry);
        setCursor(Qt::ArrowCursor);
        break;
    case kCollabComment: {
        // open comment dialog
        //QPointer<KDXSComment> commentDialog = new KDXSComment(this);
        //int ret = commentDialog->exec();
        //if (ret == QDialog::Accepted) {
        //    QString s = commentDialog->comment();
            //if (dxs && !s.isEmpty()) {
            //    dxs->call_comment(entry->idNumber(), s);
            //}
        //}
    }
    break;
    case kCollabRate: {
        // prompt for rating, and send to provider
        QPointer<KDXSRating> ratingDialog = new KDXSRating(this);
        int ret = ratingDialog->exec();
        if (ret == QDialog::Accepted) {
            int rating = ratingDialog->rating();
            //if (dxs) {
            //    dxs->call_rating(entry->idNumber(), rating);
            //}
        }
    }
    break;
    }
}

void DownloadDialog::slotCollabAction(QAction * action)
{
    DownloadDialog::EntryAction entryAction = (DownloadDialog::EntryAction)action->data().toInt();
    QModelIndex currentIndex = m_listView->currentIndex();
    QModelIndex index = d->sortingProxyModel->mapToSource(currentIndex);
    Entry entry = d->model->entryForIndex(index);
    slotPerformAction(entryAction, entry);
}

void DownloadDialog::slotListIndexChanged(const QModelIndex &index, const QModelIndex &/*old */)
{
    //kDebug() << "slotListIndexChanged called";
/*
    if (!index.isValid()) {
        m_collaborationButton->setEnabled(false);
    }
    
    Entry entry = d->model->entryForIndex(d->sortingProxyModel->mapToSource(index));
    m_collaborationButton->setEnabled(d->engine->collaborationFeatures(entry));
    */
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
    d->messageTimer->stop();

    // set text to messageLabel
    m_titleWidget->setComment(msg, type);

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        //kDebug(551) << "starting the message timer for " << timeOutMs;
        d->messageTimer->start(timeOutMs);
    }
}

void DownloadDialog::slotResetMessage() // SLOT
{
    m_titleWidget->setComment(QString());
}

void DownloadDialog::slotNetworkTimeout() // SLOT
{
    displayMessage(i18n("Timeout. Check Internet connection!"), KTitleWidget::ErrorMessage);
}

void DownloadDialog::slotSortingSelected(int sortType)   // SLOT
{
    d->model->clearEntries();
    if (sortType == Provider::Installed) {
        m_searchEdit->clear();
    }
    m_searchEdit->setEnabled(sortType != Provider::Installed);
    
    d->engine->setSortMode((Provider::SortMode)sortType);
    d->engine->reloadEntries();
}

void DownloadDialog::slotUpdateSearch()
{
    if (d->searchTerm == m_searchEdit->text().trimmed()) {
        return;
    }
    d->searchTerm = m_searchEdit->text().trimmed();
}

void DownloadDialog::slotSearchTextChanged()
{
    if (d->searchTerm == m_searchEdit->text().trimmed()) {
        return;
    }
    d->searchTerm = m_searchEdit->text().trimmed();
    d->engine->setSearchTerm(m_searchEdit->text().trimmed());
}

void DownloadDialog::slotCategories(QList<KNS3::Category*> categories)
{
    d->categorymap.clear();

    for (QList<KNS3::Category*>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        KNS3::Category *category = (*it);
        //kDebug(551) << "Category: " << category->name().representation();
        QPixmap icon = DesktopIcon(category->icon().url(), 16);
        // FIXME: use icon from remote URLs (see non-DXS providers as well)
        //d->sourceCombo->addItem(icon, category->name().representation());
        d->categorymap[category->name().representation()] = category->id();
        // FIXME: better use global id, since names are not guaranteed
        //        to be unique
    }

    //d->sourceCombo->setEnabled(true);
}


// FIXME: below here, those are for traditional GHNS


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
    //QPointer<KDXSComments> commentsdlg = new KDXSComments(this);

    //for (QStringList::const_iterator it = comments.constBegin(); it != comments.constEnd(); ++it) {
    //    //kDebug() << "Comment: " << (*it);
    //    commentsdlg->addComment("foo", (*it));
    //}

    //commentsdlg->exec();
}

///////////////// DXS ////////////////////

void DownloadDialog::slotEntryChanged(const Entry& entry)
{
    Q_UNUSED(entry)
    setCursor(Qt::ArrowCursor);
    d->model->slotEntryChanged(entry);
}

void DownloadDialog::slotPayloadFailed(const Entry& entry)
{
    setCursor(Qt::ArrowCursor);
    KMessageBox::error(this, i18n("Could not install %1", entry.name().representation()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadDialog::slotPayloadLoaded(KUrl url)
{
    Q_UNUSED(url)
    setCursor(Qt::ArrowCursor);
}

void DownloadDialog::slotProgress(const QString & text, int percentage)
{
//     m_progress->addProgress(text, percentage);
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

void DownloadDialog::slotError(const QString& message)
{
    KMessageBox::error(this, message, i18n("Get Hot New Stuff"));
}


void DownloadDialog::scrollbar(int value)
{
    if ((double)value/m_listView->verticalScrollBar()->maximum() > 0.7) {
        emit signalRequestMoreData();
    }
}


#include "downloaddialog.moc"
