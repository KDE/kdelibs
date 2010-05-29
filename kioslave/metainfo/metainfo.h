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

//  $Id: metainfo.h 465272 2005-09-29 09:47:40Z mueller $

#ifndef _METAINFO_H_
#define _METAINFO_H_

#include <kio/slavebase.h>

class MetaInfoProtocol : public KIO::SlaveBase
{
public:
    MetaInfoProtocol(const QCString &pool, const QCString &app);
    virtual ~MetaInfoProtocol();

    virtual void get(const KURL &url);
    virtual void put(const KURL& url, int permissions,
                     bool overwrite, bool resume);

};

#endif
