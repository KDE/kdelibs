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

KMenu * ItemsViewDelegate::InstallMenu(const QToolButton* button, Entry::Status status) const
{
    Q_UNUSED(button)
    KMenu * installMenu = new KMenu(NULL);
    QAction * action_install = installMenu->addAction(m_statusicons[Entry::Installed], i18n("Install"));
    QAction * action_uninstall = installMenu->addAction(m_statusicons[Entry::Deleted], i18n("Uninstall"));
    action_install->setData(Engine::Install);
    action_uninstall->setData(Engine::Uninstall);

    action_install->setVisible(status != Entry::Installed);
    action_uninstall->setVisible(status == Entry::Installed);
    return installMenu;
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

    QToolButton * installButton = new QToolButton();
    list << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, SIGNAL(triggered(QAction *)), this, SLOT(slotActionTriggered(QAction *)));
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

    QSize size(option.fontMetrics.height() * 7, widgets.at(DelegateInstallButton)->sizeHint().height());

    QLabel * infoLabel = qobject_cast<QLabel*>(widgets.at(DelegateLabel));
    infoLabel->setWordWrap(true);
    if (infoLabel != NULL) {
        if (realmodel->hasPreviewImages()) {
            // move the text right by kPreviewWidth + margin pixels to fit the preview
            infoLabel->move(PreviewWidth + margin * 2, 0);
            infoLabel->resize(QSize(option.rect.width() - PreviewWidth - (margin * 6) - size.width(), option.fontMetrics.height() * 7));
            
        } else {
            infoLabel->move(margin, 0);
            infoLabel->resize(QSize(option.rect.width() - (margin * 4) - size.width(), option.fontMetrics.height() * 7));
        }

        QString text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; margin:0 0 0 0;}\n"
            "</style></head><body><p><b>" +
            index.data(ItemsModel::kNameRole).toString() + "</b></p>\n";

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

        KUrl link = qvariant_cast<KUrl>(index.data(ItemsModel::kHomepage));
        if (!link.isEmpty()) {
            text += "<p><a href=\"" + link.url() + "\">" + i18nc("Link giving a detailed description for a Hot New Stuff item", "Visit Homepage") + "</a></p>\n";
        }

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

    QToolButton * button = qobject_cast<QToolButton*>(widgets.at(DelegateInstallButton));
    if (button != NULL) {
        Entry::Status status = Entry::Status(model->data(index, ItemsModel::kStatus).toUInt());
        //if (!button->menu()) {
        //    button->setMenu(InstallMenu(button, status));
        //    button->setIconSize(QSize(16, 16));
            button->resize(size);
        //}
        button->move(right - button->width() - margin, option.rect.height()/2 - button->height()*1.5);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        //button->setPopupMode(QToolButton::MenuButtonPopup);

        // validate our assumptions
        //Q_ASSERT(button->menu());
        //Q_ASSERT(button->menu()->actions().count() == 2);

        button->setEnabled(true);
        
        switch (status) {
        case Entry::Installed:
            button->setText(i18n("Uninstall"));
            button->setIcon(QIcon(m_statusicons[Entry::Deleted]));
            break;
        case Entry::Updateable:
            button->setText(i18n("Update"));
            button->setIcon(QIcon(m_statusicons[Entry::Updateable]));
            break;
        case Entry::Deleted:
            button->setText(i18n("Install again"));
            button->setIcon(QIcon(m_statusicons[Entry::Installed]));
            break;
        case Entry::Installing:
            button->setText(i18n("Installing"));
            button->setEnabled(false);
            button->setIcon(QIcon(m_statusicons[Entry::Updateable]));
            break;
        case Entry::Updating:
            button->setText(i18n("Updating"));
            button->setEnabled(false);
            button->setIcon(QIcon(m_statusicons[Entry::Updateable]));
            break;
        default:
            button->setText(i18n("Install"));
            button->setIcon(QIcon(m_statusicons[Entry::Installed]));
        }
    }

    KPushButton* detailsButton = qobject_cast<KPushButton*>(widgets.at(DelegateDetailsButton));
    if (detailsButton) {
        detailsButton->setText(i18n("Details..."));
        detailsButton->move(right - button->width() - margin, option.rect.height()/2 - button->height()/2);
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
        rating->move(right - button->width() - margin, option.rect.height()/2 + button->height()/2);
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

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());

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

        const QSortFilterProxyModel* model = qobject_cast<const QSortFilterProxyModel*>(index.model());
        const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
        KNS3::Entry entry = realmodel->entryForIndex(model->mapToSource(index));      

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

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
    KNS3::Entry entry = realmodel->entryForIndex(model->mapToSource(index));
    emit performAction(Engine::ContactEmail, entry);
}

void ItemsViewDelegate::slotActionTriggered(QAction *action)
{
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
    KNS3::Entry entry = realmodel->entryForIndex(model->mapToSource(index));
    emit performAction(Engine::EntryAction(action->data().toInt()), entry);
}

void ItemsViewDelegate::slotInstallClicked()
{
    QModelIndex index = focusedIndex();
kDebug() << index;
    if (index.isValid()) {
        const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
        kDebug() << model;
        const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
        kDebug() << realmodel;
        KNS3::Entry entry = realmodel->entryForIndex(model->mapToSource(index));
        if ( !entry.isValid() )
            return;

        if (entry.status() == Entry::Installed) {
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
        //const QSortFilterProxyModel * model = qobject_cast<const QSortFilterProxyModel*>(index.model());
        //const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(model->sourceModel());
        const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(index.model());
        kDebug() << realmodel;
        //KNS3::Entry entry = realmodel->entryForIndex(model->mapToSource(index));
        KNS3::Entry entry = realmodel->entryForIndex(index);
        if ( !entry.isValid() )
            return;

        emit performAction(Engine::ShowDetails, entry);
    }
}

}
