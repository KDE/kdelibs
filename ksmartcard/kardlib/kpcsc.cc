/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#include "kpcsc.h"
#include "kcardreader.h"
#include <stdlib.h>
#include <winscard.h>

KPCSC::KPCSC(bool autoConnect) {
   _connected = false;
   if (autoConnect) connect();
}


KPCSC::~KPCSC() {
   disconnect();
}


int KPCSC::connect() {
long rc;

   if (_connected) return 0;

   rc = SCardEstablishContext(SCARD_SCOPE_SYSTEM, 0, 0, &_ctx);

   if (rc != SCARD_S_SUCCESS) {
	return rc;
   	_connected = false;
   }

   _connected = true;
return 0;
}


int KPCSC::disconnect() {
long rc;

          if (!_connected) return 0;

	  rc = SCardReleaseContext(_ctx);
	  if (rc != SCARD_S_SUCCESS) {
		  return rc;
	  }

	  _connected = false;
return 0;
}


int KPCSC::reconnect() {
	disconnect();
return connect();
}


QStringList KPCSC::listReaders(int *err) {
QStringList res;
unsigned long readers;
char *rstr;
long rc;

	if (!_connected) {
		if (err) *err = -1;
		return res;
	}

	rc = SCardListReaders(_ctx, (char *)NULL, 0, &readers);
	if (rc != SCARD_S_SUCCESS) {
		if (err) *err = rc;
		return res;
	}

	rstr = new char[readers];
	rc = SCardListReaders(_ctx, (char *)NULL, rstr, &readers);
	if (rc != SCARD_S_SUCCESS) {
		if (err) *err = rc;
		delete[] rstr;
		return res;
	}

	res << rstr;
	delete[] rstr;
	if (err) *err = 0;

return res;
}


KCardReader* KPCSC::getReader(QString x) {
long rc;
KCardReader *cr = NULL;
long card;
unsigned long protocol;

   rc = SCardConnect(_ctx, x.local8Bit(), SCARD_SHARE_EXCLUSIVE,
		                          SCARD_PROTOCOL_ANY,
					  &card, &protocol);
   if (rc == SCARD_S_SUCCESS) {
      cr = new KCardReader;
      cr->setCard(_ctx, x, card, protocol);
   }

return cr;
}


