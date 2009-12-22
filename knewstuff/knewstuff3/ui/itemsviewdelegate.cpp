/*
    This file is part of KNewStuff2.
    Copyright (C) 2008 Jeremy Whiting <jeremy@scitools.com>

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

#include <QtGui/QPainter>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QApplication>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kicon.h>
#include <klocale.h>
#include <kmenu.h>
#include <krun.h>
#include <kpushbutton.h>

#include "itemsmodel.h"
#include "ratingwidget.h"
#include "ratingpainter.h"

namespace KNS3
{
    static const int DelegateLabel = 0;
    static const int DelegateInstallButton = 1;
    static const int DelegateDetailsButton = 2;
    static const int DelegateRatingWidget = 3;

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

const ItemsModel* ItemsViewDelegate::modelFromIndex(const QModelIndex& index) const
{
    // FIXME:
    // Different people report either cast to not work. Why would we sometimes get the proxy, sometimes the real thing?
    const QSortFilterProxyModel* proxyModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (proxyModel) {
        kDebug() << "Got a proxy model.";
        return qobject_cast<const ItemsModel*>(proxyModel->sourceModel());
    } else {
        kDebug() << "Got the base model.";
        return qobject_cast<const ItemsModel*>(index.model());
    }
}

KNS3::EntryInternal ItemsViewDelegate::entryForIndex(const QModelIndex& index) const
{
    const QSortFilterProxyModel* proxyModel = qobject_cast<const QSortFilterProxyModel*>(index.model());
    if (proxyModel) {
        kDebug() << "Got a proxy model.";
        const ItemsModel* model = qobject_cast<const ItemsModel*>(proxyModel->sourceModel());
        return model->entryForIndex(proxyModel->mapToSource(index));
    } else {
        kDebug() << "Got the base model.";
        const ItemsModel* model = qobject_cast<const ItemsModel*>(index.model());
        return model->entryForIndex(index);
    }
}

QList<QWidget*> ItemsViewDelegate::createItemWidgets() const
{
    QList<QWidget*> list;

    QLabel * infoLabel = new QLabel();
    infoLabel->setOpenExternalLinks(true);
    // not so nice - work around constness to install the event filter
    ItemsViewDelegate* delegate = const_cast<ItemsViewDelegate*>(this);
    infoLabel->installEventFilter(delegate);
    list << infoLabel;

    KPushButton * installButton = new KPushButton();
    list << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));

    KPushButton* detailsButton = new KPushButton();
    list << detailsButton;
    setBlockedEventTypes(detailsButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(detailsButton, SIGNAL(clicked()), this, SLOT(slotDetailsClicked()));

    RatingWidget* rating = new RatingWidget();
    rating->setMaxRating(10);
    rating->setHalfStepsEnabled(true);
    rating->setEditable(false);
    list << rating;

    return list;
}

void ItemsViewDelegate::updateItemWidgets(const QList<QWidget*> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    const ItemsModel * model = modelFromIndex(index);
    if (!model) {
        kDebug() << "WARNING - INVALID MODEL!";
        return;
    }

    // setup the install button
    int margin = option.fontMetrics.height() / 2;

    int right = option.rect.width();
    //int bottom = option.rect.height();

    QSize size(option.fontMetrics.height() * 7, widgets.at(DelegateInstallButton)->sizeHint().height());

    QLabel * infoLabel = qobject_cast<QLabel*>(widgets.at(DelegateLabel));
    infoLabel->setWordWrap(true);
    if (infoLabel != NULL) {
        if (model->hasPreviewImages()) {
            // move the text right by kPreviewWidth + margin pixels to fit the preview
            infoLabel->move(PreviewWidth + margin * 2, 0);
            infoLabel->resize(QSize(option.rect.width() - PreviewWidth - (margin * 6) - size.width(), option.fontMetrics.height() * 7));

        } else {
            infoLabel->move(margin, 0);
            infoLabel->resize(QSize(option.rect.width() - (margin * 4) - size.width(), option.fontMetrics.height() * 7));
        }

        QString text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; margin:0 0 0 0;}\n"
            "</style></head><body><p><b>";

        KUrl link = qvariant_cast<KUrl>(index.data(ItemsModel::kHomepage));
        if (!link.isEmpty()) {
            text += "<p><a href=\"" + link.url() + "\">" + index.data(ItemsModel::kNameRole).toString() + "</a></p>\n";
        } else {
            text += index.data(ItemsModel::kNameRole).toString();
        }

        text += "</b></p>\n";

        QString authorName = index.data(ItemsModel::kAuthorName).toString();
        QString email = index.data(ItemsModel::kAuthorEmail).toString();
        if (!authorName.isEmpty()) {
            if (email.isEmpty()) {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + "</p>\n";
            } else {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + " <a href=\"mailto:" + email + "\">" + email + "</a></p>\n";
            }
        }

        QString summary = "<p>" + option.fontMetrics.elidedText(index.data(ItemsModel::kSummary).toString(),
            Qt::ElideRight, infoLabel->width() * 3) + "</p>\n";
        text += summary;



        unsigned int downloads = index.data(ItemsModel::kDownloads).toUInt();
        if (downloads > 0) {
            text += i18n("<p>Downloads: %1</p>\n", downloads);
        }

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

    KPushButton * installButton = qobject_cast<KPushButton*>(widgets.at(DelegateInstallButton));
    if (installButton != 0) {
        EntryInternal::Status status = EntryInternal::Status(model->data(index, ItemsModel::kStatus).toUInt());
        installButton->resize(size);
        installButton->move(right - installButton->width() - margin, option.rect.height()/2 - installButton->height()*1.5);

        switch (status) {
            case EntryInternal::Installed:
            installButton->setText(i18n("Uninstall"));
            installButton->setEnabled(true);
            installButton->setIcon(m_statusicons[EntryInternal::Deleted]);
            break;
        case EntryInternal::Updateable:
            installButton->setText(i18n("Update"));
            installButton->setEnabled(true);
            installButton->setIcon(m_statusicons[EntryInternal::Updateable]);
            break;
        case EntryInternal::Deleted:
            installButton->setText(i18n("Install again"));
            installButton->setEnabled(true);
            installButton->setIcon(m_statusicons[EntryInternal::Installed]);
            break;
        case EntryInternal::Installing:
            installButton->setText(i18n("Installing"));
            installButton->setEnabled(false);
            installButton->setIcon(m_statusicons[EntryInternal::Updateable]);
            break;
        case EntryInternal::Updating:
            installButton->setText(i18n("Updating"));
            installButton->setEnabled(false);
            installButton->setIcon(m_statusicons[EntryInternal::Updateable]);
            break;
        default:
            installButton->setText(i18n("Install"));
            installButton->setEnabled(true);
            installButton->setIcon(m_statusicons[EntryInternal::Installed]);
        }
    }

    KPushButton* detailsButton = qobject_cast<KPushButton*>(widgets.at(DelegateDetailsButton));
    if (detailsButton) {
        detailsButton->setText(i18n("Details..."));
        detailsButton->move(right - installButton->width() - margin, option.rect.height()/2 - installButton->height()/2);
        detailsButton->resize(size);
    }

    RatingWidget * rating = qobject_cast<RatingWidget*>(widgets.at(DelegateRatingWidget));
    if (rating) {
        rating->setToolTip(i18n("Rating: %1%", model->data(index, ItemsModel::kRating).toString()));
        // assume all entries come with rating 0..100 but most are in the range 20 - 80, so 20 is 0 stars, 80 is 5 stars
        int ratingValue = model->data(index, ItemsModel::kRating).toInt();
        if (ratingValue <= 0) {
            rating->setVisible(false);
        }
        rating->setRating((ratingValue-20)*10/60);
        // put the rating label below the install button
        rating->move(right - installButton->width() - margin, option.rect.height()/2 + installButton->height()/2);
        rating->resize(size);
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

    const ItemsModel * realmodel = modelFromIndex(index);

    if (realmodel->hasPreviewImages()) {
        int height = option.rect.height();
        QPoint point(option.rect.left() + margin, option.rect.top() + ((height - PreviewHeight) / 2));

        if (index.data(ItemsModel::kPreview).toString().isEmpty()) {
            QRect rect(point, QSize(PreviewWidth, PreviewHeight));
            painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("No Preview"));
        } else {
            QImage image = index.data(ItemsModel::kPreviewPixmap).value<QImage>();
            if (!image.isNull()) {
                point.setY(option.rect.top() + ((height - image.height()) / 2));
                painter->drawImage(point, image);
                QPoint framePoint(point.x() - 5, point.y() - 5);
                painter->drawImage(framePoint, m_frameImage.scaled(image.width() + 10, image.height() + 10));
            } else {
                QRect rect(point, QSize(PreviewWidth, PreviewHeight));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }
    }

    // TODO: get the right rect and use the painter? QRect rect(100, 100, 200, 120);
    //KRatingPainter::paintRating(painter, rect, Qt::AlignLeft, index.data(ItemsModel::kRating).toInt());
    painter->restore();
}

bool ItemsViewDelegate::eventFilter(QObject *watched, QEvent *event)
{
   if (event->type() == QEvent::MouseButtonDblClick) {
        QModelIndex index = focusedIndex();
        Q_ASSERT(index.isValid());

        KNS3::EntryInternal entry = entryForIndex(index);

        performAction(Engine::ShowDetails, entry);
   }

   return KWidgetItemDelegate::eventFilter(watched, event);
}

QSize ItemsViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(option.fontMetrics.height() * 4);
    size.setHeight(qMax(option.fontMetrics.height() * 7, PreviewHeight)); // up to 6 lines of text, and two margins
    return size;
}

void ItemsViewDelegate::slotLinkClicked(const QString & url)
{
    Q_UNUSED(url)
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    KNS3::EntryInternal entry = entryForIndex(index);
    emit performAction(Engine::ContactEmail, entry);
}

void ItemsViewDelegate::slotInstallClicked()
{
    QModelIndex index = focusedIndex();
kDebug() << index;
    if (index.isValid()) {

        const ItemsModel * model = modelFromIndex(index);
        kDebug() << model;
        KNS3::EntryInternal entry = model->entryForIndex(index);
        if ( !entry.isValid() )
            return;

        if (entry.status() == EntryInternal::Installed) {
            emit performAction(Engine::Uninstall, entry);
        } else {
            emit performAction(Engine::Install, entry);
        }
    }
}

void ItemsViewDelegate::slotDetailsClicked()
{
    QModelIndex index = focusedIndex();
    kDebug() << index;

    if (index.isValid()) {        
        const ItemsModel * realmodel = modelFromIndex(index);
        kDebug() << realmodel;
        //KNS3::EntryInternal entry = realmodel->entryForIndex(model->mapToSource(index));
        KNS3::EntryInternal entry = realmodel->entryForIndex(index);
        if ( !entry.isValid() )
            return;

        emit performAction(Engine::ShowDetails, entry);
    }
}

}
