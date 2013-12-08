/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
 *  Copyright (C) 2005-2013 David Faure <faure@kde.org>
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
 */

#include "ksycocautils_p.h"
#include "ksycoca.h"
#include <QDataStream>
#include <QStringList>

void KSycocaUtilsPrivate::read( QDataStream &s, QString &str )
{
  quint32 bytes;
  s >> bytes;                          // read size of string
  if ( bytes > 8192 ) {                // null string or too big
      if (bytes != 0xffffffff)
         KSycoca::flagError();
      str.clear();
  }
  else if ( bytes > 0 ) {              // not empty
      int bt = bytes/2;
      str.resize( bt );
      QChar* ch = str.data();
      char t[8192];
      char *b = t;
      s.readRawData( b, bytes );
      while ( bt-- ) {
          *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
          b += 2;
      }
  } else {
      str.clear();
  }
}

void KSycocaUtilsPrivate::read(QDataStream &s, QStringList &list)
{
  list.clear();
  quint32 count;
  s >> count;                          // read size of list
  if (count >= 1024)
  {
     KSycoca::flagError();
     return;
  }
  for(quint32 i = 0; i < count; i++)
  {
     QString str;
     read(s, str);
     list.append( str );
     if (s.atEnd())
     {
        KSycoca::flagError();
        return;
     }
  }
}

