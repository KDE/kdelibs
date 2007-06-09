/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@gmail.com>
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
#include <QHash>
#include <QVariant>
#include <QFontMetrics>
#include <QListView>
#include <QBoxLayout>

#include <kdebug.h>
#include <kicon.h>
#include <klocale.h>
#include <kiconloader.h>

#include "progresslistdelegate.h"
#include "progresslistmodel.h"
#include "progresslistdelegate_p.h"

int ProgressListDelegate::Private::getJobId(const QModelIndex &index) const
{
    return index.model()->data(index, JobId).toInt();
}

QString ProgressListDelegate::Private::getApplicationInternalName(const QModelIndex &index) const
{
    return index.model()->data(index, ApplicationInternalName).toString();
}

QString ProgressListDelegate::Private::getApplicationName(const QModelIndex &index) const
{
    return index.model()->data(index, ApplicationName).toString();
}

QString ProgressListDelegate::Private::getIcon(const QModelIndex &index) const
{
    return index.model()->data(index, Icon).toString();
}

qlonglong ProgressListDelegate::Private::getFileTotals(const QModelIndex &index) const
{
    return index.model()->data(index, FileTotals).toLongLong();
}

qlonglong ProgressListDelegate::Private::getFilesProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, FilesProcessed).toLongLong();
}

qlonglong ProgressListDelegate::Private::getDirTotals(const QModelIndex &index) const
{
    return index.model()->data(index, DirTotals).toLongLong();
}

qlonglong ProgressListDelegate::Private::getDirsProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, DirsProcessed).toLongLong();
}

QString ProgressListDelegate::Private::getSizeTotals(const QModelIndex &index) const
{
    return index.model()->data(index, SizeTotals).toString();
}

QString ProgressListDelegate::Private::getSizeProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, SizeProcessed).toString();
}

qlonglong ProgressListDelegate::Private::getTimeTotals(const QModelIndex &index) const
{
    return index.model()->data(index, TimeTotals).toLongLong();
}

qlonglong ProgressListDelegate::Private::getTimeProcessed(const QModelIndex &index) const
{
    return index.model()->data(index, TimeElapsed).toLongLong();
}

QString ProgressListDelegate::Private::getFromLabel(const QModelIndex &index) const
{
    return index.model()->data(index, FromLabel).toString();
}

QString ProgressListDelegate::Private::getFrom(const QModelIndex &index) const
{
    return index.model()->data(index, From).toString();
}

QString ProgressListDelegate::Private::getToLabel(const QModelIndex &index) const
{
    return index.model()->data(index, ToLabel).toString();
}

QString ProgressListDelegate::Private::getTo(const QModelIndex &index) const
{
    return index.model()->data(index, To).toString();
}

QString ProgressListDelegate::Private::getSpeed(const QModelIndex &index) const
{
    return index.model()->data(index, Speed).toString();
}

int ProgressListDelegate::Private::getPercent(const QModelIndex &index) const
{
    return index.model()->data(index, Percent).toInt();
}

QString ProgressListDelegate::Private::getMessage(const QModelIndex &index) const
{
    return index.model()->data(index, Message).toString();
}

QString ProgressListDelegate::Private::getProgressMessage(const QModelIndex &index) const
{
    return index.model()->data(index, ProgressMessage).toString();
}

const QList<ActionInfo> &ProgressListDelegate::Private::getActionList(const QModelIndex &index) const
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

void ProgressListDelegate::Private::actionModified(const QModelIndex &index)
{
    listView->closePersistentEditor(index);
    listView->openPersistentEditor(index);
}

ProgressListDelegate::Private::QActionPushButton::QActionPushButton(int actionId, int jobId, const QString &actionText, QWidget *parent)
    : QPushButton(actionText, parent)
{
    this->actionId = actionId;
    this->jobId = jobId;

    connect(this, SIGNAL(clicked(bool)), this,
            SLOT(buttonPressed()));
}

void ProgressListDelegate::Private::QActionPushButton::buttonPressed()
{
    emit actionButtonPressed(actionId, jobId);
}

ProgressListDelegate::ProgressListDelegate(QObject *parent, QListView *listView)
    : QItemDelegate(parent)
    , d(new Private(parent, listView))
{
}

ProgressListDelegate::~ProgressListDelegate()
{
    delete d;
}

QWidget *ProgressListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const ProgressListModel *progressListModel = static_cast<const ProgressListModel*>(index.model());

    int jobIdModel = index.model()->data(index, JobId).toInt();

    QList<ActionInfo> actionsModel = progressListModel->actions(jobIdModel);

    if (actionsModel.isEmpty())
        return 0;

    QWidget *returnWidget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    returnWidget->setLayout(layout);

    QPushButton *newButton;
    int i = 0;
    foreach (const ActionInfo &actionIt, actionsModel)
    {
        newButton = new Private::QActionPushButton(actionIt.actionId, jobIdModel, actionIt.actionText);

        connect(newButton, SIGNAL(actionButtonPressed(int,int)), this,
                SIGNAL(actionPerformed(int,int)));

        layout->addWidget(newButton);

        if (i < actionsModel.count() - 1)
            layout->addSpacing(d->separatorPixels);

        i++;
    }

    return returnWidget;
}

void ProgressListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setClipping(false); // workaround for qt bug

    QFontMetrics fontMetrics = painter->fontMetrics();
    int textHeight = fontMetrics.height();

    int coordY = d->separatorPixels + option.rect.top();

    KIconLoader *iconLoader = static_cast<KIconLoader*>(index.internalPointer());
    KIcon iconToShow(d->getIcon(index), iconLoader);

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

    painter->drawPixmap(option.rect.right() - iconWidth - d->rightMargin, coordY, iconToShow.pixmap(iconWidth, iconHeight));

    painter->translate(d->leftMargin, d->separatorPixels + (fontMetrics.width(d->getApplicationName(index)) / 2) + (iconHeight / 2) + canvas.top());
    painter->rotate(270);

    QRect appNameRect(0, 0, fontMetrics.width(d->getApplicationName(index)), textHeight);

    painter->drawText(appNameRect, Qt::AlignLeft, d->getApplicationName(index));

    painter->resetMatrix();

    painter->setOpacity(1);

    if (!d->getMessage(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(d->getMessage(index), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getFrom(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1: %2", d->getFromLabel(index), d->getFrom(index)), Qt::ElideMiddle, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getTo(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(i18n("%1: %2", d->getToLabel(index), d->getTo(index)), Qt::ElideMiddle, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getProgressMessage(index).isEmpty())
    {
        QString textToShow = fontMetrics.elidedText(d->getProgressMessage(index), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (d->getFilesProcessed(index) > 1)
    {
        QString textToShow;
        if (d->getFileTotals(index))
            textToShow = fontMetrics.elidedText(i18np("1 of %2 files processed", "%1 of %2 files processed", d->getFilesProcessed(index), d->getFileTotals(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);
        else
            textToShow = fontMetrics.elidedText(i18np("1 file processed", "%1 files processed", d->getFilesProcessed(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (!d->getSizeProcessed(index).isEmpty() || !d->getSpeed(index).isEmpty())
    {
        QString textToShow;
        if (!d->getSizeTotals(index).isEmpty() && !d->getSpeed(index).isEmpty())
            textToShow = fontMetrics.elidedText(i18n("%1 of %2 processed at %3", d->getSizeProcessed(index), d->getSizeTotals(index), d->getSpeed(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);
        else if (!d->getSizeTotals(index).isEmpty() && d->getSpeed(index).isEmpty())
            textToShow = fontMetrics.elidedText(i18n("%1 of %2 processed", d->getSizeProcessed(index), d->getSizeTotals(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);
        else if (d->getSizeTotals(index).isEmpty() && !d->getSpeed(index).isEmpty())
            textToShow = fontMetrics.elidedText(i18n("%1 processed at %2", d->getSizeProcessed(index), d->getSpeed(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);
        else
            textToShow = fontMetrics.elidedText(i18n("%1 processed", d->getSizeProcessed(index)), Qt::ElideRight, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin);

        textHeight = fontMetrics.size(Qt::TextSingleLine, textToShow).height();

        painter->drawText(d->getCurrentLeftMargin(textHeight), coordY, fontMetrics.width(textToShow), textHeight, Qt::AlignLeft, textToShow);

        coordY += d->separatorPixels + textHeight;
    }

    if (d->getPercent(index) > -1)
    {
        QStyleOptionProgressBarV2 *progressBarModel = d->getProgressBar(index);

        progressBarModel->rect = QRect(d->getCurrentLeftMargin(textHeight), coordY, canvas.width() - d->getCurrentLeftMargin(textHeight) - d->rightMargin - iconWidth - d->separatorPixels, d->progressBarHeight);

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, progressBarModel, painter);
    }

    painter->restore();

    painter->setClipping(true); // workaround for qt bug
}

QSize ProgressListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fontMetrics = option.fontMetrics;

    int itemHeight = d->separatorPixels;
    int itemWidth = qMax(option.rect.width(), d->minimumContentWidth);

    int textSize = fontMetrics.height() + d->separatorPixels;

    if (!d->getMessage(index).isEmpty())
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, d->getMessage(index)).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (!d->getProgressMessage(index).isEmpty())
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, d->getProgressMessage(index)).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (!d->getFrom(index).isEmpty())
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, d->getFrom(index)).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (!d->getTo(index).isEmpty())
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, d->getTo(index)).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (d->getFilesProcessed(index) > 1)
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, QString::number(d->getFilesProcessed(index))).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (!d->getSizeProcessed(index).isEmpty())
    {
        textSize = fontMetrics.size(Qt::TextSingleLine, d->getSizeProcessed(index)).height() + d->separatorPixels;
        itemHeight += textSize;
    }

    if (d->getPercent(index) > -1)
        itemHeight += d->progressBarHeight + d->separatorPixels;

    if (d->editorHeight > 0)
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
#include "progresslistdelegate_p.moc"
