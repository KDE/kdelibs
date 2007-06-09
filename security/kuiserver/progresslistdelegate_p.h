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

#ifndef PROGRESSLISTDELEGATE_P_H
#define PROGRESSLISTDELEGATE_P_H

class QPushButton;

class ProgressListDelegate::Private
    : public QObject
{
    Q_OBJECT

public:
    Private(QObject *parent, QListView *listView)
        : QObject(parent)
    {
        this->listView = listView;

        ProgressListModel *progressListModel = static_cast<ProgressListModel*>(listView->model());

        connect(progressListModel, SIGNAL(actionModified(const QModelIndex&)), this,
                SLOT(actionModified(const QModelIndex&)));
    }

    ~Private()
    {
    }

    int getJobId(const QModelIndex &index) const;
    QString getApplicationInternalName(const QModelIndex &index) const;
    QString getApplicationName(const QModelIndex &index) const;
    QString getIcon(const QModelIndex &index) const;
    qlonglong getFileTotals(const QModelIndex &index) const;
    qlonglong getFilesProcessed(const QModelIndex &index) const;
    qlonglong getDirTotals(const QModelIndex &index) const;
    qlonglong getDirsProcessed(const QModelIndex &index) const;
    QString getSizeTotals(const QModelIndex &index) const;
    QString getSizeProcessed(const QModelIndex &index) const;
    qlonglong getTimeTotals(const QModelIndex &index) const;
    qlonglong getTimeProcessed(const QModelIndex &index) const;
    QString getFromLabel(const QModelIndex &index) const;
    QString getFrom(const QModelIndex &index) const;
    QString getToLabel(const QModelIndex &index) const;
    QString getTo(const QModelIndex &index) const;
    QString getSpeed(const QModelIndex &index) const;
    int getPercent(const QModelIndex &index) const;
    QString getMessage(const QModelIndex &index) const;
    QString getProgressMessage(const QModelIndex &index) const;
    const QList<ActionInfo> &getActionList(const QModelIndex &index) const;
    QStyleOptionProgressBarV2 *getProgressBar(const QModelIndex &index) const;
    int getCurrentLeftMargin(int fontHeight) const;

public Q_SLOTS:
    void actionModified(const QModelIndex &index);

public:
    int separatorPixels;
    int leftMargin;
    int rightMargin;
    int progressBarHeight;
    int minimumItemHeight;
    int minimumContentWidth;
    int editorHeight;
    int iconWidth;
    QListView *listView;
    class QActionPushButton;
};

class ProgressListDelegate::Private::QActionPushButton
    : public QPushButton
{
    Q_OBJECT

public:
    QActionPushButton(int actionId, int jobId, const QString &actionText, QWidget *parent = 0);

public Q_SLOTS:
    void buttonPressed();

Q_SIGNALS:
    void actionButtonPressed(int actionId, int jobId);

private:
    int actionId;
    int jobId;
};

#endif // PROGRESSLISTDELEGATE_P_H
