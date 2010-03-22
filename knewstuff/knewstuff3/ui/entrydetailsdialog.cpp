/*
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

#include "entrydetailsdialog.h"

#include <QtGui/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtGui/QScrollBar>

#include <kmessagebox.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktitlewidget.h>
#include <kdebug.h>

#include <knewstuff3/core/engine.h>
#include <knewstuff3/ui/imageloader.h>

#include "ui_entrydetailsdialog.h"

const char * EntryDetailsConfigGroup = "EntryDetailsDialog Settings";

using namespace KNS3;


EntryDetailsDialog::EntryDetailsDialog(Engine* engine, const EntryInternal& entry, QWidget* parent)
    : KDialog(parent), m_engine(engine), m_entry(entry)
{
    init();
}

EntryDetailsDialog::~EntryDetailsDialog()
{
    KConfigGroup group(KGlobal::config(), EntryDetailsConfigGroup);
    saveDialogSize(group, KConfigBase::Persistent);
}

void EntryDetailsDialog::init()
{
    setButtons(KDialog::None);
    QWidget* _mainWidget = new QWidget(this);
    setMainWidget(_mainWidget);
    ui.setupUi(_mainWidget);

    ui.closeButton->setGuiItem(KStandardGuiItem::Close);
    connect(ui.closeButton, SIGNAL(clicked()), SLOT(accept()));

    // load the last size from config
    KConfigGroup group(KGlobal::config(), EntryDetailsConfigGroup);
    restoreDialogSize(group);
    setMinimumSize(700, 400);

    setCaption(i18n("Get Hot New Stuff"));
    ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    // catch resize events
    ui.previewBig->installEventFilter(this);

    ui.ratingWidget->setMaxRating(10);
    ui.ratingWidget->setHalfStepsEnabled(true);
    ui.ratingWidget->setEditable(false);

    updateButtons();
    connect(ui.installButton, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui.uninstallButton, SIGNAL(clicked()), this, SLOT(uninstall()));
    // updating is the same as installing
    connect(ui.updateButton, SIGNAL(clicked()), this, SLOT(install()));

    connect(ui.voteGoodButton, SIGNAL(clicked()), this, SLOT(voteGood()));
    connect(ui.voteBadButton, SIGNAL(clicked()), this, SLOT(voteBad()));
    connect(ui.becomeFanButton, SIGNAL(clicked()), this, SLOT(becomeFan()));

    ui.installButton->setIcon(KIcon("dialog-ok"));
    ui.updateButton->setIcon(KIcon("system-software-update"));
    ui.uninstallButton->setIcon(KIcon("edit-delete"));

    connect(m_engine, SIGNAL(signalEntryDetailsLoaded(KNS3::EntryInternal)),
            this, SLOT(entryChanged(KNS3::EntryInternal)));
    connect(m_engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)),
            this, SLOT(entryStatusChanged(KNS3::EntryInternal)));
    connect(m_engine, SIGNAL(signalEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)),
            this, SLOT(slotEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)));

    // immediately show something
    entryChanged(m_entry);
    // fetch more preview images
    m_engine->loadDetails(m_entry);
}

void EntryDetailsDialog::entryChanged(const KNS3::EntryInternal& entry)
{
    m_entry = entry;
    kDebug() << "entry changed!!!!!!!!!1111111eleven" << entry.name();

    if (!m_engine->userCanVote(m_entry)) {
        ui.voteGoodButton->setEnabled(false);
        ui.voteBadButton->setEnabled(false);
    }
    if (!m_engine->userCanBecomeFan(m_entry)) {
        ui.becomeFanButton->setEnabled(false);
    }

    ui.m_titleWidget->setText(i18n("Details for %1", m_entry.name()));
    if (!m_entry.author().homepage().isEmpty()) {
        ui.authorLabel->setText("<a href=\"" + m_entry.author().homepage() + "\">" + m_entry.author().name() + "</a>");
    } else {
        ui.authorLabel->setText(m_entry.author().name());
    }

    // TODO: create function to replace bb codes (also use that in delegate)
    QString summary = m_entry.summary();
    summary.replace("[b]", "<b>");
    summary.replace("[/b]", "</b>");
    summary.replace("[i]", "<i>");
    summary.replace("[/i]", "</i>");
    summary.replace("[u]", "<i>");
    summary.replace("[/u]", "</i>");
    summary.remove("[url]");
    summary.remove("[/url]");
    summary.replace("\\\'", "\'");
    ui.descriptionLabel->setText(summary);
    ui.homepageLabel->setText("<a href=\"" + m_entry.homepage().url() + "\">" +
                              i18nc("A link to the description of this Get Hot New Stuff item", "Visit homepage...") + "</a>");

    ui.ratingWidget->setRating((m_entry.rating()-20)/6);

//    ui.previewSmall1->setVisible(false);
//    ui.previewSmall2->setVisible(false);
//    ui.previewSmall3->setVisible(false);

    for (int type = EntryInternal::PreviewSmall1; type < EntryInternal::PreviewBig3; ++type) {
        kDebug() << "LOAD: " << type;

        if (m_entry.previewUrl(EntryInternal::PreviewSmall1).isEmpty()) {
            ui.previewBig->setVisible(false);
        } else

        if (m_entry.previewImage((EntryInternal::PreviewType)type).isNull()) {
            m_engine->loadPreview(m_entry, (EntryInternal::PreviewType)type);
        } else {
            slotEntryPreviewLoaded(m_entry, (EntryInternal::PreviewType)type);
        }
    }

    if(m_entry.previewImage(EntryInternal::PreviewBig1).isNull() && !m_entry.previewUrl(EntryInternal::PreviewBig1).isEmpty()) {
        ui.previewBig->setText(i18n("Loading preview..."));
    }
    updateButtons();
}

void EntryDetailsDialog::entryStatusChanged(const KNS3::EntryInternal& entry)
{
    Q_UNUSED(entry);
    updateButtons();
}

void EntryDetailsDialog::updateButtons()
{
    kDebug() << "update buttons: " << m_entry.status();
    ui.installButton->setVisible(false);
    ui.uninstallButton->setVisible(false);
    ui.updateButton->setVisible(false);

    switch (m_entry.status()) {
        case EntryInternal::Installed:
            ui.uninstallButton->setVisible(true);
            ui.uninstallButton->setEnabled(true);
            break;
        case EntryInternal::Updateable:
            ui.updateButton->setVisible(true);
            ui.updateButton->setEnabled(true);
            ui.uninstallButton->setVisible(true);
            ui.uninstallButton->setEnabled(true);
            break;

        case EntryInternal::Invalid:
        case EntryInternal::Downloadable:
            ui.installButton->setVisible(true);
            ui.installButton->setEnabled(true);
            break;

        case EntryInternal::Installing:
            ui.installButton->setVisible(true);
            ui.installButton->setEnabled(false);
            break;
        case EntryInternal::Updating:
            ui.updateButton->setVisible(true);
            ui.updateButton->setEnabled(false);
            ui.uninstallButton->setVisible(true);
            ui.uninstallButton->setEnabled(false);
            break;
        case EntryInternal::Deleted:
            ui.installButton->setVisible(true);
            ui.installButton->setEnabled(true);
            break;
    }
}

void EntryDetailsDialog::install()
{
    m_engine->install(m_entry);
}

void EntryDetailsDialog::uninstall()
{
    m_engine->uninstall(m_entry);
}

void EntryDetailsDialog::slotEntryPreviewLoaded(const KNS3::EntryInternal& entry, KNS3::EntryInternal::PreviewType type)
{
    if (!(entry == m_entry)) {
        return;
    }

    kDebug() << "entry preview loaded : " << type << entry.name();
    switch (type) {
    case EntryInternal::PreviewSmall1:
        ui.previewSmall1->setPixmap(QPixmap::fromImage(entry.previewImage(EntryInternal::PreviewSmall1).scaled(ui.previewSmall1->size(), Qt::KeepAspectRatio)));
        break;
    case EntryInternal::PreviewSmall2:
        ui.previewSmall2->setPixmap(QPixmap::fromImage(entry.previewImage(EntryInternal::PreviewSmall2).scaled(ui.previewSmall2->size(), Qt::KeepAspectRatio)));
        break;
    case EntryInternal::PreviewSmall3:
        ui.previewSmall3->setPixmap(QPixmap::fromImage(entry.previewImage(EntryInternal::PreviewSmall3).scaled(ui.previewSmall3->size(), Qt::KeepAspectRatio)));
        break;
    case EntryInternal::PreviewBig1:
        kDebug() << "preview big 1";
        m_currentPreview = entry.previewImage(EntryInternal::PreviewBig1);
        ui.previewBig->setPixmap(QPixmap::fromImage(m_currentPreview.scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));
        break;
    case EntryInternal::PreviewBig2:
        kDebug() << "preview big 2";
        ui.previewBig->setPixmap(QPixmap::fromImage(entry.previewImage(EntryInternal::PreviewBig2).scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));
        break;
    case EntryInternal::PreviewBig3:
        kDebug() << "preview big 3";
        ui.previewBig->setPixmap(QPixmap::fromImage(entry.previewImage(EntryInternal::PreviewBig3).scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));
        break;
    }
}

bool EntryDetailsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        ui.previewBig->setPixmap(QPixmap::fromImage(m_currentPreview.scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));
    }
    return KDialog::eventFilter(obj, event);
}

void EntryDetailsDialog::voteGood()
{
    m_engine->vote(m_entry, true);
}

void EntryDetailsDialog::voteBad()
{
    m_engine->vote(m_entry, false);
}

void EntryDetailsDialog::becomeFan()
{
    m_engine->becomeFan(m_entry);
}

#include "entrydetailsdialog.moc"

