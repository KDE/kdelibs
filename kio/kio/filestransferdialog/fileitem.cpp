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

#include "fileitem.h"

FileItem::FileItem() : id(-1)
{
}

FileItem::FileItem(int id, KIO::CopyInfo file, int actions, int progress)
{
    this->id = QVariant::fromValue(id);
    this->url = QVariant::fromValue(file.uSource.toLocalFile());
    this->size = QVariant::fromValue(file.size);
    this->actions = QVariant::fromValue(actions);
    this->progress = QVariant::fromValue(progress);
}

FileItem::FileItem(int id, const QString &url, int size, int actions, int progress)
{
    this->id = QVariant::fromValue(id);
    this->url = QVariant::fromValue(url);
    this->size = QVariant::fromValue(size);
    this->actions = QVariant::fromValue(actions);
    this->progress = QVariant::fromValue(progress);
}

bool FileItem::operator==(const FileItem &other) const {
    return id == other.id;
}

bool FileItem::isNull() const
{
    return id == -1;
}

QVariant FileItem::get(int role) const
{
    switch (role) {
    case FileHelper::FileID:
        return id;
    case FileHelper::URL:
        return url;
    case FileHelper::Size:
        return size;
    case FileHelper::Actions:
        return actions;
    case FileHelper::Progress:
        return progress;
    default:
        return QVariant();
    }
}

static QHash<int, QByteArray>& static_header() {
    static QHash<int, QByteArray> h;
    if (h.isEmpty()) {
        h[FileHelper::FileID] = "fid";
        h[FileHelper::URL] = "url";
        h[FileHelper::Size] = "size";
        h[FileHelper::Actions] = "actions";
        h[FileHelper::Progress] = "progress";
    }
    return h;
}

const QHash<int, QByteArray>& FileItem::header()
{
    return static_header();
}
