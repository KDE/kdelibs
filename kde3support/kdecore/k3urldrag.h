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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KUrlDRAG_H
#define KUrlDRAG_H

#include <kde3support_export.h>
#include <kurl.h>

#include <QtCore/QStringList>
#include <Qt3Support/Q3ColorDrag>

class QMimeSource;

class K3URLDragPrivate;
/**
 * This class is to be used instead of Q3UriDrag when using KUrl.
 * The reason is: Q3UriDrag (and the XDND/W3C standards) expect URLs to
 * be encoded in UTF-8 (unicode), but KUrl uses the current locale
 * by default.
 * The other reasons for using this class are:
 * @li it exports text/plain (for dropping/pasting into lineedits, mails etc.)
 * @li it has support for metadata, shipped as part of the dragobject
 * This is important, for instance to set a correct HTTP referrer (some websites
 * require it for downloading e.g. an image).
 *
 * To create a drag object, use the K3URLDrag constructor.
 * To handle drops, use Q3UriDrag::canDecode() and K3URLDrag::decode()
 */
class KDE3SUPPORT_EXPORT K3URLDrag : public Q3UriDrag
{
public:
  /**
   * Constructs an object to drag the list of URLs in @p urls.
   * The @p dragSource and @p name arguments are passed on to Q3UriDrag,
   * and the list of urls is converted to UTF-8 before being passed
   * to Q3UriDrag.
   * @param urls the list of URLs
   * @param dragSource the parent of the QObject. Should be set when doing drag-n-drop,
   * but should be 0 when copying to the clipboard
   */
  K3URLDrag( const KUrl::List &urls, QWidget* dragSource = 0 );
  /**
   * Constructs an object to drag the list of URLs in @p urls.
   * This version also includes metadata.
   * @param urls the list of URLs
   * @param metaData a map containing meta data
   * @param dragSource the parent of the QObject. Should be set when doing drag-n-drop,
   * but should be 0 when copying to the clipboard
   * @see metaData()
   */
  K3URLDrag( const KUrl::List &urls, const QMap<QString, QString>& metaData,
            QWidget* dragSource = 0 );

  virtual ~K3URLDrag();

  /**
   * By default, K3URLDrag also exports the URLs as plain text, for e.g. dropping onto a text editor.
   * But in some cases this might not be wanted, e.g. if using the K3URLDrag in a KMultipleDrag
   * and another component of the multiple-drag provides better plain text data.
   * In such a case, setExportAsText( false ) should be called.
   */
  void setExportAsText( bool exp );

  /**
   * @deprecated Is equivalent with "new K3URLDrag(urls, dragSource, name)".
   */
  static K3URLDrag * newDrag( const KUrl::List &urls, QWidget* dragSource = 0 );

  /**
   * @deprecated Is equivalent with "new K3URLDrag(urls, metaData, dragSource, name)".
   */
  static K3URLDrag * newDrag( const KUrl::List &urls,
                             const QMap<QString, QString>& metaData,
                             QWidget* dragSource = 0 );

  /**
   * Meta-data to associate with those URLs.
   * This is an alternative way of setting the metadata:
   * either use the constructor to pass it all at once, or use
   * drag->metaData()["key"] = data;
   * @see KIO::TransferJob
   */
  QMap<QString, QString> &metaData();

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KUrls. Decoding will fail if at least one decoded value
   * is not a valid KUrl.
   * @param e the mime source
   * @param urls the list of urls will be written here
   * @return true if successful, false otherwise
   */
  static bool decode( const QMimeSource *e, KUrl::List &urls );

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KUrls and a set of metadata. Decoding will fail if
   * at least one decoded value is not a valid KUrl.
   * You should be using this one, if possible.
   * @param e the mime source
   * @param urls the list of urls will be written here
   * @param metaData the metadata map will be written here
   * @return true if successful, false otherwise
   */
  static bool decode( const QMimeSource *e, KUrl::List &urls, QMap<QString,QString>& metaData );

  /**
   * Converts a URL to a string representation suitable for dragging.
   */
  static QString urlToString(const KUrl &url);

  /**
   * Converts a string used for dragging to a URL.
   */
  static KUrl stringToUrl(const QByteArray &s);

#ifdef Q_WS_QWS
  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KUrls for Qt versions without a MIME clipboard.
   * Decoding will fail if at least one value in the list is not a valid KUrl.
   */
  static bool decode( QStringList const &e, KUrl::List &uris );
#endif

  /// @reimp
  virtual const char * format( int i ) const;
  /// @reimp
  virtual QByteArray encodedData( const char* mime ) const;

protected:
  /**
   * @deprecated Use a K3URLDrag constructor with a KUrl::List
   */
  K3URLDrag( const Q3StrList & urls, const QMap<QString,QString>& metaData,
            QWidget * dragSource );

private:
  void init(const KUrl::List &urls);

  Q3StrList m_urls;
  QMap<QString,QString> m_metaData;
  K3URLDragPrivate* d;
};

#endif

