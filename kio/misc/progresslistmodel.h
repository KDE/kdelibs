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

#ifndef PROGRESSLISTMODEL_H
#define PROGRESSLISTMODEL_H

#include <QAbstractListModel>
#include <QTimer>

#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kiconloader.h>

class QStyleOptionButton;
class QStyleOptionProgressBarV2;

struct actionInfo
{
    int actionId;
    QString actionText;
    QStyleOptionButton *optionButton;
};

struct jobInfo
{
    int jobId;
    QString applicationInternalName;
    QString applicationName;
    QString icon;
    qlonglong fileTotals;
    qlonglong filesProcessed;
    QString sizeTotals;
    QString sizeProcessed;
    qlonglong timeElapsed;
    qlonglong timeTotals;
    QString from;
    QString to;
    QString fromLabel;
    QString toLabel;
    int percent;
    QString message;
    QList<actionInfo> actionInfoList;
    QStyleOptionProgressBarV2 *progressBar;
    KIconLoader *iconLoader;
};

class ProgressListModel
    : public QAbstractItemModel
{
    Q_OBJECT

public:
    class PersistentData;

    /**
      * @brief Constructor for the model of a progress list.
      */
    ProgressListModel(QObject *parent = 0);

    /**
      * @brief Destructor for the model of a progress list.
      */
    ~ProgressListModel();

    QModelIndex parent(const QModelIndex&) const;

    /**
      * @brief Returns data from the data structure.
      */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
      * @brief Get the children model index for the given row.
      */
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex indexForJob(uint jobId) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
      * @brief Get the number of progresses added to the list.
      */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
      * @brief Insert a row into the data structure.
      */
    bool insertRow(int row, uint jobId, const QModelIndex &parent = QModelIndex());

    /**
      * @brief Remove a row from the data structure.
      */
    void removeRow(int row, const QModelIndex &parent = QModelIndex());

    /**
      * @brief Sets the data on role for a given index.
      */
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    void newJob(uint jobId, const QString &internalAppName, const QString &jobIcon, const QString &appName);

    void finishJob(uint jobId);

    void newAction(uint jobId, uint actionId, const QString &actionText);

    const QList<actionInfo> &actions(uint jobId) const;

    QStyleOptionProgressBarV2 *progressBar(const QModelIndex &index) const;

private:
    /**
      * @internal
      */
    bool setData(int row, const QVariant &value, int role = Qt::EditRole);

    /**
      * @internal
      */
    KIconLoader *iconLoader(int row) const;

    /**
      * @internal
      */
    void setIconLoader(int row, KIconLoader *iconLoader);

    QList<jobInfo> jobInfoList;
};

#endif // PROGRESSLISTMODEL_H
