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
 * The other reason for using this class is that it exports text/plain
 * (for dropping/pasting into lineedits, mails etc.)
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
  static QUriDrag * newDrag( const KURL::List &urls, QWidget* dragSource = 0, const char * name = 0 );

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs.
   */
  static bool decode( const QMimeSource *e, KURL::List &urls );

#ifdef _WS_QWS_
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
  KURLDrag( const QStrList & urls, QWidget * dragSource, const char* name ) :
    QUriDrag( urls, dragSource, name ), m_urls( urls ) {}

  virtual const char * format( int i ) const;
  virtual QByteArray encodedData( const char* mime ) const;

private:
  QStrList m_urls;
};

#endif

