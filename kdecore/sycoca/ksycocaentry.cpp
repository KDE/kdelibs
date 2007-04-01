/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
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

#include <ksycoca.h>
#include <ksycocaentry.h>

class KSycocaEntry::Private
{
public:
    Private()
        : offset( 0 ),
          deleted( false )
    {
    }

    int offset;
    bool deleted;
    QString path;
};

KSycocaEntry::KSycocaEntry(const QString &path)
    : d( new Private )
{
    d->path = path;
}

KSycocaEntry::KSycocaEntry( QDataStream &_str, int iOffset )
    : d( new Private )
{
    d->offset = iOffset;
    read( _str, d->path );
}

KSycocaEntry::~KSycocaEntry()
{
    delete d;
}

void KSycocaEntry::read( QDataStream &s, QString &str )
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
      QChar* ch = (QChar *) str.unicode();
      char t[8192];
      char *b = t;
      s.readRawData( b, bytes );
      while ( bt-- ) {
          *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
          b += 2;
      }
  } else {
      str = "";
  }
}

void KSycocaEntry::read( QDataStream &s, QStringList &list )
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

bool KSycocaEntry::isType(KSycocaType t) const
{
    return (t == KST_KSycocaEntry);
}

KSycocaType KSycocaEntry::sycocaType() const
{
    return KST_KSycocaEntry;
}

QString KSycocaEntry::entryPath() const
{
    return d->path;
}

bool KSycocaEntry::isDeleted() const
{
    return d->deleted;
}

void KSycocaEntry::setDeleted( bool deleted )
{
    d->deleted = deleted;
}

int KSycocaEntry::offset() const
{
    return d->offset;
}

void KSycocaEntry::save(QDataStream &s)
{
    d->offset = s.device()->pos(); // store position in member variable
    s << qint32(sycocaType()) << d->path;
}

void KSycocaEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


