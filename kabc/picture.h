/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_PICTURE_H
#define KABC_PICTURE_H

#include <qimage.h>

namespace KABC {

class Picture
{
  friend QDataStream &operator<<( QDataStream &, const Picture & );
  friend QDataStream &operator>>( QDataStream &, Picture & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Picture();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the picture file.
   */
  Picture( const QString &url );

  /**
   * Consturctor.
   *
   * @param data  The raw data of the picture.
   */
  Picture( const QImage &data );

  /**
   * Destructor.
   */
  ~Picture();


  bool operator==( const Picture & ) const;
  bool operator!=( const Picture & ) const;

  /**
   * Sets a URL for the location of the picture file. When using this
   * function, @ref isIntern() will return 'false' until you use
   * @ref setData().
   *
   * @param url  The location URL of the picture file.
   */
  void setUrl( const QString &url );

  /**
   * Sets the raw data of the picture. When using this function,
   * @ref isIntern() will return 'true' until you use @ref setUrl().
   *
   * @param data  The raw data of the picture.
   */
  void setData( const QImage &data );

  /**
   * Sets the type of the picture.
   */
  void setType( const QString &type );

  /**
   * Returns whether the picture is described by a URL (extern) or
   * by the raw data (intern).
   * When this method returns 'true' you can use @ref data() to
   * get the raw data. Otherwise you can request the URL of this
   * picture by @ref url() and load the raw data from that location.
   */
  bool isIntern() const;

  /**
   * Returns the location URL of this picture.
   */
  QString url() const;

  /**
   * Returns the raw data of this picture.
   */
  QImage data() const;

  /**
   * Returns the type of this picture.
   */
  QString type() const;

  /**
   * Returns string representation of the picture.
   */
  QString asString() const;

private:
  QString mUrl;
  QString mType;
  QImage mData;

  int mIntern;
};

QDataStream &operator<<( QDataStream &, const Picture & );
QDataStream &operator>>( QDataStream &, Picture & );

}
#endif
