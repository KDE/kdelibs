/**
  * This file is part of the KDE project
  * Copyright (C) 2006 Rafael Fernández López <ereslibre@gmail.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include <QApplication>
#include <QPushButton>
#include <QPainter>
#include <QStyleOptionProgressBarV2>
#include <QStyleOptionButton>
#include <QHash>
#include <QVariant>
#include <QFontMetrics>
#include <QListView>

#include <kdebug.h>
#include <kwin.h>
#include <kicon.h>
#include <klocale.h>
#include <kiconloader.h>

#include "progresslistdelegate.h"
#include "progresslistmodel.h"

class ProgressListDelegate::Private
{
public:
    Private(QListView *listView);

    int getJobId(const QModelIndex &index) const;
    QString getApplicationInternalName(const QModelIndex &index) const;
    QString getApplicationName(const QModelIndex &index) const;
    QString getIcon(const QModelIndex &index) const;
    qlonglong getFileTotals(const QModelIndex &index) const;
    qlonglong getFilesProcessed(const QModelIndex &index) const;
    QString getSizeTotals(const QModelIndex &index) const;
    QString getSizeProcessed(const QModelIndex &index) const;
    qlonglong getTimeTotals(const QModelIndex &index) const;
    qlonglong getTimeProcessed(const QModelIndex &index) const;
    QString getFromLabel(const QModelIndex &index) const;
    QString getFrom(const QModelIndex &index) const;
    QString getToLabel(const QModelIndex &index) const;
    QString getTo(const QModelIndex &index) const;
    int getPercent(const QModelIndex &index) const;
    QString getMessage(const QModelIndex &index) const;
    const QList<actionInfo> &getActionList(const QModelIndex &index) const;
    QStyleOptionProgressBarV2 *getProgressBar(const QModelIndex &index) const;
    int getCurrentLeftMargin(int fontHeight) const;

    int separatorPixels;
    int leftMargin;
    int rightMargin;
    int progressBarHeight;
    int minimumItemHeight;
    int minimumContentWidth;
    int editorHeight;
    int iconWidth;
    QListView *listView;
};

ProgressListDelegate::Private::Private(QListView *listView)
{
    this->listView = listView;
}

int ProgressListDelegate::Private::getJobId(const QModelIndex &index) const
{
    return index.model()->data(index, jobId).toInt();
}

QString ProgressListDelegate::Private::getApplicationInternalName(const QModelIndex &index) const
{
    return index.model()->data(index, applicationInternalName).toString();
}

QString ProgressListDelegate::Private::getApplicationName(const QModelIndex &index) const
{
    return index.model()->data(index, applicationName).toString();
}

QString ProgressListDelegate::Private::getIcon(const QModelIndex &index) const
{
    return index.model()->data(index, icon).toString();
}

qlonglong ProgressListDelegate::Private::getFileTotals(const QModelIndex &index) const
{
    return index.model()->data(index, fileTotals).toLongLong();
}

qlonglong ProgressListDelegate::Private::getFilesProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, filesProcessed).toLongLong();
}

QString ProgressListDelegate::Private::getSizeTotals(const QModelIndex &index) const
{
    return index.model()->data(index, sizeTotals).toString();
}

QString ProgressListDelegate::Private::getSizeProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, sizeProcessed).toString();
}

qlonglong ProgressListDelegate::Private::getTimeTotals(const QModelIndex &index) const
{
    return index.model()->data(index, timeTotals).toLongLong();
}

qlonglong ProgressListDelegate::Private::getTimeProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, timeElapsed).toLongLong();
}

QString ProgressListDelegate::Private::getFromLabel(const QModelIndex &index) const
{
    return index.model()->data(index, fromLabel).toString();
}

QString ProgressListDelegate::Private::getFrom(const QModelIndex &index) const
{
    return index.model()->data(index, from).toString();
}

QString ProgressListDelegate::Private::getToLabel(const QModelIndex &index) const
{
    return index.model()->data(index, toLabel).toString();
}

QString ProgressListDelegate::Private::getTo(const QModelIndex &index) const
{
    return index.model()->data(index, to).toString();
}

int ProgressListDelegate::Private::getPercent(const QModelIndex &index) const
{
    return index.model()->data(index, percent).toInt();
}

QString ProgressListDelegate::Private::getMessage(const QModelIndex &index) const
{
    return index.model()->data(index, message).toString();
}

const QList<actionInfo> &ProgressListDelegate::Private::getActionList(const QModelIndex &index) const
{
    const ProgressListModel *progressListModel = static_cast<const ProgressListModel*>(index.model());

    return progressListModel->actions(getJobId(index));
}

QStyleOptionProgressBarV2 *ProgressListDelegate::Private::getProgressBar(const QModelIndex &index) const
{
    const ProgressListModel *progressListModel = static_cast<const ProgressListModel*>(index.model());

    return progressListModel->progressBar(index);
}

int ProgressListDelegate::Private::getCurrentLeftMargin(int fontHeight) const
{
    return leftMargin + separatorPixels + fontHeight;
}

ProgressListDelegate::ProgressListDelegate(QObject *parent, QListView *listView)
    : QItemDelegate(parent)
    , d(new Private(listView))
{
}

ProgressListDelegate::~ProgressListDelegate()
{
    delete d;
}

QWidget *ProgressListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    #warning implement (ereslibre)

    return 0;
}

bool ProgressListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                       const QStyleOptionViewItem &option, const QModelIndex &index)
{
    d->listView->repaint();

    return this->event(event);
}


void ProgressListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fontMetrics = painter->fontMetrics();
    int textHeight = fontMetrics.height();

    int coordY = d->separatorPixels;

    int jobIdModel = d->getJobId(index);
    QString applicationInternalNameModel = d->getApplicationInternalName(index);
    QString applicationNameModel = d->getApplicationName(index);
    QString iconModel = d->getIcon(index);
    qlonglong fileTotalsModel = d->getFileTotals(index);
    qlonglong filesProcessedModel = d->getFilesProcessed(index);
    QString sizeTotalsModel = d->getSizeTotals(index);
    QString sizeProcessedModel = d->getSizeProcessed(index);
    qlonglong timeTotalsModel = d->getTimeTotals(index);
    qlonglong timeProcessedModel = d->getTimeProcessed(index);
    QString fromModel = d->getFromLabel(index) + d->getFrom(index);
    QString toModel = d->getToLabel(index) + d->getTo(index);
    int percentModel = d->getPercent(index);
    QString messageModel = d->getMessage(index);
    const QList<actionInfo> actionInfoList = d->getActionList(index);

    KIconLoader *iconLoader = static_cast<KIconLoader*>(index.internalPointer());
    KIcon iconToShow(iconModel, iconLoader);

    QColor unselectedTextColor = option.palette.text().color();
    QColor selectedTextColor = option.palette.highlightedText().color();
    QPen currentPen = painter->pen();
    QPen unselectedPen = QPen(currentPen);
    QPen selectedPen = QPen(currentPen);

    unselectedPen.setColor(unselectedTextColor);
    selectedPen.setColor(selectedTextColor);

    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, option.palette.highlight());
        painter->setPen(selectedPen);
    }
    else
    {
        painter->setPen(unselectedPen);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QRect canvas = option.rect;
    int iconWidth = canvas.height() - d->leftMargin - d->rightMargin;
    int iconHeight = iconWidth;
    d->iconWidth = iconWidth;

    painter->setOpacity(0.25);

    painter->drawPixmap(option.rect.right() - iconWidth - d->rightMargin, canvas.top() + coordY, iconToShow.pixmap(iconWidth, iconHeight));

    painter->translate(d->leftMargin, d->separatorPixels + (fontMetrics.width(applicationNameModel) / 2) + (iconHeight / 2) + canvas.top());
    painter->rotate(270);

    QRect appNameRect(0, 0, fontMetrics.width(applicationNameModel), fontMetrics.height());

    painter->drawText(appNameRect, Qt::AlignLeft, applicationNameModel);

    painter->resetMatrix();

    painter->setOpacity(1);

    if (!d->getMessage(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(d->getMessage(index), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin);

        painter->drawText(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getFrom(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1: %2", d->getFromLabel(index), d->getFrom(index)), Qt::ElideMiddle, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin);

        painter->drawText(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getTo(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1: %2", d->getToLabel(index), d->getTo(index)), Qt::ElideMiddle, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin);

        painter->drawText(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (d->getFileTotals(index) > 1)
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1 of %2 files processed", QString::number(d->getFilesProcessed(index)), QString::number(d->getFileTotals(index))), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin);

        painter->drawText(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getSizeTotals(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1 of %2 processed", d->getSizeProcessed(index), d->getSizeTotals(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin);

        painter->drawText(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (d->getPercent(index) > -1)
    {
        QStyleOptionProgressBarV2 *progressBarModel = d->getProgressBar(index);

        progressBarModel->rect = QRect(d->getCurrentLeftMargin(fontMetrics.height()), canvas.top() + coordY, canvas.width() - d->getCurrentLeftMargin(fontMetrics.height()) - d->rightMargin - iconWidth - d->separatorPixels, d->progressBarHeight);

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, progressBarModel, painter);
    }

    painter->restore();
}

void ProgressListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    #warning implement me (ereslibre)

    return;
}

void ProgressListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    #warning implement me (ereslibre)

    return;
}

QSize ProgressListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fontMetrics = option.fontMetrics;

    int itemHeight = d->separatorPixels;
    int itemWidth = qMax(option.rect.width(), d->minimumContentWidth);

    int textSize = fontMetrics.height() + d->separatorPixels;

    if (!d->getMessage(index).isEmpty())
        itemHeight += textSize;

    if (!d->getFrom(index).isEmpty())
        itemHeight += textSize;

    if (!d->getTo(index).isEmpty())
        itemHeight += textSize;

    if (d->getFileTotals(index) > 1)
        itemHeight += textSize;

    if (!d->getSizeTotals(index).isEmpty())
        itemHeight += textSize;

    if (d->getPercent(index) > -1)
        itemHeight += d->progressBarHeight + d->separatorPixels;

    if (d->editorHeight)
        itemHeight += d->editorHeight + d->separatorPixels;

    if (itemHeight + d->separatorPixels >= d->minimumItemHeight)
        itemHeight += d->separatorPixels;
    else
        itemHeight = d->minimumItemHeight;

    return QSize(itemWidth, itemHeight);
}

void ProgressListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const
{
    QRect editorRect(d->getCurrentLeftMargin(option.fontMetrics.height()), option.rect.height() - d->separatorPixels - d->editorHeight + option.rect.top(), option.rect.width() - d->getCurrentLeftMargin(option.fontMetrics.height()) - d->rightMargin - d->separatorPixels - d->iconWidth, d->editorHeight);

    editor->setGeometry(editorRect);
}

void ProgressListDelegate::setSeparatorPixels(int separatorPixels)
{
    d->separatorPixels = separatorPixels;
}

void ProgressListDelegate::setLeftMargin(int leftMargin)
{
    d->leftMargin = leftMargin;
}

void ProgressListDelegate::setRightMargin(int rightMargin)
{
    d->rightMargin = rightMargin;
}

void ProgressListDelegate::setProgressBarHeight(int progressBarHeight)
{
    d->progressBarHeight = progressBarHeight;
}

void ProgressListDelegate::setMinimumItemHeight(int minimumItemHeight)
{
    d->minimumItemHeight = minimumItemHeight;
}

void ProgressListDelegate::setMinimumContentWidth(int minimumContentWidth)
{
    d->minimumContentWidth = minimumContentWidth;
}

void ProgressListDelegate::setEditorHeight(int editorHeight)
{
    d->editorHeight = editorHeight;
}

#include "progresslistdelegate.moc"
