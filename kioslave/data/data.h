//  data.h
// ============
//
// Interface of the KDE data protocol kioslave
//
// Author: Leo Savernik
// Email: l.savernik@aon.at
// (C) 2002 by Leo Savernik
// Created: Sam Dez 28 14:11:18 CET 2002

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef __data_h__
#define __data_h__

class QString;
class QCString;

class KURL;

#include <kio/slavebase.h>

/** This kioslave provides support of data urls as specified by rfc 2397
 * @see http://www.ietf.org/rfc/rfc2397.txt
 * @author Leo Savernik
 */
#ifndef TESTKIO
class DataProtocol : public KIO::SlaveBase {
#else
class DataProtocol : public TestSlave {
#endif

public:
  DataProtocol(const QCString &pool_socket, const QCString &app_socket);
  virtual ~DataProtocol();
  virtual void mimetype(const KURL &url);
  virtual void get(const KURL &url);
};


#endif
