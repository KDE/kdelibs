/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KURLDRAG_H
#define __KURLDRAG_H

#include <qstringlist.h>
#include <qdragobject.h>
#include <kurl.h>
class QMimeSource;

/**
 * This class is to be used instead of QUriDrag when using KURL.
 * The reason is : QUriDrag (and the XDND/W3C standards) expect URLs to
 * be encoded in UTF-8 (unicode), but KURL uses the current locale
 * by default.
 * The other reasons for using this class are:
 * - it exports text/plain (for dropping/pasting into lineedits, mails etc.)
 * - it has support for metadata, shipped as part of the dragobject
 * This is important, for instance to set a correct HTTP referrer (some websites
 * require it for downloading e.g. an image).
 *
 * To create a drag object, use KURLDrag::newDrag with a list of KURLs.
 * To decode a drop, use KURLDrag::decode or QUriDrag::decodeLocalFiles.
 */
class KURLDrag : public QUriDrag
{
public:
  /**
   * Constructs an object to drag the list of URLs in urls.
   * The dragSource and name arguments are passed on to QUriDrag,
   * and the list of urls is converted to UTF-8 before being passed
   * to QUriDrag.
   */
  static KURLDrag * newDrag( const KURL::List &urls, QWidget* dragSource = 0, const char * name = 0 );

  /**
   * Constructs an object to drag the list of URLs in urls.
   * This version also includes metadata
   */
  static KURLDrag * newDrag( const KURL::List &urls, const QMap<QString, QString>& metaData,
                             QWidget* dragSource = 0, const char * name = 0 );

  /**
   * Meta-data to associate with those URLs (to be used after newDrag)
   * @see KIO::TransferJob etc.
   * This is an alternative way of setting the metadata:
   * either use the constructor to pass it all at once, or use
   * drag->metaData()["key"] = data;
   */
  QMap<QString, QString> &metaData() { return m_metaData; }

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs.
   */
  static bool decode( const QMimeSource *e, KURL::List &urls );

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs and a set of metadata.
   * You should be using this one, if possible.
   */
  static bool decode( const QMimeSource *e, KURL::List &urls, QMap<QString,QString>& metaData );

#ifdef Q_WS_QWS
  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs for Qt versions without a MIME clipboard.
   */
  static bool decode( QStringList const &e, KURL::List &uris );
#endif

protected:
  /**
   * Protected constructor - use @ref newDrag
   */
  KURLDrag( const QStrList & urls, const QMap<QString,QString>& metaData,
            QWidget * dragSource, const char* name ) :
      QUriDrag( urls, dragSource, name ), m_urls( urls ), m_metaData( metaData ) {}

  virtual const char * format( int i ) const;
  virtual QByteArray encodedData( const char* mime ) const;

private:
  QStrList m_urls;
  QMap<QString,QString> m_metaData;
};

#endif

