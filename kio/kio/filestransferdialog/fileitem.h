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

#ifndef FILEITEM_H
#define FILEITEM_H

#include <QVariantList>
#include "filehelpers.h"
#include "kio/copyjob.h"

class FileItem
{
public:
    FileItem();
    FileItem(int id, KIO::CopyInfo file, int actions = 0, int progress = 0);
    FileItem(int id, const QString &url, int size, int actions = 0, int progress = 0);
    bool operator==(const FileItem &other) const;

    bool isNull() const;
    QVariant get(int role) const;
    static const QHash<int, QByteArray>& header();

    QVariant id;
    QVariant url;
    QVariant size;
    QVariant actions;
    QVariant progress;
};

#endif // FILEITEM_H
