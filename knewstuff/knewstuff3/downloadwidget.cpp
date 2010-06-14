/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    
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

#include "downloadwidget.h"
#include "downloadwidget_p.h"

#include <QtCore/QTimer>
#include <QtGui/QScrollBar>
#include <QtGui/QKeyEvent>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"
#include "ui/itemsgridviewdelegate.h"

using namespace KNS3;

DownloadWidget::DownloadWidget(QWidget* parent)
    : QWidget(parent)
    , d(new DownloadWidgetPrivate(this))
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

DownloadWidget::DownloadWidget(const QString& configFile, QWidget * parent)
        : QWidget(parent)
        , d(new DownloadWidgetPrivate(this))
{
    init(configFile);
}

void DownloadWidget::init(const QString& configFile)
{
    d->init(configFile);
}

DownloadWidget::~DownloadWidget()
{
    delete d;
}

Entry::List DownloadWidget::changedEntries()
{
    Entry::List entries;
    foreach (const EntryInternal &e, d->changedEntries) {
        entries.append(e.toEntry());
    }
    return entries;
}

Entry::List DownloadWidget::installedEntries()
{
    Entry::List entries;
    foreach (const EntryInternal &e, d->changedEntries) {
        if (e.status() == Entry::Installed) {
            entries.append(e.toEntry());
        }
    }
    return entries;
}


DownloadWidgetPrivate::DownloadWidgetPrivate(DownloadWidget* q)
: q(q)
, engine(new Engine)
, model(new ItemsModel(engine))
, messageTimer(0)
, dialogMode(false)
{
}

DownloadWidgetPrivate::~DownloadWidgetPrivate()
{
    delete messageTimer;
    delete delegate;
    delete model;
    delete engine;
}

void DownloadWidgetPrivate::slotResetMessage() // SLOT
{
    ui.m_titleWidget->setComment(QString());
}

void DownloadWidgetPrivate::slotNetworkTimeout() // SLOT
{
    displayMessage(i18n("Timeout. Check Internet connection."), KTitleWidget::ErrorMessage);
}

void DownloadWidgetPrivate::sortingChanged()
{
    Provider::SortMode sortMode = Provider::Newest;
    if (ui.ratingRadio->isChecked()) {
        sortMode = Provider::Rating;
    } else if (ui.mostDownloadsRadio->isChecked()) {
         sortMode = Provider::Downloads;
    } else if (ui.installedRadio->isChecked()) {
        sortMode = Provider::Installed;
    }

    model->clearEntries();
    if (sortMode == Provider::Installed) {
        ui.m_searchEdit->clear();
    }
    ui.m_searchEdit->setEnabled(sortMode != Provider::Installed);

    engine->setSortMode(sortMode);
}

void DownloadWidgetPrivate::slotUpdateSearch()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
}

void DownloadWidgetPrivate::slotSearchTextChanged()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
    engine->setSearchTerm(ui.m_searchEdit->text().trimmed());
}

void DownloadWidgetPrivate::slotCategoryChanged(int idx)
{
    if (idx == 0) {
        // All Categories item selected, reset filter
        engine->setCategoriesFilter(QStringList());

    } else {
        QString category = ui.m_categoryCombo->currentText();
        if (!category.isEmpty()) {
            QStringList filter(category);
            engine->setCategoriesFilter(filter);
        }
    }
}

void DownloadWidgetPrivate::slotInfo(QString provider, QString server, QString version)
{
    QString link = QString("<a href=\"%1\">%1</a>").arg(server);
    QString infostring = i18n("Server: %1", link);
    infostring += i18n("<br />Provider: %1", provider);
    infostring += i18n("<br />Version: %1", version);

    KMessageBox::information(0,
                             infostring,
                             i18n("Provider information"));
}

void DownloadWidgetPrivate::slotEntryChanged(const EntryInternal& entry)
{
    changedEntries.insert(entry);
    model->slotEntryChanged(entry);
}

void DownloadWidgetPrivate::slotPayloadFailed(const EntryInternal& entry)
{
    KMessageBox::error(0, i18n("Could not install %1", entry.name()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadWidgetPrivate::slotPayloadLoaded(KUrl url)
{
    Q_UNUSED(url)
}

void DownloadWidgetPrivate::slotError(const QString& message)
{
    KMessageBox::error(0, message, i18n("Get Hot New Stuff"));
}

void DownloadWidgetPrivate::scrollbarValueChanged(int value)
{
    if ((double)value/ui.m_listView->verticalScrollBar()->maximum() > 0.9) {
        engine->requestMoreData();
    }
}

void DownloadWidgetPrivate::init(const QString& configFile)
{
    m_configFile = configFile;
    ui.setupUi(q);
    ui.m_titleWidget->setVisible(false);
    ui.closeButton->setVisible(dialogMode);
    ui.backButton->setVisible(false);
    ui.backButton->setGuiItem(KStandardGuiItem::Back);
    q->connect(ui.backButton, SIGNAL(clicked()), q, SLOT(slotShowOverview()));
    
    q->connect(engine, SIGNAL(signalBusy(const QString&)), ui.progressIndicator, SLOT(busy(const QString&)));
    q->connect(engine, SIGNAL(signalError(const QString&)), ui.progressIndicator, SLOT(error(const QString&)));
    q->connect(engine, SIGNAL(signalIdle(const QString&)), ui.progressIndicator, SLOT(idle(const QString&)));

    q->connect(engine, SIGNAL(signalProvidersLoaded()), q, SLOT(slotProvidersLoaded()));
    // Entries have been fetched and should be shown:
    q->connect(engine, SIGNAL(signalEntriesLoaded(KNS3::EntryInternal::List)), q, SLOT(slotEntriesLoaded(KNS3::EntryInternal::List)));

    // An entry has changes - eg because it was installed
    q->connect(engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)), q, SLOT(slotEntryChanged(KNS3::EntryInternal)));

    q->connect(engine, SIGNAL(signalResetView()), model, SLOT(clearEntries()));
    q->connect(engine, SIGNAL(signalEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)),
               model, SLOT(slotEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)));

    engine->init(configFile);

    delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
    ui.m_listView->setItemDelegate(delegate);
    ui.m_listView->setModel(model);

    ui.iconViewButton->setIcon(KIcon("view-list-icons"));
    ui.iconViewButton->setToolTip(i18n("Icons view mode"));
    ui.listViewButton->setIcon(KIcon("view-list-details"));
    ui.listViewButton->setToolTip(i18n("Details view mode"));

    q->connect(ui.listViewButton, SIGNAL(clicked()), q, SLOT(slotListViewListMode()));
    q->connect(ui.iconViewButton, SIGNAL(clicked()), q, SLOT(slotListViewIconMode()));

    q->connect(ui.newestRadio,  SIGNAL(clicked()), q, SLOT(sortingChanged()));
    q->connect(ui.ratingRadio,  SIGNAL(clicked()), q, SLOT(sortingChanged()));
    q->connect(ui.mostDownloadsRadio,  SIGNAL(clicked()), q, SLOT(sortingChanged()));
    q->connect(ui.installedRadio,  SIGNAL(clicked()), q, SLOT(sortingChanged()));

    q->connect(ui.m_searchEdit, SIGNAL(textChanged(const QString &)), q, SLOT(slotSearchTextChanged()));
    q->connect(ui.m_searchEdit, SIGNAL(editingFinished()), q, SLOT(slotUpdateSearch()));

    ui.m_providerLabel->setVisible(false);
    ui.m_providerCombo->setVisible(false);
    ui.m_providerCombo->addItem(i18n("All Providers"));

    QStringList categories = engine->categories();
    if (categories.size() < 2) {
        ui.m_categoryLabel->setVisible(false);
        ui.m_categoryCombo->setVisible(false);
    } else {
        ui.m_categoryCombo->addItem(i18n("All Categories"));
        foreach(const QString& category, categories) {
            ui.m_categoryCombo->addItem(category);
        }
    }

    ui.detailsStack->widget(0)->layout()->setMargin(0);
    ui.detailsStack->widget(1)->layout()->setMargin(0);

    q->connect(ui.m_categoryCombo, SIGNAL(activated(int)), q, SLOT(slotCategoryChanged(int)));

    // let the search line edit trap the enter key, otherwise it closes the dialog
    ui.m_searchEdit->setTrapReturnKey(true);

    q->connect(ui.m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(scrollbarValueChanged(int)));
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));

    details = new EntryDetails(engine, &ui);
    q->connect(delegate, SIGNAL(signalShowDetails(KNS3::EntryInternal)), q, SLOT(slotShowDetails(KNS3::EntryInternal)));

    slotShowOverview();
}

void DownloadWidgetPrivate::slotListViewListMode()
{
    ui.listViewButton->setChecked(true);
    ui.iconViewButton->setChecked(false);
    setListViewMode(QListView::ListMode);
}

void DownloadWidgetPrivate::slotListViewIconMode()
{
    ui.listViewButton->setChecked(false);
    ui.iconViewButton->setChecked(true);
    setListViewMode(QListView::IconMode);
}

void DownloadWidgetPrivate::setListViewMode(QListView::ViewMode mode)
{
    if (ui.m_listView->viewMode() == mode) {
        return;
    }

    ItemsViewBaseDelegate* oldDelegate = delegate;
    if (mode == QListView::ListMode) {
        delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::ListMode);
        ui.m_listView->setResizeMode(QListView::Fixed);
    } else {
        delegate = new ItemsGridViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::IconMode);
        ui.m_listView->setResizeMode(QListView::Adjust);
    }
    ui.m_listView->setItemDelegate(delegate);
    delete oldDelegate;
    
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));
    q->connect(delegate, SIGNAL(signalShowDetails(KNS3::EntryInternal)), q, SLOT(slotShowDetails(KNS3::EntryInternal)));
}

void DownloadWidgetPrivate::slotProvidersLoaded()
{
    kDebug() << "providers loaded";
    engine->reloadEntries();
}

void DownloadWidgetPrivate::slotEntriesLoaded(const EntryInternal::List& entries)
{
    foreach(const KNS3::EntryInternal &entry, entries) {
        if (!categories.contains(entry.category())) {
            kDebug() << "Found category: " << entry.category();
            categories.insert(entry.category());
        }
    }
    model->slotEntriesLoaded(entries);
}

void DownloadWidgetPrivate::displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs)
{
    if (!messageTimer) {
        messageTimer = new QTimer;
        messageTimer->setSingleShot(true);
        q->connect(messageTimer, SIGNAL(timeout()), q, SLOT(slotResetMessage()));
    }
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    ui.m_titleWidget->setComment(msg, type);

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        //kDebug(551) << "starting the message timer for " << timeOutMs;
        messageTimer->start(timeOutMs);
    }
}

void DownloadWidgetPrivate::slotShowDetails(const KNS3::EntryInternal& entry)
{
    if (!entry.isValid()) {
        kDebug() << "invalid entry";
        return;
    }
    titleText = ui.m_titleWidget->text();
    
    details->setEntry(entry);
    ui.backButton->setVisible(true);
    ui.detailsStack->setCurrentIndex(1);
    ui.descriptionScrollArea->verticalScrollBar()->setValue(0);
    ui.preview1->setImage(QImage());
    ui.preview2->setImage(QImage());
    ui.preview3->setImage(QImage());
    ui.previewBig->setImage(QImage());
}

void DownloadWidgetPrivate::slotShowOverview()
{
    ui.backButton->setVisible(false);
    
    ui.updateButton->setVisible(false);
    ui.installButton->setVisible(false);
    ui.becomeFanButton->setVisible(false);
    ui.uninstallButton->setVisible(false);
    
    ui.detailsStack->setCurrentIndex(0);
    ui.m_titleWidget->setText(titleText);
}


#include "downloadwidget.moc"
