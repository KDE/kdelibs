/*
    This file is part of KNewStuff2.
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>

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

#include "itemsviewdelegate.h"
#include "itemsmodel.h"

#include <QtGui/QPainter>
#include <QtGui/QSortFilterProxyModel>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <krun.h>

static const int kLabel = 0;
static const int kInstall = 1;
static const int kRating = 2;

namespace KNS
{
ItemsViewDelegate::ItemsViewDelegate(QAbstractItemView *itemView, QObject * parent)
        : KWidgetItemDelegate(itemView, parent)
{
    QString framefile = KStandardDirs::locate("data", "knewstuff/pics/thumb_frame.png");

    m_frameImage = QPixmap(framefile).toImage();

    // Invalid
    m_statusicons << KIcon("dialog-error");
    // Downloadable
    m_statusicons << KIcon();
    //Installed
    m_statusicons << KIcon("dialog-ok");
    //Updateable
    m_statusicons << KIcon("system-software-update");
    //Deleted
    m_statusicons << KIcon("edit-delete");
}

ItemsViewDelegate::~ItemsViewDelegate()
{
}

KMenu * ItemsViewDelegate::InstallMenu(const QToolButton* button, Entry::Status status) const
{
    Q_UNUSED(button)
    KMenu * installMenu = new KMenu(NULL);
    QAction * action_install = installMenu->addAction(m_statusicons[Entry::Installed], i18n("Install"));
    QAction * action_uninstall = installMenu->addAction(m_statusicons[Entry::Deleted], i18n("Uninstall"));
    action_install->setData(DownloadDialog::kInstall);
    action_uninstall->setData(DownloadDialog::kUninstall);

    action_install->setVisible(status != Entry::Installed);
    action_uninstall->setVisible(status == Entry::Installed);
    return installMenu;
}

QList<QWidget*> ItemsViewDelegate::createItemWidgets() const
{
    QList<QWidget*> list;

    QLabel * infoLabel = new QLabel();
    infoLabel->setOpenExternalLinks(true);
    list << infoLabel;

    QToolButton * installButton = new QToolButton();
    list << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, SIGNAL(triggered(QAction *)), this, SLOT(slotActionTriggered(QAction *)));
    connect(installButton, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));

    QLabel * ratingLabel = new QLabel();
    list << ratingLabel;

    return list;
}

void ItemsViewDelegate::updateItemWidgets(const QList<QWidget*> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (model == NULL) {
        return;
    }

    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
    if (realmodel == NULL || !index.isValid()) {
        return;
    }

    // setup the install button
    int margin = option.fontMetrics.height() / 2;

    int right = option.rect.width();
    //int bottom = option.rect.height();

    QSize size(option.fontMetrics.height() * 7, widgets.at(kInstall)->sizeHint().height());

    QLabel * infoLabel = qobject_cast<QLabel*>(widgets.at(kLabel));
	infoLabel->setWordWrap(true);
    if (infoLabel != NULL) {
        if (realmodel->hasPreviewImages()) {
            // move the text right by kPreviewWidth + margin pixels to fit the preview
            infoLabel->move(kPreviewWidth + margin * 2, 0);
            infoLabel->resize(QSize(option.rect.width() - kPreviewWidth - (margin * 6) - size.width(), option.fontMetrics.height() * 7));
        } else {
            infoLabel->move(margin, 0);
            infoLabel->resize(QSize(option.rect.width() - (margin * 4) - size.width(), option.fontMetrics.height() * 7));
        }

        QString text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
			"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; margin:0 0 0 0;}\n"
			"</style></head><body><p><b>" +
			index.data(ItemsModel::kNameRole).toString() + "</b></p>\n";

        QString summary = "<p>" + option.fontMetrics.elidedText(index.data(ItemsModel::kSummary).toString(),
		  Qt::ElideRight, infoLabel->width() * 3) + "</p>\n";
        text += summary;

        QString authorName = index.data(ItemsModel::kAuthorName).toString();
        QString email = index.data(ItemsModel::kAuthorEmail).toString();
        if (!authorName.isEmpty()) {
            if (email.isEmpty()) {
                text += "<p><i>" + authorName + "</i></p>\n";
            } else {
                text += "<p><i>" + authorName + "</i> <a href=\"mailto:" + email + "\">" + email + "</a></p>\n";
            }
        }

        unsigned int downloads = index.data(ItemsModel::kDownloads).toUInt();
        text += downloads == 0 ? i18n("<p>No Downloads</p>") : i18n("<p>Downloads: %1</p>\n", downloads);

		text += "</body></html>";
        text.replace("[b]", "<b>");
        text.replace("[/b]", "</b>");
        text.replace("[i]", "<i>");
        text.replace("[/i]", "</i>");
        text.replace("[u]", "<i>");
        text.replace("[/u]", "</i>");
        text.remove("[url]");
        text.remove("[/url]");
        text.replace("\\\'", "\'");
        infoLabel->setText(text.simplified());
    }

    QToolButton * button = qobject_cast<QToolButton*>(widgets.at(kInstall));
    if (button != NULL) {
        Entry::Status status = Entry::Status(model->data(index, ItemsModel::kStatus).toUInt());
        //if (!button->menu()) {
        //    button->setMenu(InstallMenu(button, status));
        //    button->setIconSize(QSize(16, 16));
            button->resize(size);
        //}
        button->move(right - button->width() - margin, option.rect.height() / 2 - button->height());
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        //button->setPopupMode(QToolButton::MenuButtonPopup);

        // validate our assumptions
        //Q_ASSERT(button->menu());
        //Q_ASSERT(button->menu()->actions().count() == 2);

        // get the two actions
        //QAction * action_install = button->menu()->actions()[0];
        //QAction * action_uninstall = button->menu()->actions()[1];
        switch (status) {
        case Entry::Installed:
            button->setText(i18n("Uninstall"));
            //action_install->setVisible(false);
            //action_uninstall->setVisible(true);
            button->setIcon(QIcon(m_statusicons[Entry::Deleted]));
            break;
        case Entry::Updateable:
            button->setText(i18n("Update"));
            //action_uninstall->setVisible(false);
            //action_install->setText(i18n("Update"));
            //action_install->setVisible(true);
            //action_install->setIcon(QIcon(m_statusicons[Entry::Updateable]));
            button->setIcon(QIcon(m_statusicons[Entry::Updateable]));
            break;
        case Entry::Deleted:
            /// @todo Set different button text when string freeze is over? "Install again"
            button->setText(i18n("Install"));
            //action_uninstall->setVisible(false);
            //action_install->setText(i18n("Install"));
            //action_install->setVisible(true);
            //action_install->setIcon(QIcon(m_statusicons[Entry::Installed]));
            button->setIcon(QIcon(m_statusicons[Entry::Installed]));
            break;
        default:
            button->setText(i18n("Install"));
            //action_uninstall->setVisible(false);
            //action_install->setVisible(true);
            //action_install->setIcon(QIcon(m_statusicons[Entry::Installed]));
            button->setIcon(QIcon(m_statusicons[Entry::Installed]));
        }
    }

    QLabel * ratingLabel = qobject_cast<QLabel*>(widgets.at(kRating));
    if (ratingLabel != NULL) {
        ratingLabel->setText(i18n("Rating: %1", model->data(index, ItemsModel::kRating).toString()));

        // put the rating label below the install button
        ratingLabel->move(right - button->width() - margin, option.rect.height() / 2 + button->height()/2);
        ratingLabel->resize(size);
    }
}

// draw the entry based on what
// paint the item at index with all it's attributes shown
void ItemsViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    int margin = option.fontMetrics.height() / 2;

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(option.palette.text().color()));
    }

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());

    if (realmodel->hasPreviewImages()) {

        int height = option.rect.height();
        QPoint point(option.rect.left() + margin, option.rect.top() + ((height - kPreviewHeight) / 2));

        if (index.data(ItemsModel::kPreview).toString().isEmpty()) {
            QRect rect(point, QSize(kPreviewWidth, kPreviewHeight));
            painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("No Preview"));
        } else {
            QImage image = index.data(ItemsModel::kPreviewPixmap).value<QImage>();
            if (!image.isNull()) {
                point.setY(option.rect.top() + ((height - image.height()) / 2));
                painter->drawImage(point, image);
                QPoint framePoint(point.x() - 5, point.y() - 5);
                painter->drawImage(framePoint, m_frameImage.scaled(image.width() + 10, image.height() + 10));
            } else {
                QRect rect(point, QSize(kPreviewWidth, kPreviewHeight));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }
    }

    painter->restore();
}

//bool ItemsViewDelegate::eventFilter(QObject *watched, QEvent *event)
//{
//    if (event->type() == QEvent::ToolTip) {
//
//    }

//    return KWidgetItemDelegate::eventFilter(watched, event);
//}

QSize ItemsViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(option.fontMetrics.height() * 4);
    size.setHeight(qMax(option.fontMetrics.height() * 7, kPreviewHeight)); // up to 6 lines of text, and two margins

    return size;
}

void ItemsViewDelegate::slotLinkClicked(const QString & url)
{
    Q_UNUSED(url)
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
    KNS::Entry * entry = realmodel->entryForIndex(model->mapToSource(index));
    emit performAction(DownloadDialog::kContactEmail, entry);
}

void ItemsViewDelegate::slotActionTriggered(QAction *action)
{
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
    KNS::Entry * entry = realmodel->entryForIndex(model->mapToSource(index));
    emit performAction(DownloadDialog::EntryAction(action->data().toInt()), entry);
}

void ItemsViewDelegate::slotInstallClicked()
{
    QModelIndex index = focusedIndex();

    if (index.isValid()) {
        const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
        const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
        KNS::Entry * entry = realmodel->entryForIndex(model->mapToSource(index));
        if ( !entry )
            return;

        if (entry->status() == Entry::Installed) {
            emit performAction(DownloadDialog::kUninstall, entry);
        } else {
            emit performAction(DownloadDialog::kInstall, entry);
        }
    }
}
}
