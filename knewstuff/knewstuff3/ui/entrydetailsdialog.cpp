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



EntryDetailsDialog::EntryDetailsDialog(Engine* engine, const Entry& entry, QWidget* parent)
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
    ui.m_titleWidget->setText(i18n("Details for %1", m_entry.name()));
    ui.m_titleWidget->setPixmap(KIcon(KGlobal::activeComponent().aboutData()->programIconName()));

    ui.previewSmallGroup->setVisible(false);

    ui.authorLabel->setText(m_entry.author().name());
    ui.descriptionLabel->setText(m_entry.summary());
    ui.homepageLabel->setText("<a href=\"" + m_entry.homepage().url() + "\">" +
                              i18nc("A link to the description of this Get Hot New Stuff item", "Visit homepage...") + "</a>");


    ui.ratingWidget->setMaxRating(10);
    ui.ratingWidget->setHalfStepsEnabled(true);
    ui.ratingWidget->setEditable(false);
    ui.ratingWidget->setRating((m_entry.rating()-20)/6);

    connect(ui.voteGoodButton, SIGNAL(clicked()), this, SLOT(voteGood()));
    connect(ui.voteBadButton, SIGNAL(clicked()), this, SLOT(voteBad()));
    connect(ui.becomeFanButton, SIGNAL(clicked()), this, SLOT(becomeFan()));

    if(m_entry.previewSmall().isEmpty() && m_entry.previewBig().isEmpty()) {
        ui.previewBig->setVisible(false);
    } else {
        QString url = m_entry.previewBig();
        if (url.isEmpty()) {
            url = m_entry.previewSmall();
        }
        ui.previewBig->setText(i18n("Loading preview..."));
        ImageLoader *pix = new ImageLoader(url, this);
        connect(pix, SIGNAL(signalLoaded(const QString &, const QImage&)),
                this, SLOT(slotEntryPreviewLoaded(const QString &, const QImage&)));

        ui.previewBig->installEventFilter(this);
    }
}

void EntryDetailsDialog::slotEntryPreviewLoaded(const QString &, const QImage& image)
{
    m_previewBig1 = image;
    ui.previewBig->setPixmap(QPixmap::fromImage(image.scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));
}

bool EntryDetailsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        kDebug() << "Resize";
        ui.previewBig->setPixmap(QPixmap::fromImage(m_previewBig1.scaled(ui.previewBig->size(), Qt::KeepAspectRatio)));

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

