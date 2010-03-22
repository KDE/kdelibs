/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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
#include "uploaddialog.h"

#include <QtCore/QTimer>
#include <QtGui/QScrollBar>
#include <QtGui/QKeyEvent>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"

#include "ui_downloaddialog.h"


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
    d->ui.setupUi(this);

    d->ui.m_titleWidget->setVisible(false);
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
        if (e.status() == EntryInternal::Installed) {
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
    engine->init(configFile);

    // Entries have been fetched and should be shown:
    q->connect(engine, SIGNAL(signalEntriesLoaded(KNS3::EntryInternal::List)), q, SLOT(slotEntriesLoaded(KNS3::EntryInternal::List)));

    q->connect(engine, SIGNAL(signalError(const QString&)), q, SLOT(slotError(const QString&)));

    // An entry has changes - eg because it was installed
    q->connect(engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)), q, SLOT(slotEntryChanged(KNS3::EntryInternal)));

    q->connect(engine, SIGNAL(signalResetView()), model, SLOT(clearEntries()));
    q->connect(engine, SIGNAL(signalEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)),
               model, SLOT(slotEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)));

    delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
    ui.m_listView->setItemDelegate(delegate);
    ui.m_listView->setModel(model);

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

    q->connect(ui.m_categoryCombo, SIGNAL(activated(int)), q, SLOT(slotCategoryChanged(int)));

    ui.m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
    "%1 Add-On Installer",
    KGlobal::activeComponent().aboutData()->programName()));
    ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    // let the search line edit trap the enter key, otherwise it closes the dialog
    ui.m_searchEdit->setTrapReturnKey(true);

    q->connect(ui.m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), q, SLOT(scrollbarValueChanged(int)));

    q->connect(engine, SIGNAL(jobStarted(KJob*, const QString&)), ui.progressIndicator, SLOT(addJob(KJob*, const QString&)));
    q->connect(model, SIGNAL(jobStarted(KJob*, const QString&)), ui.progressIndicator, SLOT(addJob(KJob*, const QString&)));
    q->connect(ui.m_uploadButton, SIGNAL(clicked()), q, SLOT(slotUpload()));
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

void DownloadWidgetPrivate::slotUpload()
{
    kDebug() << "do upload";
    UploadDialog dialog(m_configFile, ui.m_uploadButton);
    dialog.exec();
}

#include "downloadwidget.moc"
