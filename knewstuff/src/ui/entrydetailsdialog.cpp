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

#include <QMenu>
#include <QDebug>

#include "core/engine.h"
#include "ui/imageloader.h"
#include <attica/provider.h>

using namespace KNS3;

EntryDetails::EntryDetails(Engine* engine, Ui::DownloadWidget* widget)
    : QObject(widget->m_listView), m_engine(engine), ui(widget)
{
    init();
}

EntryDetails::~EntryDetails()
{
}

void EntryDetails::init()
{
    connect(ui->preview1, SIGNAL(clicked()), this, SLOT(preview1Selected()));
    connect(ui->preview2, SIGNAL(clicked()), this, SLOT(preview2Selected()));
    connect(ui->preview3, SIGNAL(clicked()), this, SLOT(preview3Selected()));

    ui->ratingWidget->setMaxRating(10);
    ui->ratingWidget->setHalfStepsEnabled(true);

    updateButtons();
    connect(ui->installButton, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui->uninstallButton, SIGNAL(clicked()), this, SLOT(uninstall()));
    // updating is the same as installing
    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(install()));
    connect(ui->becomeFanButton, SIGNAL(clicked()), this, SLOT(becomeFan()));

    ui->installButton->setIcon(QIcon::fromTheme("dialog-ok"));
    ui->updateButton->setIcon(QIcon::fromTheme("system-software-update"));
    ui->uninstallButton->setIcon(QIcon::fromTheme("edit-delete"));

    connect(m_engine, SIGNAL(signalEntryDetailsLoaded(KNS3::EntryInternal)),
            this, SLOT(entryChanged(KNS3::EntryInternal)));
    connect(m_engine, SIGNAL(signalEntryChanged(KNS3::EntryInternal)),
            this, SLOT(entryStatusChanged(KNS3::EntryInternal)));
    connect(m_engine, SIGNAL(signalEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)),
            this, SLOT(slotEntryPreviewLoaded(KNS3::EntryInternal,KNS3::EntryInternal::PreviewType)));
}

void EntryDetails::setEntry(const KNS3::EntryInternal& entry)
{
    m_entry = entry;
    // immediately show something
    entryChanged(m_entry);
    // fetch more preview images
    m_engine->loadDetails(m_entry);
}

void EntryDetails::entryChanged(const KNS3::EntryInternal& entry)
{
    if (ui->detailsStack->currentIndex() == 0) {
        return;
    }
    m_entry = entry;

    // FIXME
    //ui->ratingWidget->setEditable(m_engine->userCanVote(m_entry));

    if (!m_engine->userCanBecomeFan(m_entry)) {
        ui->becomeFanButton->setEnabled(false);
    }

    ui->m_titleWidget->setText(i18n("Details for %1", m_entry.name()));
    if (!m_entry.author().homepage().isEmpty()) {
        ui->authorLabel->setText("<a href=\"" + m_entry.author().homepage() + "\">" + m_entry.author().name() + "</a>");
    } else if (!m_entry.author().email().isEmpty()) {
        ui->authorLabel->setText("<a href=\"mailto:" + m_entry.author().email() + "\">" + m_entry.author().name() + "</a>");
    } else {
        ui->authorLabel->setText(m_entry.author().name());
    }

    QString summary = replaceBBCode(m_entry.summary()).replace("\n", "<br/>");
    QString changelog = replaceBBCode(m_entry.changelog()).replace("\n", "<br/>");

    QString description = "<html><body>" + summary;
    if (!changelog.isEmpty()) {
        description += "<br/><p><b>" + i18n("Changelog:") + "</b><br/>" + changelog + "</p>";
    }
    description += "</body></html>";
    ui->descriptionLabel->setText(description);
    
    QString homepageText("<a href=\"" + m_entry.homepage().url() + "\">" +
                              i18nc("A link to the description of this Get Hot New Stuff item", "Homepage") + "</a>");

    if (!m_entry.donationLink().isEmpty()) {
        homepageText += "<br><a href=\"" + m_entry.donationLink() + "\">" + i18nc("A link to make a donation for a Get Hot New Stuff item (opens a web browser)", "Make a donation") + "</a>";
    }
    if (!m_entry.knowledgebaseLink().isEmpty()) {
        homepageText += "<br><a href=\"" + m_entry.knowledgebaseLink() + "\">" 
            + i18ncp("A link to the knowledgebase (like a forum) (opens a web browser)", "Knowledgebase (no entries)", "Knowledgebase (%1 entries)", m_entry.numberKnowledgebaseEntries()) + "</a>";
    }
    ui->homepageLabel->setText(homepageText);
    ui->homepageLabel->setToolTip(i18nc("Tooltip for a link in a dialog", "Opens in a browser window"));
    
    if (m_entry.rating() > 0) {
        ui->ratingWidget->setVisible(true);
        disconnect(ui->ratingWidget, SIGNAL(ratingChanged(uint)), this, SLOT(ratingChanged(uint)));
        // Most of the voting is 20 - 80, so rate 20 as 0 stars and 80 as 5 stars
        int rating = qMax(0, qMin(10, (m_entry.rating()-20)/6));
        ui->ratingWidget->setRating(rating);
        connect(ui->ratingWidget, SIGNAL(ratingChanged(uint)), this, SLOT(ratingChanged(uint)));
    } else {
        ui->ratingWidget->setVisible(false);
    }

    bool hideSmallPreviews = m_entry.previewUrl(EntryInternal::PreviewSmall2).isEmpty()
           && m_entry.previewUrl(EntryInternal::PreviewSmall3).isEmpty();
           
    ui->preview1->setVisible(!hideSmallPreviews);
    ui->preview2->setVisible(!hideSmallPreviews);
    ui->preview3->setVisible(!hideSmallPreviews);

    // in static xml we often only get a small preview, use that in details
    if(m_entry.previewUrl(EntryInternal::PreviewBig1).isEmpty() && !m_entry.previewUrl(EntryInternal::PreviewSmall1).isEmpty()) {
        m_entry.setPreviewUrl(m_entry.previewUrl(EntryInternal::PreviewSmall1), EntryInternal::PreviewBig1);
        m_entry.setPreviewImage(m_entry.previewImage(EntryInternal::PreviewSmall1), EntryInternal::PreviewBig1);
    }

    for (int type = EntryInternal::PreviewSmall1; type <= EntryInternal::PreviewBig3; ++type) {
        if (m_entry.previewUrl(EntryInternal::PreviewSmall1).isEmpty()) {
            ui->previewBig->setVisible(false);
        } else

        if (!m_entry.previewUrl((EntryInternal::PreviewType)type).isEmpty()) {
            // qDebug() << "type: " << type << m_entry.previewUrl((EntryInternal::PreviewType)type);
            if (m_entry.previewImage((EntryInternal::PreviewType)type).isNull()) {
                m_engine->loadPreview(m_entry, (EntryInternal::PreviewType)type);
            } else {
                slotEntryPreviewLoaded(m_entry, (EntryInternal::PreviewType)type);
            }
        }
    }
    
    updateButtons();
}

void EntryDetails::entryStatusChanged(const KNS3::EntryInternal& entry)
{
    Q_UNUSED(entry);
    updateButtons();
}

void EntryDetails::updateButtons()
{
    if (ui->detailsStack->currentIndex() == 0) {
        return;
    }
    // qDebug() << "update buttons: " << m_entry.status();
    ui->installButton->setVisible(false);
    ui->uninstallButton->setVisible(false);
    ui->updateButton->setVisible(false);

    switch (m_entry.status()) {
        case Entry::Installed:
            ui->uninstallButton->setVisible(true);
            ui->uninstallButton->setEnabled(true);
            break;
        case Entry::Updateable:
            ui->updateButton->setVisible(true);
            ui->updateButton->setEnabled(true);
            ui->uninstallButton->setVisible(true);
            ui->uninstallButton->setEnabled(true);
            break;

        case Entry::Invalid:
        case Entry::Downloadable:
            ui->installButton->setVisible(true);
            ui->installButton->setEnabled(true);
            break;

        case Entry::Installing:
            ui->installButton->setVisible(true);
            ui->installButton->setEnabled(false);
            break;
        case Entry::Updating:
            ui->updateButton->setVisible(true);
            ui->updateButton->setEnabled(false);
            ui->uninstallButton->setVisible(true);
            ui->uninstallButton->setEnabled(false);
            break;
        case Entry::Deleted:
            ui->installButton->setVisible(true);
            ui->installButton->setEnabled(true);
            break;
    }
    
    if (ui->installButton->menu()) {
        QMenu* buttonMenu = ui->installButton->menu();
        buttonMenu->clear();
        ui->installButton->setMenu(0);
        buttonMenu->deleteLater();
    }
    if (ui->installButton->isVisible() && m_entry.downloadLinkCount() > 1) {
        QMenu * installMenu = new QMenu(ui->installButton);
        foreach (EntryInternal::DownloadLinkInformation info, m_entry.downloadLinkInformationList()) {
            QString text = info.name;
            if (!info.distributionType.trimmed().isEmpty()) {
                text + " (" + info.distributionType.trimmed() + ")";
            }
            QAction* installAction = installMenu->addAction(QIcon::fromTheme("dialog-ok"), text);
            installAction->setData(info.id);
        }
        // qDebug() << "links: " << m_entry.downloadLinkInformationList().size();
        ui->installButton->setMenu(installMenu);
    }
}

void EntryDetails::install()
{
    m_engine->install(m_entry);
}

void EntryDetails::uninstall()
{
    m_engine->uninstall(m_entry);
}

void EntryDetails::slotEntryPreviewLoaded(const KNS3::EntryInternal& entry, KNS3::EntryInternal::PreviewType type)
{
    if (!(entry == m_entry)) {
        return;
    }

    switch (type) {
    case EntryInternal::PreviewSmall1:
        ui->preview1->setImage(entry.previewImage(EntryInternal::PreviewSmall1));
        break;
    case EntryInternal::PreviewSmall2:
        ui->preview2->setImage(entry.previewImage(EntryInternal::PreviewSmall2));
        break;
    case EntryInternal::PreviewSmall3:
        ui->preview3->setImage(entry.previewImage(EntryInternal::PreviewSmall3));
        break;
    case EntryInternal::PreviewBig1:
        m_currentPreview = entry.previewImage(EntryInternal::PreviewBig1);
        ui->previewBig->setImage(m_currentPreview);
        break;
    default:
        break;
    }
}

void EntryDetails::preview1Selected()
{
    previewSelected(0);
}

void EntryDetails::preview2Selected()
{
    previewSelected(1);
}

void EntryDetails::preview3Selected()
{
    previewSelected(2);
}

void EntryDetails::previewSelected(int current)
{
    EntryInternal::PreviewType type = static_cast<EntryInternal::PreviewType>(EntryInternal::PreviewBig1 + current);
    m_currentPreview = m_entry.previewImage(type);
    ui->previewBig->setImage(m_currentPreview);
}

void EntryDetails::ratingChanged(uint rating)
{
    // engine expects values from 0..100
    // qDebug() << "rating: " << rating << " -> " << rating*10;
    m_engine->vote(m_entry, rating*10);
}

void EntryDetails::becomeFan()
{
    m_engine->becomeFan(m_entry);
}


