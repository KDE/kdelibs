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

#include "progresslistmodel.h"
#include "progresslistdelegate.h"

#include <QStyleOptionProgressBarV2>

#include <kiconloader.h>
#include <kio/defaultprogress.h>

ProgressListModel::ProgressListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ProgressListModel::~ProgressListModel()
{
    foreach (const jobInfo &it, jobInfoList)
    {
        delete it.progressBar;
        delete it.iconLoader;
        delete it.defaultProgress;
    }
}

QModelIndex ProgressListModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

QVariant ProgressListModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (!index.isValid())
        return result;

    switch (role)
    {
        case ProgressListDelegate::jobId:
            result = jobInfoList[index.row()].jobId;
            break;
        case ProgressListDelegate::applicationInternalName:
            result = jobInfoList[index.row()].applicationInternalName;
            break;
        case ProgressListDelegate::applicationName:
            result = jobInfoList[index.row()].applicationName;
            break;
        case ProgressListDelegate::icon:
            result = jobInfoList[index.row()].icon;
            break;
        case ProgressListDelegate::fileTotals:
            result = jobInfoList[index.row()].fileTotals;
            break;
        case ProgressListDelegate::filesProcessed:
            result = jobInfoList[index.row()].filesProcessed;
            break;
        case ProgressListDelegate::sizeTotals:
            result = jobInfoList[index.row()].sizeTotals;
            break;
        case ProgressListDelegate::sizeProcessed:
            result = jobInfoList[index.row()].sizeProcessed;
            break;
        case ProgressListDelegate::timeTotals:
            result = jobInfoList[index.row()].timeTotals;
            break;
        case ProgressListDelegate::timeElapsed:
            result = jobInfoList[index.row()].timeElapsed;
            break;
        case ProgressListDelegate::from:
            result = jobInfoList[index.row()].from;
            break;
        case ProgressListDelegate::to:
            result = jobInfoList[index.row()].to;
            break;
        case ProgressListDelegate::fromLabel:
            result = jobInfoList[index.row()].fromLabel;
            break;
        case ProgressListDelegate::toLabel:
            result = jobInfoList[index.row()].toLabel;
            break;
        case ProgressListDelegate::speed:
            result = jobInfoList[index.row()].speed;
            break;
        case ProgressListDelegate::percent:
            result = jobInfoList[index.row()].percent;
            break;
        case ProgressListDelegate::message:
            result = jobInfoList[index.row()].message;
            break;
        default:
            return result;
    }

    return result;
}

Qt::ItemFlags ProgressListModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable
           | Qt::ItemIsEnabled
           | Qt::ItemIsEditable;
}

QModelIndex ProgressListModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= rowCount())
        return QModelIndex();

    return createIndex(row, column, iconLoader(row));
}

QModelIndex ProgressListModel::indexForJob(uint jobId) const
{
    int i = 0;
    foreach (const jobInfo &it, jobInfoList)
    {
        if (it.jobId == jobId)
            return createIndex(i, 0, iconLoader(i));

        i++;
    }

    return QModelIndex();
}

int ProgressListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

int ProgressListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : jobInfoList.count();
}

bool ProgressListModel::insertRow(int row, uint jobId, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), row, row);

    jobInfo newJob;

    newJob.jobId = jobId;
    newJob.applicationInternalName = QString();
    newJob.applicationName = QString();
    newJob.icon = QString();
    newJob.fileTotals = -1;
    newJob.filesProcessed = -1;
    newJob.sizeTotals = QString();
    newJob.sizeProcessed = QString();
    newJob.timeElapsed = -1;
    newJob.timeTotals = -1;
    newJob.from = QString();
    newJob.to = QString();
    newJob.fromLabel = QString();
    newJob.toLabel = QString();
    newJob.speed = QString();
    newJob.percent = -1;
    newJob.message = QString();
    newJob.progressBar = 0;
    newJob.iconLoader = 0;
    newJob.defaultProgress = 0;

    jobInfoList.append(newJob);

    endInsertRows();

    return true;
}

void ProgressListModel::removeRow(int row, const QModelIndex &parent)
{
    if (row >= rowCount())
        return;

    beginRemoveRows(QModelIndex(), row, row);

    jobInfoList.removeAt(row);

    endRemoveRows();
}

bool ProgressListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    switch (role)
    {
        case ProgressListDelegate::jobId:
            jobInfoList[index.row()].jobId = value.toInt();
            break;
        case ProgressListDelegate::applicationInternalName:
            jobInfoList[index.row()].applicationInternalName = value.toString();
            break;
        case ProgressListDelegate::applicationName:
            jobInfoList[index.row()].applicationName = value.toString();
            break;
        case ProgressListDelegate::icon:
            jobInfoList[index.row()].icon = value.toString();
            break;
        case ProgressListDelegate::fileTotals:
            jobInfoList[index.row()].fileTotals = value.toLongLong();
            break;
        case ProgressListDelegate::filesProcessed:
            jobInfoList[index.row()].filesProcessed = value.toLongLong();
            break;
        case ProgressListDelegate::sizeTotals:
            jobInfoList[index.row()].sizeTotals = value.toString();
            break;
        case ProgressListDelegate::sizeProcessed:
            jobInfoList[index.row()].sizeProcessed = value.toString();
            break;
        case ProgressListDelegate::timeTotals:
            jobInfoList[index.row()].timeTotals = value.toLongLong();
            break;
        case ProgressListDelegate::timeElapsed:
            jobInfoList[index.row()].timeElapsed = value.toLongLong();
            break;
        case ProgressListDelegate::from:
            jobInfoList[index.row()].from = value.toString();
            break;
        case ProgressListDelegate::to:
            jobInfoList[index.row()].to = value.toString();
            break;
        case ProgressListDelegate::fromLabel:
            jobInfoList[index.row()].fromLabel = value.toString();
            break;
        case ProgressListDelegate::toLabel:
            jobInfoList[index.row()].toLabel = value.toString();
            break;
        case ProgressListDelegate::speed:
            jobInfoList[index.row()].speed = value.toString();
            break;
        case ProgressListDelegate::percent:
            if (!jobInfoList[index.row()].progressBar)
            {
                jobInfoList[index.row()].progressBar = new QStyleOptionProgressBarV2();
                jobInfoList[index.row()].progressBar->maximum = 100;
                jobInfoList[index.row()].progressBar->minimum = 0;
            }
            jobInfoList[index.row()].percent = value.toInt();
            if (jobInfoList[index.row()].progressBar)
            {
                jobInfoList[index.row()].progressBar->progress = jobInfoList[index.row()].percent;
            }
            break;
        case ProgressListDelegate::message:
            jobInfoList[index.row()].message = value.toString();
            break;
        default:
            return false;
    }

    emit dataChanged(index, index);

    return true;
}

void ProgressListModel::newJob(uint jobId, const QString &internalAppName, const QString &jobIcon, const QString &appName, bool showProgress)
{
    int newRow = rowCount();

    insertRow(rowCount(), jobId);
    setData(newRow, internalAppName, ProgressListDelegate::applicationInternalName);
    setData(newRow, appName, ProgressListDelegate::applicationName);
    setData(newRow, jobIcon, ProgressListDelegate::icon);
    setIconLoader(newRow, new KIconLoader(internalAppName));

    // Add a configuration window for showing this progresses or not (and more options) (ereslibre)
    // KIO::DefaultProgress *progressWindow = new KIO::DefaultProgress(true);
    // setDefaultProgress(newRow, progressWindow);
}

void ProgressListModel::finishJob(uint jobId)
{
    QModelIndex indexToRemove = indexForJob(jobId);

    if (indexToRemove.isValid())
        removeRow(indexToRemove.row());
}

void ProgressListModel::newAction(uint jobId, uint actionId, const QString &actionText)
{
    QModelIndex index = indexForJob(jobId);

    int row = index.row();

    actionInfo newActionInfo;
    newActionInfo.actionId = actionId;
    newActionInfo.actionText = actionText;

    jobInfoList[row].actionInfoList.append(newActionInfo);

    emit actionAdded(index);
    emit dataChanged(index, index);
}

void ProgressListModel::editAction(int jobId, int actionId, const QString &actionText)
{
    QModelIndex index = indexForJob(jobId);

    int i = 0;
    bool keepSearching = true;
    actionInfo actionIt;
    while ((i < jobInfoList[index.row()].actionInfoList.count()) &&
           keepSearching)
    {
        actionIt = jobInfoList[index.row()].actionInfoList[i];

        if (actionId == actionIt.actionId)
        {
            jobInfoList[index.row()].actionInfoList[i].actionText = actionText;

            keepSearching = false;
        }

        i++;
    }

    emit actionEdited(index);
    emit dataChanged(index, index);
}

void ProgressListModel::removeAction(int jobId, int actionId)
{
    QModelIndex index = indexForJob(jobId);

    int i = 0;
    bool keepSearching = true;
    actionInfo actionIt;
    while ((i < jobInfoList[index.row()].actionInfoList.count()) &&
           keepSearching)
    {
        actionIt = jobInfoList[index.row()].actionInfoList[i];

        if (actionId == actionIt.actionId)
        {
            jobInfoList[index.row()].actionInfoList.removeAt(i);

            keepSearching = false;
        }

        i++;
    }

    emit actionRemoved(index);
    emit dataChanged(index, index);
}

const QList<actionInfo> &ProgressListModel::actions(uint jobId) const
{
    return jobInfoList[indexForJob(jobId).row()].actionInfoList;
}

QStyleOptionProgressBarV2 *ProgressListModel::progressBar(const QModelIndex &index) const
{
    return jobInfoList[index.row()].progressBar;
}

KIO::DefaultProgress *ProgressListModel::defaultProgress(int row) const
{
    return jobInfoList[row].defaultProgress;
}

void ProgressListModel::setDefaultProgress(int row, KIO::DefaultProgress *defaultProgress)
{
    jobInfoList[row].defaultProgress = defaultProgress;
}

bool ProgressListModel::setData(int row, const QVariant &value, int role)
{
    return setData(index(row), value, role);
}

KIconLoader *ProgressListModel::iconLoader(int row) const
{
    return jobInfoList[row].iconLoader;
}

void ProgressListModel::setIconLoader(int row, KIconLoader *iconLoader)
{
    jobInfoList[row].iconLoader = iconLoader;
}

#include "progresslistmodel.moc"
