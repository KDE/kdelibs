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

#ifndef _DCOPOBJECT_H
#define _DCOPOBJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qstring.h>

class DCOPObject
{
 public:
  DCOPObject(QObject *obj);
  DCOPObject(const QCString &objId);
  virtual ~DCOPObject();

  QCString id() const;

  /**
   * dispatch a message.
   */
  virtual bool process(const QCString &fun, const QByteArray &data,
		       QByteArray &replyData) = 0;

  static bool hasObject(const QCString &objId);
  static DCOPObject *find(const QCString &objId);

 private:
  QCString ident;
};

#endif
