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
#ifndef KABC_GEO_H
#define KABC_GEO_H
//$Id$

#include <qstring.h>

namespace KABC {

/**
  @short Geographic position
  
  This class represents a geographic position.
*/
class Geo
{
    friend QDataStream &operator<<( QDataStream &, const Geo & );
    friend QDataStream &operator>>( QDataStream &, Geo & );

  public:
    /**
      Construct invalid geographics position object.
    */
    Geo();
    /**
      Construct geographics position object.
      
      @param latitude  Geographical latitude
      @param longitude Geographical longitude
    */
    Geo( float latitude, float longitude );
    
    /**
      Set latitude.
    */
    void setLatitude( float );
    /**
      Get latitude.
    */
    float latitude() const;
    
    /**
      Set longitude.
    */
    void setLongitude( float );
    /**
      Get longitude.
    */
    float longitude() const;
  
    /**
      Return, if this object contains a valid geographical position.
    */
    bool isValid() const;
    
    bool operator==( const Geo & ) const;
    bool operator!=( const Geo & ) const;

    /**
      Return string representation of geographical position.
    */
    QString asString() const;
      
  private:
    float mLatitude;
    float mLongitude;
    
    bool mValid;
    bool mValidLat;
    bool mValidLong;
};

QDataStream &operator<<( QDataStream &, const Geo & );
QDataStream &operator>>( QDataStream &, Geo & );

}

#endif
