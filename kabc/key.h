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

#ifndef KABC_KEY_H
#define KABC_KEY_H

#include <qvaluelist.h>

namespace KABC {

/**
 * @short A class to store an encryption key.
 */
class Key
{
  friend QDataStream &operator<<( QDataStream &, const Key & );
  friend QDataStream &operator>>( QDataStream &, Key & );

public:
  typedef QValueList<Key> List;
  typedef QValueList<int> TypeList;  

  /**
   * Key types
   *
   * @li X509   - X509 key
   * @li PGP    - Pretty Good Privacy key
   * @li Custom - Custom or IANA conform key
   */
  enum Types {
    X509,
    PGP,
    Custom
  };

  /**
   * Constructor.
   *
   * @param text  The text data.
   * @param type  The key type, @see Types.
   */
  Key( const QString &text = QString::null, int type = PGP );

  /**
   * Constructor.
   *
   * @param binary  The binary data.
   * @param type    The key type, @see Types.
   */
  Key( const QCString &binary = 0, int type = PGP );

  /**
   * Destructor.
   */
  ~Key();
    
  bool operator==( const Key & ) const;
  bool operator!=( const Key & ) const;

  /**
   * Sets the unique identifier.
   */
  void setId( const QString &id );

  /**
   * Returns the unique identifier.
   */
  QString id() const;

  /**
   * Sets binary data.
   */
  void setBinaryData( const QCString &binary );

  /**
   * Returns the binary data.
   */
  QCString binaryData() const;

  /**
   * Sets text data.
   */
  void setTextData( const QString &text );

  /**
   * Returns the text data.
   */
  QString textData() const;

  /**
   * Returns whether the key contains binary or text data.
   */
  bool isBinary() const;

  /**
   * Sets the type, @see Type.
   */
  void setType( int type );

  /**
   * Sets custom type string.
   */
  void setCustomTypeString( const QString &custom );

  /**
   * Returns the type, @see Type.
   */
  int type() const;

  /**
   * Returns the custom type string.
   */
  QString customTypeString() const;

  /**
   * Returns a list of all available key types.
   */
  static TypeList typeList();
  
  /**
   * Returns a translated label for a given key type.
   */
  static QString typeLabel( int type );

private:
  QCString mBinaryData;
  QString mId;
  QString mTextData;
  QString mCustomTypeString;

  int mIsBinary;
  int mType;
};

QDataStream &operator<<( QDataStream &, const Key & );
QDataStream &operator>>( QDataStream &, Key & );

}
#endif
