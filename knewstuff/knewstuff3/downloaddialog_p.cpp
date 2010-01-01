/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jeremy@scitools.com>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "downloaddialog_p.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktitlewidget.h>
#include <kdebug.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"

#include "ui_downloaddialog.h"


using namespace KNS3;

DownloadDialogPrivate::DownloadDialogPrivate()
: engine(new Engine), model(new ItemsModel)
, sortingProxyModel(new QSortFilterProxyModel) , messageTimer(0)
{
    sortingProxyModel->setFilterRole(ItemsModel::kNameRole);
    sortingProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    sortingProxyModel->setSourceModel(model);
}

DownloadDialogPrivate::~DownloadDialogPrivate()
{
    delete messageTimer;
    delete delegate;
    delete sortingProxyModel;
    delete model;
    delete engine;
}

void DownloadDialogPrivate::slotResetMessage() // SLOT
{
    ui.m_titleWidget->setComment(QString());
}

void DownloadDialogPrivate::slotNetworkTimeout() // SLOT
{
    displayMessage(i18n("Timeout. Check Internet connection."), KTitleWidget::ErrorMessage);
}

void DownloadDialogPrivate::sortingChanged()
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

void DownloadDialogPrivate::slotUpdateSearch()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
}

void DownloadDialogPrivate::slotSearchTextChanged()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
    engine->setSearchTerm(ui.m_searchEdit->text().trimmed());
}

void DownloadDialogPrivate::slotCategoryChanged(int idx)
{
    if (idx == 0) {
        // All Categories item selected, reset filter
        engine->setCategoriesFilter(QStringList());
    }

    QString category = ui.m_categoryCombo->currentText();
    if (!category.isEmpty()) {
        QStringList filter(category);
        engine->setCategoriesFilter(filter);
    }
}

void DownloadDialogPrivate::slotInfo(QString provider, QString server, QString version)
{
    QString link = QString("<a href=\"%1\">%1</a>").arg(server);
    QString infostring = i18n("Server: %1", link);
    infostring += i18n("<br />Provider: %1", provider);
    infostring += i18n("<br />Version: %1", version);

    KMessageBox::information(0,
                             infostring,
                             i18n("Provider information"));
}

void DownloadDialogPrivate::slotEntryChanged(const EntryInternal& entry)
{
    changedEntries.insert(entry);
    model->slotEntryChanged(entry);
}

void DownloadDialogPrivate::slotPayloadFailed(const EntryInternal& entry)
{
    KMessageBox::error(0, i18n("Could not install %1", entry.name()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadDialogPrivate::slotPayloadLoaded(KUrl url)
{
    Q_UNUSED(url)
}

void DownloadDialogPrivate::slotError(const QString& message)
{
    KMessageBox::error(0, message, i18n("Get Hot New Stuff"));
}

void DownloadDialogPrivate::scrollbarValueChanged(int value)
{
    if ((double)value/ui.m_listView->verticalScrollBar()->maximum() > 0.9) {
        engine->requestMoreData();
    }
}

void DownloadDialogPrivate::init(const QString& configFile)
{
    engine->init(configFile);

    // Entries have been fetched and should be shown:
    connect(engine, SIGNAL(signalEntriesLoaded(KNS3::EntryInternal::List)), this, SLOT(slotEntriesLoaded(KNS3::EntryInternal::List)));

    connect(engine, SIGNAL(signalError(const QString&)), SLOT(slotError(const QString&)));

    // An entry has changes - eg because it was installed
    connect(engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)), SLOT(slotEntryChanged(KNS3::EntryInternal)));

    connect(engine, SIGNAL(signalResetView()), model, SLOT(clearEntries()));

    // FIXME show download progress
    connect(engine, SIGNAL(signalProgress(QString, int)), SLOT(slotProgress(QString, int)));

    delegate = new ItemsViewDelegate(ui.m_listView);
    ui.m_listView->setItemDelegate(delegate);
    connect(delegate, SIGNAL(performAction(KNS3::Engine::EntryAction, const KNS3::EntryInternal&)),
            engine, SLOT(slotPerformAction(KNS3::Engine::EntryAction, const KNS3::EntryInternal&)));

    ui.m_listView->setModel(sortingProxyModel);

    connect(ui.newestRadio,  SIGNAL(clicked()), this, SLOT(sortingChanged()));
    connect(ui.ratingRadio,  SIGNAL(clicked()), this, SLOT(sortingChanged()));
    connect(ui.mostDownloadsRadio,  SIGNAL(clicked()), this, SLOT(sortingChanged()));
    connect(ui.installedRadio,  SIGNAL(clicked()), this, SLOT(sortingChanged()));

    connect(ui.m_searchEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSearchTextChanged()));
    connect(ui.m_searchEdit, SIGNAL(editingFinished()), SLOT(slotUpdateSearch()));

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

    connect(ui.m_categoryCombo, SIGNAL(activated(int)), SLOT(slotCategoryChanged(int)));


    ui.m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
    "%1 Add-On Installer",
    KGlobal::activeComponent().aboutData()->programName()));
    ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    connect(ui.m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollbarValueChanged(int)));

    // FIXME connect(engine, SIGNAL(signalJobStarted(KJob*)), ui.progressIndicator, SLOT(addJob(KJob*)));
    connect(model, SIGNAL(jobStarted(KJob*, const QString&)), ui.progressIndicator, SLOT(addJob(KJob*, const QString&)));
}

void DownloadDialogPrivate::slotEntriesLoaded(const EntryInternal::List& entries)
{
    foreach(const KNS3::EntryInternal &entry, entries) {
        if (!categories.contains(entry.category())) {
            kDebug() << "Found category: " << entry.category();
            categories.insert(entry.category());
        }
    }
    model->slotEntriesLoaded(entries);
}

void DownloadDialogPrivate::displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs)
{
    if (!messageTimer) {
        messageTimer = new QTimer;
        messageTimer->setSingleShot(true);
        connect(messageTimer, SIGNAL(timeout()), SLOT(slotResetMessage()));
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

#include "downloaddialog_p.moc"
