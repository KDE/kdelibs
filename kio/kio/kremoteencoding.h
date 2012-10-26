/* This file is part of the KDE libraries
   Copyright (C) 2003 Thiago Macieira <thiago.macieira@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KREMOTEENCODING_H
#define KREMOTEENCODING_H

#include <kio/kio_export.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
class QUrl;

class KRemoteEncodingPrivate;
/**
 * Allows encoding and decoding properly remote filenames into Unicode.
 *
 * Certain protocols do not specify an appropriate encoding for decoding
 * their 8-bit data into proper Unicode forms. Therefore, ioslaves should
 * use this class in order to convert those forms into QStrings before
 * creating the respective KIO::UDSEntry. The same is true for decoding
 * URLs to its components.
 *
 * Each KIO::SlaveBase has one object of this kind, even if it is not necessary.
 * It can be accessed through KIO::SlaveBase::remoteEncoding.
 *
 * @short A class for handling remote filenames
 * @author Thiago Macieira <thiago.macieira@kdemail.net>
 */
class KIO_EXPORT KRemoteEncoding
{
public:
  /**
   * Constructor.
   *
   * Constructs this object to use the given encoding name.
   * If @p name is a null pointer, the standard encoding will be used.
   */
  explicit KRemoteEncoding(const char *name = 0);

  /**
   * Destructor
   */
  virtual ~KRemoteEncoding();

  /**
   * Converts the given full pathname or filename to Unicode.
   * This function is supposed to work for dirnames, filenames
   * or a full pathname.
   */
  QString decode(const QByteArray& name) const;

  /**
   * Converts the given name from Unicode.
   * This function is supposed to work for dirnames, filenames
   * or a full pathname.
   */
  QByteArray encode(const QString& name) const;

  /**
   * Converts the given URL into its 8-bit components
   */
  QByteArray encode(const QUrl& url) const;

  /**
   * Converts the given URL into 8-bit form and separate the
   * dirname from the filename. This is useful for slave functions
   * like stat or get.
   *
   * The dirname is returned with the final slash always stripped
   */
  QByteArray directory(const QUrl& url, bool ignore_trailing_slash = true) const;

  /**
   * Converts the given URL into 8-bit form and retrieve the filename.
   */
  QByteArray fileName(const QUrl& url) const;

  /**
   * Returns the encoding being used.
   */
  const char *encoding() const;

  /**
   * Returns the MIB for the codec being used.
   */
  int encodingMib() const;

  /**
   * Sets the encoding being used.
   * This function does not change the global configuration.
   *
   * Pass a null pointer in @p name to revert to the standard
   * encoding.
   */
  void setEncoding(const char* name);

protected:
  virtual void virtual_hook(int id, void* data);

private:
  KRemoteEncodingPrivate* const d;

  Q_DISABLE_COPY(KRemoteEncoding)
};

#endif
