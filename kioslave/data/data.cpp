//  data.cpp
// ============
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
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#include "data.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kurl.h>
#include <kio/global.h>

#include <qcstring.h>
#include <qstring.h>

#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace KIO;
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

/** attribute/value pairs */
typedef QMap<QCString,QCString> DataAttributes;

/** structure containing header information */
struct DataHeader {
  QCString mime_type;		// mime type of content (lowercase)
  DataAttributes attributes;	// attribute/value pairs (attribute lowercase,
  				// 	value unchanged)
  bool is_base64;		// true if data is base64 encoded
  QCString url;			// url translated to 8 bit
  int data_offset;		// zero-indexed position within url
  				// where the real data begins. May point beyond
      				// the end to indicate that there is no data
};

/** returns the position of the first occurrance of any of the given characters
  * @p c1 to @p c3 or buf.size() if none is contained.
  * @param buf buffer where to look for c
  * @param begin zero-indexed starting position
  * @param c1 character to find
  * @param c2 alternative character to find or '\0' to ignore
  * @param c3 alternative character to find or '\0' to ignore
  */
static int find(const QByteArray &buf, int begin, char c1, char c2 = '\0',
		char c3 = '\0') {
  int pos = begin;
  int size = (int)buf.size();
  while (pos < size) {
    char ch = buf[pos];
    if (ch == c1
    	|| (c2 != '\0' && ch == c2)
	|| (c3 != '\0' && ch == c3))
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
 * @param c2 alternative character to find or '\0' to ignore
 * @param c3 alternative character to find or '\0' to ignore
 */
inline QCString extract(const QByteArray &buf, int &pos, char c1,
		char c2 = '\0', char c3 = '\0') {
  int oldpos = pos;
  pos = find(buf,oldpos,c1,c2,c3);
  return QCString(buf.data() + oldpos, pos - oldpos + 1);
}

/** ignores all whitespaces
 * @param buf buffer to operate on
 * @param pos position to shift to first non-whitespace character
 *	Upon return @p pos will either point to the first non-whitespace
 *	character or to the end of the buffer.
 */
inline void ignoreWS(const QByteArray &buf, int &pos) {
  int size = (int)buf.size();
  char ch = buf[pos];
  while (pos < size && (ch == ' ' || ch == '\t' || ch == '\n'
  	|| ch == '\r'))
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
static QCString parseQuotedString(const QByteArray &buf, int &pos) {
  int size = (int)buf.size();
  QCString res;
  pos++;		// jump over leading quote
  bool escaped = false;	// if true means next character is literal
  bool parsing = true;	// true as long as end quote not found
  while (parsing && pos < size) {
    char ch = buf[pos++];
    if (escaped) {
      res += ch;
      escaped = false;
    } else {
      switch (ch) {
        case '"': parsing = false; break;
        case '\\': escaped = true; break;
        default: res += ch; break;
      }/*end switch*/
    }/*end if*/
  }/*wend*/
  return res;
}

/** parses the header of a data url
 * @param url the data url
 * @param header_info fills the given DataHeader structure with the header
 *		information
 */
static void parseDataHeader(const KURL &url, DataHeader &header_info) {
  // initialize header info members
  header_info.mime_type = "text/plain";
  header_info.attributes["charset"] = "us-ascii";
  header_info.is_base64 = false;
  
  // convert everything to local 8bit encoding
  QByteArray &raw_url = header_info.url = KURL::decode_string(url.url()).local8Bit();
  int raw_url_len = (int)raw_url.size();
  
  // jump over scheme part (must be "data:", we don't even check that)
  header_info.data_offset = raw_url.find(':');
  header_info.data_offset++;	// jump over colon or to begin if scheme was missing

  // read mime type
  if (header_info.data_offset >= raw_url_len) return;
  QCString mime_type = extract(raw_url,header_info.data_offset,';',',')
  			.stripWhiteSpace();
  if (!mime_type.isEmpty()) header_info.mime_type = mime_type;

  if (header_info.data_offset >= raw_url_len) return;
  // jump over delimiter token and return if data reached
  if (raw_url[header_info.data_offset++] == ',') return;

  // read all attributes and store them
  bool data_begin_reached = false;
  while (!data_begin_reached && header_info.data_offset < raw_url_len) {
    // read attribute
    QCString attribute = extract(raw_url,header_info.data_offset,'=',';',',')
    			.stripWhiteSpace();
    if (header_info.data_offset >= raw_url_len
    	|| raw_url[header_info.data_offset] != '=') {
      // no assigment, must be base64 option
      if (attribute == "base64") header_info.is_base64 = true;
    } else {
      header_info.data_offset++; // jump over '=' token
      
      // read value
      ignoreWS(raw_url,header_info.data_offset);
      if (header_info.data_offset >= raw_url_len) return;

      QCString value;
      if (raw_url[header_info.data_offset] == '"') {
        value = parseQuotedString(raw_url,header_info.data_offset);
        ignoreWS(raw_url,header_info.data_offset);
      } else
        value = extract(raw_url,header_info.data_offset,';',',')
        		.stripWhiteSpace();

      // add attribute to map
      header_info.attributes[attribute.lower()] = value;
        
    }/*end if*/
    if (header_info.data_offset < raw_url_len
	&& raw_url[header_info.data_offset] == ',')
      data_begin_reached = true;
    header_info.data_offset++; // jump over separator token
  }/*wend*/
}

DataProtocol::DataProtocol(const QCString &pool_socket, const QCString &app_socket)
#ifndef TESTKIO
	: SlaveBase("kio_data", pool_socket, app_socket) {
#else
	: TestSlave("kio_data", pool_socket, app_socket) {
#endif
  kdDebug() << "DataProtocol::DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

DataProtocol::~DataProtocol() {
  kdDebug() << "DataProtocol::~DataProtocol()" << endl;
}

/* --------------------------------------------------------------------- */

void DataProtocol::get(const KURL& url) {
  kdDebug() << "kio_data::get(const KURL& url)" << endl ;

  DataHeader hdr;
  parseDataHeader(url,hdr);

  int size = (int)hdr.url.size();
  int data_ofs = QMIN(hdr.data_offset,size);
  QByteArray inData;
  inData.setRawData(hdr.url.data()+data_ofs,size - data_ofs);
  QByteArray outData;
  QByteArray *theData;
  
  if (hdr.is_base64) {
    KCodecs::base64Decode(inData,outData);
    theData = &outData;
  } else theData = &inData;

  mimeType(hdr.mime_type);
  totalSize(theData->size());

  DataAttributes::ConstIterator it;
  for (it = hdr.attributes.begin(); it != hdr.attributes.end(); ++it) {
    setMetaData(it.key(),it.data());
  }/*next it*/
  
  data(*theData);
  data(QByteArray());
  finished();
  inData.resetRawData(hdr.url.data()+data_ofs,size - data_ofs);
}

/* --------------------------------------------------------------------- */

void DataProtocol::mimetype(const KURL &url) {
  DataHeader hdr;
  parseDataHeader(url,hdr);
  mimeType(hdr.mime_type);
  finished();
}

/* --------------------------------------------------------------------- */

