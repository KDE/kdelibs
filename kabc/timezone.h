/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KABC_TIMEZONE_H
#define KABC_TIMEZONE_H
// $Id$

#include <qstring.h>

namespace KABC {

/*
  @short Time zone information.
  
  This class stores information about a time zone.
*/
class TimeZone
{
    friend QDataStream &operator<<( QDataStream &, const TimeZone & );
    friend QDataStream &operator>>( QDataStream &, TimeZone & );

  public:
    /*
      Construct invalid time zone.
    */
    TimeZone();
    /*
      Construct time zone.
    
      @param offset Offset in minutes relative to UTC.
    */
    TimeZone( int offset );
    
    /*
      Set time zone offset relative to UTC.
      
      @param offset Offset in minutes.
    */
    void setOffset( int );
    /*
      Return offset in minutes relative to UTC.
    */
    int offset() const;

    /*
      Return, if this time zone object is valid.
    */
    bool isValid() const;
    
    bool operator==( const TimeZone & ) const;
    bool operator!=( const TimeZone & ) const;
    
    /*
      Return string representation of time zone offset.
    */
    QString asString() const;
    
  private:
    int mOffset;  // Offset in minutes

    bool mValid;
};

QDataStream &operator<<( QDataStream &, const TimeZone & );
QDataStream &operator>>( QDataStream &, TimeZone & );

}

#endif
