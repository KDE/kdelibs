/*  This file is part of the KDE project
    Copyright (C) 2012 Cyril Oblikov <munknex@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2+ as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <math.h>
#include "filemodel.h"

FileModel::FileModel(QObject *parent) :
    QAbstractListModel(parent)
{
    setRoleNames(FileItem::header());
    connect(this, SIGNAL(rowsInserted(const QModelIndex&,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex&,int,int)), SIGNAL(countChanged()));
}

void FileModel::enqueue(const FileItem &file)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_files.append(file);
    qDebug("FileModel::enqueue appending fid:%d name:%s to %s", file.id.toInt(), qPrintable(file.url.toString()), qPrintable(objectName()));
    endInsertRows();
    emit countChanged();
}

FileItem FileModel::dequeue()
{
    beginRemoveRows(QModelIndex(), 0, 0);
    if (m_files.isEmpty()) {
        qFatal("UnfinishedModel::dequeue(): m_files is empty");
    }
    FileItem file = m_files.takeFirst();
    endRemoveRows();
    return file;
}

int FileModel::findPosById(int id) const
{
    int pos = -1;
    for (int i = 0; i < m_files.count(); i++) {
        if (m_files[i].id == id) {
            pos = i;
            break;
        }
    }
    return pos;
}

bool FileModel::contains(int id) const
{
    return findPosById(id) != -1;
}

FileItem FileModel::peekAtFile(int id) const
{
    qDebug("FileModel::peekAtFile: fid:%d", id);

    FileItem requestedFile;
    int pos = findPosById(id);
    if (pos != -1)
        requestedFile = m_files[pos];

    return requestedFile;
}

FileItem FileModel::takeFile(int id)
{
    qDebug("FileModel::takeFile: fid:%d from:%s", id, qPrintable(objectName()));

    FileItem requestedFile;
    int pos = findPosById(id);
    if (pos != -1) {
        qDebug("FileModel::takeFile: found pos = %d", pos);
        requestedFile = m_files[pos];
        beginRemoveRows(QModelIndex(), pos, pos);
        m_files.removeOne(requestedFile);
        endRemoveRows();
    }
    return requestedFile;
}

void FileModel::replaceFile(int id, const FileItem &newFile)
{
    qDebug("FileModel::replaceFile: fid:%d", id);

    int pos = findPosById(id);
    if (pos != -1) {
        qDebug("FileModel::replaceFile: found pos = %d", pos);
        m_files[pos] = newFile;
        emit dataChanged(createIndex(pos, 0), createIndex(pos, 0));
    }
}

void FileModel::updateFileProgress(int id, qreal progress, FileHelper::FileActions updActions)
{
    //qDebug("FileModel::updateFileProgress: fid:%d, progress:%f", id, progress);

    int pos = findPosById(id);
    if (pos != -1) {
        m_files[pos].progress = QVariant::fromValue(progress);
        int firstUpdated = pos - 1;
        while (firstUpdated >= 0 &&  m_files[firstUpdated].actions.toInt() != updActions) {
            m_files[firstUpdated].progress = QVariant::fromValue(1.0);
            qDebug("FileModel::updateFileProgress: fid:%d, actions:%d", id, updActions);
            m_files[firstUpdated].actions = QVariant::fromValue((int)updActions);
            firstUpdated--;
        }
        firstUpdated++;
        emit dataChanged(createIndex(firstUpdated, 0), createIndex(pos, 0));
    } else {
        qDebug("Error: FileModel::updateFileProgress: fid:%d, progress:%f no such file", id, progress);
    }
}

void FileModel::setFinished()
{
    qDebug("FileModel::setFinished:");
    if (!m_files.isEmpty()) {
        qDebug("FileModel::setFinished: for file %s", qPrintable(m_files.last().url.toString()));
        m_files.last().progress = QVariant::fromValue(1.0);
        m_files.last().actions = FileHelper::Finished;
        int pos = m_files.count() - 1;
        emit dataChanged(createIndex(pos, 0), createIndex(pos, 0));
    }
}

void FileModel::removeFinished()
{
    int firstUnfinished = m_files.count();
    for (int i = 0; i < m_files.count(); i++) {
        const FileItem &file = m_files[i];
        if (file.actions.toInt() != FileHelper::Finished) {
            firstUnfinished = i;
            break;
        }
    }

    beginRemoveRows(QModelIndex(), 0, firstUnfinished - 1);
    if (firstUnfinished == m_files.count()) {
        m_files.clear();
    } else {
        m_files = m_files.mid(firstUnfinished);
    }
    endRemoveRows();
}

QList<int> FileModel::allIDs() const
{
    QList<int> fids;
    foreach (const FileItem &file, m_files) {
        fids.append(file.id.toInt());
    }
    return fids;
}

QList<int> FileModel::takeAll()
{
    QList<int> fids;
    foreach (const FileItem &file, m_files) {
        fids.append(file.id.toInt());
    }

    beginRemoveRows(QModelIndex(), 0, m_files.count() - 1);
    m_files.clear();
    endRemoveRows();
    return fids;
}

int FileModel::rowCount(const QModelIndex &) const
{
    return m_files.count();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > m_files.count() || role <= Qt::UserRole)
        return QVariant();

    const FileItem &file = m_files[index.row()];
    return file.get(role);
}
