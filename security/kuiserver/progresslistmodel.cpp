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

#include "progresslistmodel.h"
#include "progresslistdelegate.h"

#include <QStyleOptionProgressBarV2>

#include <kiconloader.h>
#include <kwidgetjobtracker.h>

ProgressListModel::ProgressListModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ProgressListModel::~ProgressListModel()
{
    foreach (const JobInfo &it, jobInfoList)
    {
        delete it.progressBar;
        delete it.iconLoader;
        delete it.defaultTracker;
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
        case ProgressListDelegate::JobId:
            result = jobInfoList[index.row()].jobId;
            break;
        case ProgressListDelegate::ApplicationInternalName:
            result = jobInfoList[index.row()].applicationInternalName;
            break;
        case ProgressListDelegate::ApplicationName:
            result = jobInfoList[index.row()].applicationName;
            break;
        case ProgressListDelegate::Icon:
            result = jobInfoList[index.row()].icon;
            break;
        case ProgressListDelegate::FileTotals:
            result = jobInfoList[index.row()].fileTotals;
            break;
        case ProgressListDelegate::FilesProcessed:
            result = jobInfoList[index.row()].filesProcessed;
            break;
        case ProgressListDelegate::SizeTotals:
            result = jobInfoList[index.row()].sizeTotals;
            break;
        case ProgressListDelegate::SizeProcessed:
            result = jobInfoList[index.row()].sizeProcessed;
            break;
        case ProgressListDelegate::TimeTotals:
            result = jobInfoList[index.row()].timeTotals;
            break;
        case ProgressListDelegate::TimeElapsed:
            result = jobInfoList[index.row()].timeElapsed;
            break;
        case ProgressListDelegate::From:
            result = jobInfoList[index.row()].from;
            break;
        case ProgressListDelegate::To:
            result = jobInfoList[index.row()].to;
            break;
        case ProgressListDelegate::FromLabel:
            result = jobInfoList[index.row()].fromLabel;
            break;
        case ProgressListDelegate::ToLabel:
            result = jobInfoList[index.row()].toLabel;
            break;
        case ProgressListDelegate::Speed:
            result = jobInfoList[index.row()].speed;
            break;
        case ProgressListDelegate::Percent:
            result = jobInfoList[index.row()].percent;
            break;
        case ProgressListDelegate::Message:
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
    foreach (const JobInfo &it, jobInfoList)
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

bool ProgressListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(count);
    Q_UNUSED(parent);

    beginInsertRows(QModelIndex(), row, row);

    JobInfo newJob;

    newJob.jobId = -1;
    newJob.applicationInternalName = QString();
    newJob.applicationName = QString();
    newJob.icon = QString();
    newJob.fileTotals = -1;
    newJob.filesProcessed = -1;
    newJob.dirTotals = -1;
    newJob.dirsProcessed = -1;
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
    newJob.progressMessage = QString();
    newJob.progressBar = 0;
    newJob.iconLoader = 0;
    newJob.defaultTracker = 0;

    jobInfoList.append(newJob);

    endInsertRows();

    return true;
}

bool ProgressListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(count);
    Q_UNUSED(parent);

    if (row >= rowCount())
        return false;

    beginRemoveRows(QModelIndex(), row, row);

    jobInfoList.removeAt(row);

    endRemoveRows();

    return true;
}

bool ProgressListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    switch (role)
    {
        case ProgressListDelegate::JobId:
            jobInfoList[index.row()].jobId = value.toInt();
            break;
        case ProgressListDelegate::ApplicationInternalName:
            jobInfoList[index.row()].applicationInternalName = value.toString();
            break;
        case ProgressListDelegate::ApplicationName:
            jobInfoList[index.row()].applicationName = value.toString();
            break;
        case ProgressListDelegate::Icon:
            jobInfoList[index.row()].icon = value.toString();
            break;
        case ProgressListDelegate::FileTotals:
            jobInfoList[index.row()].fileTotals = value.toLongLong();
            break;
        case ProgressListDelegate::FilesProcessed:
            jobInfoList[index.row()].filesProcessed = value.toLongLong();
            break;
        case ProgressListDelegate::SizeTotals:
            jobInfoList[index.row()].sizeTotals = value.toString();
            break;
        case ProgressListDelegate::SizeProcessed:
            jobInfoList[index.row()].sizeProcessed = value.toString();
            break;
        case ProgressListDelegate::TimeTotals:
            jobInfoList[index.row()].timeTotals = value.toLongLong();
            break;
        case ProgressListDelegate::TimeElapsed:
            jobInfoList[index.row()].timeElapsed = value.toLongLong();
            break;
        case ProgressListDelegate::From:
            jobInfoList[index.row()].from = value.toString();
            break;
        case ProgressListDelegate::To:
            jobInfoList[index.row()].to = value.toString();
            break;
        case ProgressListDelegate::FromLabel:
            jobInfoList[index.row()].fromLabel = value.toString();
            break;
        case ProgressListDelegate::ToLabel:
            jobInfoList[index.row()].toLabel = value.toString();
            break;
        case ProgressListDelegate::Speed:
            jobInfoList[index.row()].speed = value.toString();
            break;
        case ProgressListDelegate::Percent:
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
        case ProgressListDelegate::Message:
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

    insertRow(rowCount());
    setData(newRow, jobId, ProgressListDelegate::JobId);
    setData(newRow, internalAppName, ProgressListDelegate::ApplicationInternalName);
    setData(newRow, appName, ProgressListDelegate::ApplicationName);
    setData(newRow, jobIcon, ProgressListDelegate::Icon);
    setIconLoader(newRow, new KIconLoader(internalAppName));
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

    ActionInfo newActionInfo;
    newActionInfo.actionId = actionId;
    newActionInfo.actionText = actionText;

    jobInfoList[row].actionInfoList.append(newActionInfo);

    emit dataChanged(index, index);
    emit actionModified(index);
}

void ProgressListModel::editAction(int jobId, int actionId, const QString &actionText)
{
    QModelIndex index = indexForJob(jobId);

    int i = 0;
    bool keepSearching = true;
    ActionInfo actionIt;
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

    emit dataChanged(index, index);
    emit actionModified(index);
}

const QList<ActionInfo> &ProgressListModel::actions(uint jobId) const
{
    return jobInfoList[indexForJob(jobId).row()].actionInfoList;
}

QStyleOptionProgressBarV2 *ProgressListModel::progressBar(const QModelIndex &index) const
{
    return jobInfoList[index.row()].progressBar;
}

KWidgetJobTracker *ProgressListModel::defaultTracker(int row) const
{
    return jobInfoList[row].defaultTracker;
}

void ProgressListModel::setDefaultTracker(int row, KWidgetJobTracker *defaultTracker)
{
    jobInfoList[row].defaultTracker = defaultTracker;
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
