/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     

#ifndef KDB_INDEX_H
#define KDB_INDEX_H "$Id$"

#include <qlist.h>
#include <qstring.h>

#include <kdb/object.h>

namespace KDB {

class Index;
typedef QList<Index>         IndexList;
typedef QListIterator<Index> IndexIterator;
	
/**
 * This class should provide information for an index of a table
 *
 * Things such fields involved, ordering, if primary or not and so on.
 *
 * @author Alessandro Praduroux <pradu@thekompany.com>
 * @version kdbcore 0.0.2
 */
class Index : public Object{

    Q_OBJECT

public:
    Index(QObject *parent = 0L, const char *name = 0L);

    virtual ~Index();

};

}
#endif
