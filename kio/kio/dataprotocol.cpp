//  dataprotocol.cpp
// ==================
//
// Implementation of the data protocol (rfc 2397)
//
// Author: Leo Savernik
// Email: l.savernik@aon.at
// (C) 2002, 2003 by Leo Savernik
// Created: Sam Dez 28 14:11:18 CET 2002

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; version 2.                 *
 *                                                                         *
 ***************************************************************************/

#include "dataprotocol.h"

#include <kdebug.h>
#include <kcodecs.h>
#include <kurl.h>
#include <kio/global.h>
#include <kglobal.h>

#include <qbytearray.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextcodec.h>

#ifdef DATAKIOSLAVE
#  include <kinstance.h>
#  include <stdlib.h>
#endif

#if !defined(DATAKIOSLAVE) && !defined(TESTKIO)
#  define DISPATCH(f) dispatch_##f
#else
#  define DISPATCH(f) f
#endif

using namespace KIO;
#ifdef DATAKIOSLAVE
extern "C" {

  int kdemain( int argc, char **argv ) {
    KInstance instance( "kio_data" );

    kdDebug(7101) << "*** Starting kio_data " << endl;

    if (argc != 4) {
      kdDebug(7101) << "Usage: kio_data  protocol domain-socket1 domain-socket2" << endl;
      exit(-1);
    }

    DataProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kdDebug(7101) << "*** kio_data Done" << endl;
    return 0;
  }
}
#endif

/** structure containing header information */
struct DataHeader {
  QString mime_type;		// mime type of content (lowercase)
  MetaData attributes;		// attribute/value pairs (attribute lowercase,
  				// 	value unchanged)
  bool is_base64;		// true if data is base64 encoded
  QString url;			// reference to decoded url
  int data_offset;		// zero-indexed position within url
  				// where the real data begins. May point beyond
      				// the end to indicate that there is no data
  QString charset;		// shortcut to charset (it always exists)
};

/** returns the position of the first occurrence of any of the given characters
  * @p c1 to @p c3 or buf.length() if none is contained.
  * @param buf buffer where to look for c
  * @param begin zero-indexed starting position
  * @param c1 character to find
  * @param c2 alternative character to find or '\0' to ignore
  * @param c3 alternative character to find or '\0' to ignore
  */
static int find(const QString &buf, int begin, QChar c1,
		QChar c2 = QLatin1Char('\0'), QChar c3 = QLatin1Char('\0')) {
  int pos = begin;
  int size = buf.length();
  while (pos < size) {
    QChar ch = buf[pos];
    if (ch == c1
    	|| (c2 != QLatin1Char('\0') && ch == c2)
	|| (c3 != QLatin1Char('\0') && ch == c3))
      break;
    pos++;
  }/*wend*/
  return pos;
}

/** extracts the string between the current position @p pos and the first
 * occurrence of either @p c1 to @p c3 exclusively and updates @p pos
 * to point at the found delimiter or at the end of the buffer if
 * neither character occurred.
 * @param buf buffer where to look for
 * @param pos zero-indexed position within buffer
 * @param c1 character to find
 * @param c2 alternative character to find or 0 to ignore
 * @param c3 alternative character to find or 0 to ignore
 */
inline QString extract(const QString &buf, int &pos, QChar c1,
		QChar c2 = QLatin1Char('\0'), QChar c3 = QLatin1Char('\0')) {
  int oldpos = pos;
  pos = find(buf,oldpos,c1,c2,c3);
  return buf.mid(oldpos, pos-oldpos);
}

/** ignores all whitespaces
 * @param buf buffer to operate on
 * @param pos position to shift to first non-whitespace character
 *	Upon return @p pos will either point to the first non-whitespace
 *	character or to the end of the buffer.
 */
inline void ignoreWS(const QString &buf, int &pos) {
  int size = buf.length();
  QChar ch = buf[pos];
  while (pos < size && ch.isSpace())
    ch = buf[++pos];
}

/** parses a quoted string as per rfc 822.
 *
 * If trailing quote is missing, the whole rest of the buffer is returned.
 * @param buf buffer to operate on
 * @param pos position pointing to the leading quote
 * @return the extracted string. @p pos will be updated to point to the
 * 	character following the trailing quote.
 */
static QString parseQuotedString(const QString &buf, int &pos) {
  int size = buf.length();
  QString res;
  res.reserve(size);    // can't be larger than buf
  pos++;		// jump over leading quote
  bool escaped = false;	// if true means next character is literal
  bool parsing = true;	// true as long as end quote not found
  while (parsing && pos < size) {
    QChar ch = buf[pos++];
    if (escaped) {
      res += ch;
      escaped = false;
    } else {
      switch (ch.unicode()) {
        case '"': parsing = false; break;
        case '\\': escaped = true; break;
        default: res += ch; break;
      }/*end switch*/
    }/*end if*/
  }/*wend*/
  res.squeeze();
  return res;
}

/** parses the header of a data url
 * @param url the data url
 * @param header_info fills the given DataHeader structure with the header
 *		information
 */
static void parseDataHeader(const KURL &url, DataHeader &header_info) {
  static const QString& text_plain = KGlobal::staticQString("text/plain");
  static const QString& charset = KGlobal::staticQString("charset");
  static const QString& us_ascii = KGlobal::staticQString("us-ascii");
  static const QString& base64 = KGlobal::staticQString("base64");
  
  // initialize header info members
  header_info.mime_type = text_plain;
  header_info.charset = header_info.attributes.insert(charset,us_ascii).value();
  header_info.is_base64 = false;

  // decode url and save it
  QString &raw_url = header_info.url = KURL::decode_string(url.url());
  int raw_url_len = raw_url.length();

  // jump over scheme part (must be "data:", we don't even check that)
  header_info.data_offset = raw_url.indexOf(QLatin1Char(':'));
  header_info.data_offset++;	// jump over colon or to begin if scheme was missing

  // read mime type
  if (header_info.data_offset >= raw_url_len) return;
  QString mime_type = extract(raw_url, header_info.data_offset,
  			      QLatin1Char(';'), QLatin1Char(',')).trimmed();
  if (!mime_type.isEmpty()) header_info.mime_type = mime_type;

  if (header_info.data_offset >= raw_url_len) return;
  // jump over delimiter token and return if data reached
  if (raw_url[header_info.data_offset++] == QLatin1Char(',')) return;

  // read all attributes and store them
  bool data_begin_reached = false;
  while (!data_begin_reached && header_info.data_offset < raw_url_len) {
    // read attribute
    QString attribute = extract(raw_url, header_info.data_offset,
    				QLatin1Char('='), QLatin1Char(';'),
				QLatin1Char(',')).trimmed();
    if (header_info.data_offset >= raw_url_len
    	|| raw_url[header_info.data_offset] != QLatin1Char('=')) {
      // no assigment, must be base64 option
      if (attribute == base64)
        header_info.is_base64 = true;
    } else {
      header_info.data_offset++; // jump over '=' token

      // read value
      ignoreWS(raw_url,header_info.data_offset);
      if (header_info.data_offset >= raw_url_len) return;

      QString value;
      if (raw_url[header_info.data_offset] == QLatin1Char('"')) {
        value = parseQuotedString(raw_url,header_info.data_offset);
        ignoreWS(raw_url,header_info.data_offset);
      } else
        value = extract(raw_url, header_info.data_offset, QLatin1Char(';'),
			QLatin1Char(',')).trimmed();

      // add attribute to map
      header_info.attributes[attribute.toLower()] = value;

    }/*end if*/
    if (header_info.data_offset < raw_url_len
	&& raw_url[header_info.data_offset] == QLatin1Char(','))
      data_begin_reached = true;
    header_info.data_offset++; // jump over separator token
  }/*wend*/
}

#ifdef DATAKIOSLAVE
DataProtocol::DataProtocol(const QByteArray &pool_socket, const QByteArray &app_socket)
	: SlaveBase("kio_data", pool_socket, app_socket) {
#else
DataProtocol::DataProtocol() {
#endif
  kdDebug() << "DataProtocol::DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

DataProtocol::~DataProtocol() {
  kdDebug() << "DataProtocol::~DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

void DataProtocol::get(const KURL& url) {
  ref();
  //kdDebug() << "===============================================================================================================================================================================" << endl;
  kdDebug() << "kio_data@"<<this<<"::get(const KURL& url)" << endl ;

  DataHeader hdr;
  parseDataHeader(url,hdr);

  int size = hdr.url.length();
  int data_ofs = qMin(hdr.data_offset,size);
  // FIXME: string is copied, would be nice if we could have a reference only
  QString url_data = hdr.url.mid(data_ofs);
  QByteArray outData;

  if (hdr.is_base64) {
    // base64 stuff is expected to contain the correct charset, so we just
    // decode it and pass it to the receiver
    KCodecs::base64Decode(url_data.toLocal8Bit(),outData);
  } else {
    // FIXME: This is all flawed, must be reworked thoroughly
    // non encoded data must be converted to the given charset
    QTextCodec *codec = QTextCodec::codecForName(hdr.charset.toLatin1());
    if (codec != 0) {
      outData = codec->fromUnicode(url_data);
    } else {
      // if there is no approprate codec, just use local encoding. This
      // should work for >90% of all cases.
      outData = url_data.toLocal8Bit();
    }/*end if*/
  }/*end if*/

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit mimeType@"<<this << endl ;
  mimeType(hdr.mime_type);
  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit totalSize@"<<this << endl ;
  totalSize(outData.size());

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit setMetaData@"<<this << endl ;
#if defined(TESTKIO) || defined(DATAKIOSLAVE)
  MetaData::ConstIterator it;
  for (it = hdr.attributes.begin(); it != hdr.attributes.end(); ++it) {
    setMetaData(it.key(),it.data());
  }/*next it*/
#else
  setAllMetaData(hdr.attributes);
#endif

  //kdDebug() << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
  //kdDebug() << "emit sendMetaData@"<<this << endl ;
  sendMetaData();
  //kdDebug() << "^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C^[[C" << endl;
//   kdDebug() << "(1) queue size " << dispatchQueue.size() << endl;
  // empiric studies have shown that this shouldn't be queued & dispatched
  data(outData);
//   kdDebug() << "(2) queue size " << dispatchQueue.size() << endl;
  DISPATCH(data(QByteArray()));
//   kdDebug() << "(3) queue size " << dispatchQueue.size() << endl;
  DISPATCH(finished());
//   kdDebug() << "(4) queue size " << dispatchQueue.size() << endl;
  deref();
}

/* --------------------------------------------------------------------- */

void DataProtocol::mimetype(const KURL &url) {
  ref();
  DataHeader hdr;
  parseDataHeader(url,hdr);
  mimeType(hdr.mime_type);
  finished();
  deref();
}

/* --------------------------------------------------------------------- */
