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

#ifndef KABC_SOUND_H
#define KABC_SOUND_H

#include <qcstring.h>
#include <qstring.h>

namespace KABC {

class Sound
{
  friend QDataStream &operator<<( QDataStream &, const Sound & );
  friend QDataStream &operator>>( QDataStream &, Sound & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Sound();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the sound file.
   */
  Sound( const QString &url );

  /**
   * Consturctor.
   *
   * @param data  The raw data of the sound.
   */
  Sound( const QByteArray &data );

  /**
   * Destructor.
   */
  ~Sound();


  bool operator==( const Sound & ) const;
  bool operator!=( const Sound & ) const;

  /**
   * Sets a URL for the location of the sound file. When using this
   * function, @ref isIntern() will return 'false' until you use
   * @ref setData().
   *
   * @param url  The location URL of the sound file.
   */
  void setUrl( const QString &url );

  /**
   * Sets the raw data of the sound. When using this function,
   * @ref isIntern() will return 'true' until you use @ref setUrl().
   *
   * @param data  The raw data of the sound.
   */
  void setData( const QByteArray &data );

  /**
   * Returns whether the sound is described by a URL (extern) or
   * by the raw data (intern).
   * When this method returns 'true' you can use @ref data() to
   * get the raw data. Otherwise you can request the URL of this
   * sound by @ref url() and load the raw data from that location.
   */
  bool isIntern() const;

  /**
   * Returns the location URL of this sound.
   */
  QString url() const;

  /**
   * Returns the raw data of this sound.
   */
  QByteArray data() const;

  /**
   * Returns string representation of the sound.
   */
  QString asString() const;

private:
  QString mUrl;
  QByteArray mData;

  int mIntern;
};

QDataStream &operator<<( QDataStream &, const Sound & );
QDataStream &operator>>( QDataStream &, Sound & );

}
#endif
