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
#include <klocale.h>
#include <stdlib.h>
#include <winscard.h>

#include <kdebug.h>

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
		_connected = false;
		
		return rc;
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
QString readerName=QString::null;
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

	
	
	for (int i=0;i<readers-1;i++){

		if (rstr[i]=='\0')
		 { 
		   res << readerName;
		   readerName=QString::null;
		   continue;
		 }
		readerName+= rstr[i];
	}

       
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


KCardCommand KPCSC::encodeCommand(const QString command) {
KCardCommand x(0);

	if (command.length() % 2)
		return x;

	x.resize(command.length()/2);

	for (unsigned int n = 0, j = 0; j < command.length(); n++, j += 2) {
		unsigned char Hnib = QChar(command.at(j)).upper();
		unsigned char Lnib = QChar(command.at(j+1)).upper();

		Hnib -= (Hnib <= 57) ? 48 : 55;
		Lnib -= (Lnib <= 57) ? 48 : 55;

		if (Hnib <= 0x0f && Lnib <= 0x0f) {
			x[n] = Hnib;
			x[n] <<= 4;
			x[n] |= Lnib;
		} else {
			return KCardCommand(0);
		}
	}

	//x[x.size()-1] = 0;
	
return x;
}


QString KPCSC::decodeCommand(const KCardCommand command) {
QString x;

	for (unsigned int i = 0; i < command.size(); i++) {
		unsigned short y = command[i] & 0x00ff;

		if (y <= 0x0f) {
			x += "0";
		}

		x += QString::number(y,16).upper();
	}

return x;
}

QString KPCSC::translateError (const long error){

  QString j(pcsc_stringify_error(error));
  
  return j;


}

long KPCSC::context() {
return _ctx;
}



