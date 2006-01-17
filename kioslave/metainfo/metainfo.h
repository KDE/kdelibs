/*  This file is part of the KDE libraries
    Copyright (C) 2002 Rolf Magnus <ramagnus@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation version 2.0

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _METAINFO_H_
#define _METAINFO_H_

#include <kio/slavebase.h>

class MetaInfoProtocol : public KIO::SlaveBase
{
public:
    MetaInfoProtocol(const QByteArray &pool, const QByteArray &app);
    virtual ~MetaInfoProtocol();

    virtual void get(const KUrl &url);
    virtual void put(const KUrl& url, int permissions,
                     bool overwrite, bool resume);

};

#endif
