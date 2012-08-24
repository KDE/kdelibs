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

#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "filehelpers.h"
#include "fileitem.h"


class FileModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(FileActions)

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit FileModel(QObject *parent = 0);
    int count() const { return rowCount(); }

    void enqueue(const FileItem &file);
    FileItem dequeue();
    bool contains(int id) const;
    FileItem peekAtFile(int id) const;
    FileItem takeFile(int id);
    void replaceFile(int id, const FileItem &newFile);
    void updateFileProgress(int id, qulonglong size, FileHelper::FileActions actions);
    void setFinished();
    void removeFinished();
    QList<int> allIDs() const;

    // overloaded
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

signals:
    void countChanged();

private:
    int findPosById(int id) const;
    QList<FileItem> m_files;
};

#endif // FILEMODEL_H
