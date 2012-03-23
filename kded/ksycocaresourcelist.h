/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/
#ifndef KSYCOCARESOURCELIST_H
#define KSYCOCARESOURCELIST_H

#include <QtCore/QLinkedList>
#include <QtCore/QString>
#include <QtCore/QRegExp>


struct KSycocaResource
{
    QByteArray resource;
    QString subdir;
    QString extension;
};

class KSycocaResourceList : public QLinkedList<KSycocaResource>
{
public:
    KSycocaResourceList() { }

    // resource is just used in the databaseChanged signal
    // subdir is always under QStandardPaths::GenericDataLocation. E.g. mime, kde5/services, etc.
    void add(const QByteArray& resource, const QString& subdir, const QString &filter)
    {
        KSycocaResource res;
        res.resource = resource;
        res.subdir = subdir;
        res.extension = filter.mid(1);
        append(res);
    }
};

#endif
