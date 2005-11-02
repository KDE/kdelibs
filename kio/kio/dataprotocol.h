//  dataprotocol.h
// ================
//
// Interface of the KDE data protocol core operations
//
// Author: Leo Savernik
// Email: l.savernik@aon.at
// (C) 2002 by Leo Savernik
// Created: Sam Dez 28 14:11:18 CET 2002

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; version 2.                 *
 *                                                                         *
 ***************************************************************************/

#ifndef DATAPROTOCOL_H
#define DATAPROTOCOL_H

// dataprotocol.* interprets the following defines
// DATAKIOSLAVE: define if you want to compile this into a stand-alone
//		kioslave
// TESTKIO: define for test-driving
// Both defines are mutually exclusive. Defining none of them compiles
// DataProtocol for internal usage within libkiocore.

class QString;
class QByteArray;

class KURL;

#if defined(DATAKIOSLAVE)
#  include <kio/slavebase.h>
#elif !defined(TESTKIO)
#  include "kio/dataslave.h"
#endif

namespace KIO {

/** This kioslave provides support of data urls as specified by rfc 2397
 * @see http://www.ietf.org/rfc/rfc2397.txt
 * @author Leo Savernik
 */
#if defined(DATAKIOSLAVE)
class DataProtocol : public KIO::SlaveBase {
#elif defined(TESTKIO)
class DataProtocol : public TestSlave {
#else
class DataProtocol : public DataSlave {
#endif

public:
#if defined(DATAKIOSLAVE)
  DataProtocol(const QByteArray &pool_socket, const QByteArray &app_socket);
#else
  DataProtocol();
#endif
  virtual ~DataProtocol();
  virtual void mimetype(const KURL &url);
  virtual void get(const KURL &url);
#if defined(TESTKIO)
  void ref() {}
  void deref() {}
#endif
};

}/*end namespace*/

#endif
