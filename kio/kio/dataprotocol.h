//  dataprotocol.h
// ================
//
// Interface of the KDE data protocol core operations
//
// Author: Leo Savernik
// Email: l.savernik@aon.at
// Copyright (C) 2002 by Leo Savernik <l.savernik@aon.at>
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

/* Wondering what this is all about? Leo explained it to me:
 *
 * That's simple, you can compile it into a standalone executable that is
 * registered like any other kioslave.
 *
 * However, given that data-urls don't depend on any external data it seemed
 * overkill, therefore I added a special hack that the kio-dataslave is invoked
 * in-process on the client side.
 *
 * Hence, by defining DATAKIOSLAVE you can disable this special hack and compile
 * dataprotocol.* into a standalone kioslave.
 */

class QByteArray;

class QUrl;

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
  virtual void mimetype(const QUrl &url);
  virtual void get(const QUrl &url);
};

}/*end namespace*/

#endif
