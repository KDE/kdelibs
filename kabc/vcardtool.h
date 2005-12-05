/*
    This file is part of libkabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_VCARDTOOL_H
#define KABC_VCARDTOOL_H

#include "addressee.h"
#include "vcardparser.h"

class QDateTime;

namespace KABC {

class Key;
class Picture;
class Secrecy;
class Sound;

class KABC_EXPORT VCardTool
{
  public:
    VCardTool();
    ~VCardTool();

    /**
      Creates a string that contains the addressees from the list in
      the vCard format.
     */
    QString createVCards( const Addressee::List& list, VCard::Version version = VCard::v3_0 );

    /**
      Parses the string and returns a list of addressee objects.
     */
    Addressee::List parseVCards( const QString& vcard );

  private:
    /**
      Split a string and replaces escaped separators on the fly with
      unescaped ones.
     */
    QStringList splitString( const QChar &sep, const QString &value );

    QDateTime parseDateTime( const QString &str );
    QString createDateTime( const QDateTime &dateTime );

    Picture parsePicture( const VCardLine &line );
    VCardLine createPicture( const QString &identifier, const Picture &pic );

    Sound parseSound( const VCardLine &line );
    VCardLine createSound( const Sound &snd );

    Key parseKey( const VCardLine &line );
    VCardLine createKey( const Key &key );

    Secrecy parseSecrecy( const VCardLine &line );
    VCardLine createSecrecy( const Secrecy &secrecy );


    QMap<QString, int> mAddressTypeMap;
    QMap<QString, int> mPhoneTypeMap;

    class VCardToolPrivate;
    VCardToolPrivate *d;
};

}

#endif
