/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1999 Matthias Ettrich <ettrich@kde.org>
 
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

#include <dcopobject.h>

static QMap<QCString, DCOPObject *> *objMap_ = 0;

inline QMap<QCString, DCOPObject *> *objMap()
{
  if (!objMap_)
    objMap_ = new QMap<QCString, DCOPObject *>;
  return objMap_;
}

DCOPObject::DCOPObject(QObject *obj)
{
  QObject *currentObj = obj;
  while (currentObj != 0L) {
    ident.prepend( currentObj->name() );
    ident.prepend("/");
    currentObj = currentObj->parent();
  }
  ident = ident.mid(2);
  qDebug("set dcopobject id to %s",ident.data());

  objMap()->insert(ident, this);
}

DCOPObject::DCOPObject(const QCString &objId)
  : ident(objId)
{
  ident = objId;
  objMap()->insert(ident, this);
}

DCOPObject::~DCOPObject()
{
  objMap()->remove(ident);
}

QCString DCOPObject::id() const
{
  return ident;
}

bool DCOPObject::hasObject(const QCString &objId)
{
  if (objMap()->contains(objId))
    return true;
  else
    return false;
}

DCOPObject *DCOPObject::find(const QCString &objId)
{
  QMap<QCString, DCOPObject *>::ConstIterator it;
  it = objMap()->find(objId);
  if (it != objMap()->end())
    return *it;
  else
    return 0L;
}
