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


#include "kcardreader.h"
#include "kpcsc.h"
#include <winscard.h>
#include <stdlib.h>


KCardReader::KCardReader() {
	_transacting = false;
}


KCardReader::~KCardReader() {
	cancelTransaction();
	SCardDisconnect(_card, SCARD_RESET_CARD);
}


void KCardReader::setCard(long ctx, QString name, long card, unsigned long protocol) {
   _ctx = ctx;
   _name = name;
   _card = card;
   _protocol = protocol;
}


bool KCardReader::isCardPresent() {
long rc;
unsigned long state;

	rc = SCardStatus(_card, NULL, NULL, &state, &_protocol, NULL, NULL);
	if (rc != SCARD_S_SUCCESS) {
		return false;
	}

return true;
}


bool KCardReader::inTransaction() {
	return _transacting;
}


int KCardReader::beginTransaction() {
long rc;

	if (_transacting) return 1;

	rc = SCardBeginTransaction(_card);
	if (rc != SCARD_S_SUCCESS) return rc;

	_transacting = true;

return 0;
}


int KCardReader::endTransaction() {
long rc;

	if (!_transacting) return 1;

	rc = SCardEndTransaction(_card, 0);
	_transacting = false;

	if (rc != SCARD_S_SUCCESS) return rc;

return 0;
}


int KCardReader::cancelTransaction() {
	if (!_transacting) return 1;

	SCardCancelTransaction(_card);
	_transacting = false;
return 0;
}



