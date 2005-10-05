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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_PICTURE_H
#define KABC_PICTURE_H

#include <qimage.h>
#include <qstring.h>
#include <QDataStream>

#include <kdelibs_export.h>

namespace KABC {

class KABC_EXPORT Picture
{
  friend KABC_EXPORT QDataStream &operator<<( QDataStream &, const Picture & );
  friend KABC_EXPORT QDataStream &operator>>( QDataStream &, Picture & );

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
   * function, isIntern() will return 'false' until you use
   * setData().
   *
   * @param url  The location URL of the picture file.
   */
  void setUrl( const QString &url );

  /**
   * Sets the raw data of the picture. When using this function,
   * isIntern() will return 'true' until you use setUrl().
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
   * When this method returns 'true' you can use data() to
   * get the raw data. Otherwise you can request the URL of this
   * picture by url() and load the raw data from that location.
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

KABC_EXPORT QDataStream &operator<<( QDataStream &, const Picture & );
KABC_EXPORT QDataStream &operator>>( QDataStream &, Picture & );

}
#endif
