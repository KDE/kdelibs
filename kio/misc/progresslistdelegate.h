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

#ifndef PROGRESSLISTDELEGATE_H
#define PROGRESSLISTDELEGATE_H

#include <QItemDelegate>
#include <QModelIndex>

class QListView;

class ProgressListDelegate
    : public QItemDelegate
{
    Q_OBJECT
    Q_ENUMS(ProgressItemRole)

public:
    /**
      * @brief Constructor for the progress delegate.
      */
    ProgressListDelegate(QObject *parent = 0, QListView *listView = 0);

    /**
     * @brief Destructor for the progress delegate.
     */
    ~ProgressListDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    /**
      * @brief Paints the progress delegate.
      */
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    /**
      * @brief Gets the size of the progress delegate.
      */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setSeparatorPixels(int separatorPixels);
    void setLeftMargin(int leftMargin);
    void setRightMargin(int rightMargin);
    void setProgressBarHeight(int progressBarHeight);
    void setMinimumItemHeight(int minimumItemHeight);
    void setMinimumContentWidth(int minimumContentWidth);
    void setEditorHeight(int editorHeight);

    enum ProgressItemRole
    {
        jobId = 33,
        applicationInternalName,
        applicationName,
        icon,
        fileTotals,
        filesProcessed,
        sizeTotals,
        sizeProcessed,
        timeTotals,
        timeElapsed,
        from,
        to,
        fromLabel,
        toLabel,
        percent,
        message
    };

Q_SIGNALS:
    void actionPerformed(int id) const;

private:
    class Private;
    Private *d;
};

#endif // PROGRESSLISTDELEGATE_H
