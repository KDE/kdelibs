/*
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "itemsgridviewdelegate.h"

#include <QtGui/QPainter>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QApplication>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <KDebug>
#include <KLocale>
#include <KMenu>

#include "itemsmodel.h"
#include "ratingwidget.h"
#include "ratingpainter.h"

namespace KNS3
{
    static const int DelegateTitleLabel = 0;
    static const int DelegateAuthorLabel = 1;
    static const int DelegateDownloadCounterLabel = 2;
    static const int DelegateInstallButton = 3;
    static const int DelegateRatingWidget = 4;
    //optional maybe not sure for now
    static const int DelegateDetailsButton = 5;
    static const int DelegateDescriptionLabel = 6;
    
ItemsGridViewDelegate::ItemsGridViewDelegate(QAbstractItemView *itemView, Engine* engine, QObject * parent)
        : ItemsViewBaseDelegate(itemView, engine, parent)
{
}

ItemsGridViewDelegate::~ItemsGridViewDelegate()
{
}

QList<QWidget*> ItemsGridViewDelegate::createItemWidgets() const
{
    QList<QWidget*> m_widgetList;
    QLabel * titleLabel = new QLabel();
    titleLabel->setOpenExternalLinks(true);
    // not so nice - work around constness to install the event filter
    ItemsGridViewDelegate* delegate = const_cast<ItemsGridViewDelegate*>(this);
    titleLabel->installEventFilter(delegate);
    m_widgetList << titleLabel;

    QLabel * authorLabel = new QLabel();
    m_widgetList << authorLabel;
    
    QLabel * downloadCounterLabel = new QLabel();
    m_widgetList << downloadCounterLabel;
    
    QToolButton * installButton = new QToolButton();
    installButton->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    installButton->setPopupMode(QToolButton::InstantPopup);
    m_widgetList << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, SIGNAL(clicked()), this, SLOT(slotInstallClicked()));
    connect(installButton, SIGNAL(triggered(QAction *)), this, SLOT(slotInstallActionTriggered(QAction *)));

    RatingWidget* rating = new RatingWidget();
    rating->setMaxRating(10);
    rating->setHalfStepsEnabled(true);
    rating->setEditable(false);
    m_widgetList << rating;
    
    /*KPushButton* detailsButton = new KPushButton();
    m_widgetList << detailsButton;
    setBlockedEventTypes(detailsButton, QList<QEvent::Type>() << QEvent::MouseButtonPress
                         << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(detailsButton, SIGNAL(clicked()), this, SLOT(slotDetailsClicked()));
*/
    return m_widgetList;
}

void ItemsGridViewDelegate::updateItemWidgets(const QList<QWidget*> widgets,
        const QStyleOptionViewItem &option,
        const QPersistentModelIndex &index) const
{
    const ItemsModel * model = qobject_cast<const ItemsModel*>(index.model());
    if (!model) {
        kDebug() << "WARNING - INVALID MODEL!";
        return;
    }

    EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
    int margin = option.fontMetrics.height() / 2;
    int elementYPos = PreviewHeight + margin + frameThickness*2;
    
    //setup rating widget
    RatingWidget * rating = qobject_cast<RatingWidget*>(widgets.at(DelegateRatingWidget));
    if (rating) {
        if (entry.rating() > 0) {
            rating->setToolTip(i18n("Rating: %1%", entry.rating()));
            // assume all entries come with rating 0..100 but most are in the range 20 - 80, so 20 is 0 stars, 80 is 5 stars
            rating->setRating((entry.rating()-20)*10/60);
            //make the rating widget smaller than the one at list view
            int newWidth = 68;
            QSize size(newWidth, 15);
            rating->resize(size);
            //put rating widget under image rectangle
            rating->move((ItemGridWidth-newWidth)/2, elementYPos);
            elementYPos += rating->height();
        } else {
            //is it better to stay visible?
            rating->setVisible(false);
        }
    }
    elementYPos += margin;
    //setup title label
    QLabel * titleLabel = qobject_cast<QLabel*>(widgets.at(DelegateTitleLabel));
    if (titleLabel != NULL) {
        titleLabel->setWordWrap(true);
        titleLabel->setAlignment(Qt::AlignHCenter);
        //titleLabel->setFrameStyle(QFrame::Panel);
        titleLabel->resize(QSize(option.rect.width() - (margin * 2), option.fontMetrics.height() * 2));
        titleLabel->move((ItemGridWidth-titleLabel->width())/2,elementYPos);
        
        QString title;
        KUrl link = qvariant_cast<KUrl>(entry.homepage());
        if (!link.isEmpty()) {
            title += "<b><a href=\"" + link.url() + "\">" + entry.name() + "</a></b>\n";
        } else {
            title += "<b>" + entry.name() + "</b>";
        }
        titleLabel->setText(title);
        elementYPos += titleLabel->height();
    }
    //setup author label
    QLabel * authorLabel = qobject_cast<QLabel*>(widgets.at(DelegateAuthorLabel));
    if (authorLabel != NULL) {
        authorLabel->setWordWrap(true);
        authorLabel->setAlignment(Qt::AlignHCenter);
        authorLabel->resize(QSize(option.rect.width() - (margin * 2), option.fontMetrics.height()));
        authorLabel->move((ItemGridWidth-authorLabel->width())/2,elementYPos);
        
        QString text;
        QString authorName = entry.author().name();
        QString email = entry.author().email();
        QString authorPage = entry.author().homepage();

        if (!authorName.isEmpty()) {
            if (!authorPage.isEmpty()) {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", " <a href=\"" + authorPage + "\">" + authorName + "</a></p>\n");
            } else if (!email.isEmpty()) {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + " <a href=\"mailto:" + email + "\">" + email + "</a></p>\n";
            } else {
                text += "<p>" + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + "</p>\n";
            }
        }
        authorLabel->setText(text);
        elementYPos += authorLabel->height();
    }
    elementYPos += margin;
    //setup download label
    QLabel * downloadLabel = qobject_cast<QLabel*>(widgets.at(DelegateDownloadCounterLabel));
    if (downloadLabel != NULL) {
        downloadLabel->setWordWrap(true);
        downloadLabel->setAlignment(Qt::AlignHCenter);
        downloadLabel->resize(QSize(option.rect.width() - (margin * 2), option.fontMetrics.height()));
        downloadLabel->move((ItemGridWidth-downloadLabel->width())/2,elementYPos);
        
        unsigned int fans = entry.numberFans();
        unsigned int downloads = entry.downloadCount();
        
        QString text;
        QString fanString;
        QString downloadString;
        if (fans > 0) {
            fanString = i18np("1 fan", "%1 fans", fans);
        }
        if (downloads > 0) {
            downloadString = i18np("1 download", "%1 downloads", downloads);
        }
        if (downloads > 0 || fans > 0) {
            text += "<p>" + downloadString;
            if (downloads > 0 && fans > 0) {
                text += ", ";
            }
            text += fanString + QLatin1String("</p>");
        }
        downloadLabel->setText(text);
        elementYPos += downloadLabel->height();
    }
    elementYPos += margin;
    
    QToolButton * installButton = qobject_cast<QToolButton*>(widgets.at(DelegateInstallButton));
    if (installButton != 0) {

        if (installButton->menu()) {
            QMenu* buttonMenu = installButton->menu();
            buttonMenu->clear();
            installButton->setMenu(0);
            buttonMenu->deleteLater();
        }

        bool installable = false;
        bool enabled = true;
        QString text;
        KIcon icon;

        switch (entry.status()) {
        case EntryInternal::Installed:
            text = i18n("Uninstall");
            icon = m_iconDelete;
            break;
        case EntryInternal::Updateable:
            text = i18n("Update");
            icon = m_iconUpdate;
            installable = true;
            break;
        case EntryInternal::Installing:
            text = i18n("Installing");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case EntryInternal::Updating:
            text = i18n("Updating");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case EntryInternal::Downloadable:
            text = i18n("Install");
            icon = m_iconInstall;
            installable = true;
            break;
        case EntryInternal::Deleted:
            text = i18n("Install Again");
            icon = m_iconInstall;
            installable = true;
            break;
        default:
            text = i18n("Install");
        }
        installButton->setText(text);
        installButton->setEnabled(enabled);
        installButton->setIcon(icon);
        if (installable && entry.downloadLinkCount() > 1) {
            KMenu * installMenu = new KMenu(installButton);
            foreach (EntryInternal::DownloadLinkInformation info, entry.downloadLinkInformationList()) {
                QString text = info.name;
                if (!info.distributionType.trimmed().isEmpty()) {
                    text + " (" + info.distributionType.trimmed() + ")";
                }
                QAction* installAction = installMenu->addAction(m_iconInstall, text);
                installAction->setData(info.id);
            }
            installButton->setMenu(installMenu);
        }

    }
}

void ItemsGridViewDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{    
    int margin = option.fontMetrics.height() / 2;

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        kDebug() << "draw selected";
        painter->setPen(QPen(option.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(option.palette.text().color()));
    }

    const ItemsModel * realmodel = qobject_cast<const ItemsModel*>(index.model());

    if (realmodel->hasPreviewImages()) {
        int height = option.rect.height();
        int width = option.rect.width();
        QPoint framePoint(option.rect.left() + ((width - PreviewWidth - frameThickness*2) / 2), option.rect.top() + margin);

        KNS3::EntryInternal entry = index.data(Qt::UserRole).value<KNS3::EntryInternal>();
        if (entry.previewUrl(EntryInternal::PreviewSmall1).isEmpty()) {
            ;
        } else {
            QImage image = entry.previewImage(EntryInternal::PreviewSmall1);
            if (!image.isNull()) {
                //image always at the center of frame
                painter->drawPixmap(framePoint, m_frameImage.scaled(PreviewWidth + 14, PreviewHeight + 14));
                
                QPoint previewPoint(framePoint.x() + 7 + (PreviewWidth - image.width())/2, framePoint.y() + 7 + (PreviewHeight - image.height())/2);
                painter->drawImage(previewPoint, image);                
            } else {
                QRect rect(framePoint, QSize(PreviewWidth + frameThickness*2, PreviewHeight + frameThickness*2));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }
    }

    painter->restore();
}

QSize ItemsGridViewDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(ItemGridWidth);
    size.setHeight(qMax(option.fontMetrics.height() * 17, ItemGridHeight)); // up to 6 lines of text, and two margins
    return size;
}

}

#include "itemsgridviewdelegate.moc"
