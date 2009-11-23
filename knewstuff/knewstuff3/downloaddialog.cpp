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

#include "downloaddialog.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktitlewidget.h>

#include "ui/itemsmodel.h"
#include "ui/itemsviewdelegate.h"

#include "ui_downloaddialog.h"

const char * ConfigGroup = "DownloadDialog Settings";

using namespace KNS3;

class DownloadDialog::Private {
public:
    Ui::DownloadDialog ui;
    // The engine that does all the work
    Engine *engine;
    // Model to show the entries
    KNS3::ItemsModel* model;
    // sort items according to sort combo
    QSortFilterProxyModel * sortingProxyModel;
    // Timeout for messge display
    QTimer* messageTimer;
    
    ItemsViewDelegate * delegate;
    
    QString searchTerm;
    QSet<Entry> changedEntries;
    
    Private()
        : engine(new Engine), model(new ItemsModel)
       , sortingProxyModel(new QSortFilterProxyModel) , messageTimer(0)
    {
        sortingProxyModel->setFilterRole(ItemsModel::kNameRole);
        sortingProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        sortingProxyModel->setSourceModel(model);
    }
    
    ~Private() {
        delete messageTimer;
        delete delegate;
        delete sortingProxyModel;
        delete model;
        delete engine;
    }

    void init(const QString& configFile)
    {
        engine->init(configFile);
    }

    void displayMessage(const QString & msg, KTitleWidget::MessageType type, int timeOutMs = 0)
    {
        if (!messageTimer) {
            messageTimer = new QTimer;
            messageTimer->setSingleShot(true);
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
};


DownloadDialog::DownloadDialog(QWidget* parent)
    : KDialog(parent)
    , d(new Private)
{
    KComponentData component = KGlobal::activeComponent();
    QString name = component.componentName();
    init(name + ".knsrc");
}

DownloadDialog::DownloadDialog(const QString& configFile, QWidget * parent)
        : KDialog(parent)
        , d(new Private)
{
    init(configFile);
}

void DownloadDialog::init(const QString& configFile)
{
    d->init(configFile);
    
    setButtons(KDialog::None);
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    d->ui.setupUi(_mainWidget);

    d->ui.closeButton->setGuiItem(KStandardGuiItem::Close);
    connect(d->ui.closeButton, SIGNAL(clicked()), SLOT(accept()));
    
    // Entries have been fetched and should be shown:
    connect(d->engine, SIGNAL(signalEntriesLoaded(KNS3::Entry::List)), d->model, SLOT(slotEntriesLoaded(KNS3::Entry::List)));
    
    connect(d->engine, SIGNAL(signalError(const QString&)), SLOT(slotError(const QString&)));

    // An entry has changes - eg because it was installed
    connect(d->engine, SIGNAL(signalEntryChanged(KNS3::Entry)), SLOT(slotEntryChanged(KNS3::Entry)));

    connect(d->engine, SIGNAL(signalResetView()), d->model, SLOT(clearEntries()));
    
    // FIXME show download progress
    connect(d->engine, SIGNAL(signalProgress(QString, int)), SLOT(slotProgress(QString, int)));
    
    connect(d->messageTimer, SIGNAL(timeout()), SLOT(slotResetMessage()));

    d->delegate = new ItemsViewDelegate(d->ui.m_listView);
    d->ui.m_listView->setItemDelegate(d->delegate);
    connect(d->delegate, SIGNAL(performAction(KNS3::Engine::EntryAction, const KNS3::Entry&)),
            d->engine, SLOT(slotPerformAction(KNS3::Engine::EntryAction, const KNS3::Entry&)));

    d->ui.m_listView->setModel(d->sortingProxyModel);

    // create left picture widget (if picture found)
    //QPixmap p( KStandardDirs::locate( "data", "knewstuff/pics/ghns.png" ) );
    //if ( !p.isNull() )
    //   horLay->addWidget( new ExtendImageWidget( p, this ) );
    // FIXME KDE4PORT: if we use a left bar image, find a better way

    // FIXME set sorting options in m_sortCombo, make the sortFilterProxyModel use the sorting
    // maybe also clear the list of entries and ask providers to refetch them (either from cache or dynamically)
    d->ui.m_sortCombo->insertItem(Provider::Rating, i18nc("Sorting order of the list of items in get hot new stuff", "Rating"));
    d->ui.m_sortCombo->insertItem(Provider::Newest, i18nc("Sorting order of the list of items in get hot new stuff", "Newest"));
    d->ui.m_sortCombo->insertItem(Provider::Downloads, i18nc("Sorting order of the list of items in get hot new stuff", "Most Downloads"));
    d->ui.m_sortCombo->insertItem(Provider::Alphabetical, i18nc("Sorting order of the list of items in get hot new stuff", "Alphabetical"));
    d->ui.m_sortCombo->insertItem(Provider::Installed, i18nc("Sorting order of the list of items in get hot new stuff", "Installed only"));
    
    connect(d->ui.m_sortCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotSortingSelected(int)));
    connect(d->ui.m_searchEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSearchTextChanged()));
    connect(d->ui.m_searchEdit, SIGNAL(editingFinished()), SLOT(slotUpdateSearch()));

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
    d->ui.m_titleWidget->setText(i18nc("Program name followed by 'Add On Installer'",
                                 "%1 Add-On Installer",
                                 KGlobal::activeComponent().aboutData()->programName()));
    d->ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    connect(d->ui.m_listView->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(scrollbarValueChanged(int)));
    
    // FIXME connect(d->engine, SIGNAL(signalJobStarted(KJob*)), d->ui.progressIndicator, SLOT(addJob(KJob*)));
    connect(d->model, SIGNAL(jobStarted(KJob*, const QString&)), d->ui.progressIndicator, SLOT(addJob(KJob*, const QString&)));
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KGlobal::config(), ConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
    delete d;
}

void DownloadDialog::slotResetMessage() // SLOT
{
    d->ui.m_titleWidget->setComment(QString());
}

void DownloadDialog::slotNetworkTimeout() // SLOT
{
    d->displayMessage(i18n("Timeout. Check Internet connection."), KTitleWidget::ErrorMessage);
}

void DownloadDialog::slotSortingSelected(int sortType)   // SLOT
{
    d->model->clearEntries();
    if (sortType == Provider::Installed) {
        d->ui.m_searchEdit->clear();
    }
    d->ui.m_searchEdit->setEnabled(sortType != Provider::Installed);
    
    d->engine->setSortMode((Provider::SortMode)sortType);
    d->engine->reloadEntries();
}

void DownloadDialog::slotUpdateSearch()
{
    if (d->searchTerm == d->ui.m_searchEdit->text().trimmed()) {
        return;
    }
    d->searchTerm = d->ui.m_searchEdit->text().trimmed();
}

void DownloadDialog::slotSearchTextChanged()
{
    if (d->searchTerm == d->ui.m_searchEdit->text().trimmed()) {
        return;
    }
    d->searchTerm = d->ui.m_searchEdit->text().trimmed();
    d->engine->setSearchTerm(d->ui.m_searchEdit->text().trimmed());
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

void DownloadDialog::slotEntryChanged(const Entry& entry)
{
    d->changedEntries.insert(entry);
    d->model->slotEntryChanged(entry);
}

void DownloadDialog::slotPayloadFailed(const Entry& entry)
{
    setCursor(Qt::ArrowCursor);
    KMessageBox::error(this, i18n("Could not install %1", entry.name()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadDialog::slotPayloadLoaded(KUrl url)
{
    Q_UNUSED(url)
    setCursor(Qt::ArrowCursor);
}

void DownloadDialog::slotError(const QString& message)
{
    KMessageBox::error(this, message, i18n("Get Hot New Stuff"));
}

void DownloadDialog::scrollbarValueChanged(int value)
{
    if ((double)value/d->ui.m_listView->verticalScrollBar()->maximum() > 0.9) {
        d->engine->slotRequestMoreData();
    }
}

Entry::List DownloadDialog::changedEntries()
{
    return d->changedEntries.toList();
}

Entry::List DownloadDialog::installedEntries()
{
    QList<Entry> entries;
    foreach (Entry e, d->changedEntries) {
        if (e.status() == Entry::Installed) {
            entries.append(e);
        }
    }
    return entries;
}


#include "downloaddialog.moc"
