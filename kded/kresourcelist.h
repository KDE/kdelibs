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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/
#ifndef __kresourcelist_h__
#define __kresourcelist_h__

#include <qlinkedlist.h>
#include <qstring.h>
#include <qregexp.h>

class QDataStream;
class KSycocaEntry;

struct KSycocaResource
{
   QString resource;
   QString extension;
};

class KSycocaResourceList : public QLinkedList<KSycocaResource>
{
public:
   KSycocaResourceList() { };
   void add(const QString &resource, const QString &filter)
   {
     KSycocaResource res;
     res.resource = resource;
     res.extension = filter.mid(1);
     append(res);
   }
};

#endif
