/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Leo Savernik <l.savernik@aon.at>
 *  Derived from slave.cpp
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>

#include "dataslave.h"

#include <klocale.h>

using namespace KIO;

DataSlave::DataSlave() :
	Slave(true, 0, QString::null, QString::null)
{
}

DataSlave::~DataSlave() {
}

void DataSlave::hold(const KURL &/*url*/) {
  // ignored
}

void DataSlave::suspend() {
  // it's impossible to suspend data urls
}

void DataSlave::resume() {
  // resume not possible for data urls
}

void DataSlave::send(int cmd, const QByteArray &arr) {
  QDataStream stream(arr, IO_ReadOnly);

  KURL url;

  switch (cmd) {
    case CMD_GET: {
      stream >> url;
      get(url);
      break;
    }
    case CMD_MIMETYPE: {
      stream >> url;
      mimetype(url);
      break;
    }
    // ignore these (must not emit error, otherwise SIGSEGV occurs)
    case CMD_META_DATA:
    case CMD_SUBURL:
      break;
    default:
      error(ERR_UNSUPPORTED_ACTION,
		unsupportedActionErrorString(QString::fromLatin1("data"),cmd));
  }/*end switch*/
}

bool DataSlave::suspended() {
  // cannot happen therefore always false
  return false;
}

void DataSlave::setHost(const QString &/*host*/, int /*port*/,
                     const QString &/*user*/, const QString &/*passwd*/) {
  // irrelevant -> will be ignored
}

void DataSlave::setConfig(const MetaData &/*config*/) {
  // FIXME: decide to handle this directly or not at all
#if 0
    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << config;
    slaveconn.send( CMD_CONFIG, data );
#endif
}

void DataSlave::setAllMetaData(const MetaData &md) {
  meta_data = md;
}

void DataSlave::sendMetaData() {
  emit metaData(meta_data);
}

void DataSlave::virtual_hook( int id, void* data ) {
  switch (id) {
    case VIRTUAL_SUSPEND: suspend(); return;
    case VIRTUAL_RESUME: resume(); return;
    case VIRTUAL_SEND: {
      SendParams *params = reinterpret_cast<SendParams *>(data);
      send(params->cmd, *params->arr);
      return;
    }
    case VIRTUAL_HOLD: {
      HoldParams *params = reinterpret_cast<HoldParams *>(data);
      hold(*params->url);
      return;
    }
    case VIRTUAL_SUSPENDED: {
      SuspendedParams *params = reinterpret_cast<SuspendedParams *>(data);
      params->retval = suspended();
      return;
    }
    case VIRTUAL_SET_HOST: {
      SetHostParams *params = reinterpret_cast<SetHostParams *>(data);
      setHost(*params->host,params->port,*params->user,*params->passwd);
      return;
    }
    case VIRTUAL_SET_CONFIG: {
      SetConfigParams *params = reinterpret_cast<SetConfigParams *>(data);
      setConfig(*params->config);
      return;
    }
    default:
      KIO::Slave::virtual_hook( id, data );
  }
}

#include "dataslave.moc"
