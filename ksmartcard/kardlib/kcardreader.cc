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
#include <winscard.h>
#include <stdlib.h>
#include <kdebug.h>


KCardReader::KCardReader() {
	_transacting = false;
	_state = 0;
	_atrLen = 0;
	_readerLen = 0;
	
}


KCardReader::~KCardReader() {
	cancelTransaction();
	//SCardDisconnect(_card, SCARD_RESET_CARD);
	SCardDisconnect(_card, SCARD_UNPOWER_CARD);
}

int KCardReader::resetCard() {

  long rc;
  rc=SCardDisconnect(_card, SCARD_RESET_CARD);
  if (rc != SCARD_S_SUCCESS) {
    return rc;
  }
  return 0;
}

void KCardReader::setCard(long ctx, QString name, long card, unsigned long protocol) {
   _ctx = ctx;
   _name = name;
   _card = card;
   _protocol = protocol;
}


bool KCardReader::isCardPresent() {
long rc = 0;

	rc = SCardStatus(_card, NULL, &_readerLen, &_state, &_protocol, NULL, &_atrLen);
	if (rc != SCARD_S_SUCCESS) {
	  
	  return false;
	}

	/*
	if (!(state & SCARD_STATE_PRESENT)) {
		return false;
	}
	*/

return true;
}


bool KCardReader::inTransaction() {
	return _transacting;
}


int KCardReader::beginTransaction() {
long rc;

	if (_transacting) return 1;

	rc = SCardBeginTransaction(_card);
	if (rc != SCARD_S_SUCCESS){

	  return rc;
	}

	_transacting = true;

return 0;
}


int KCardReader::endTransaction() {
long rc;

	if (!_transacting) return 1;

	rc = SCardEndTransaction(_card, 0);
	_transacting = false;

	if (rc != SCARD_S_SUCCESS){

	  return rc;
	}

 

return 0;
}


int KCardReader::cancelTransaction() {
	if (!_transacting) return 1;

	SCardCancelTransaction(_card);
	_transacting = false;
return 0;
}

int KCardReader::doCommand(QString command, QString& response, QString & status) {
  int rc;
  KCardCommand resp;

  
  rc = doCommand(command,response);
  status=response.right(4);
  response=response.mid(0,response.length()-4);
  

  
  return rc;
}


int KCardReader::doCommand(QString command, QString& response) {
  int rc;
  KCardCommand resp;
  
  rc = doCommand(KPCSC::encodeCommand(command), resp);
  response = KPCSC::decodeCommand(resp);
  


  return rc;
}

int KCardReader::doCommand(KCardCommand command, KCardCommand& response, KCardCommand & status) {
  
  int rc;
  rc=doCommand(command,response);
  for (unsigned int i=0;i<2;i++){
    status[0+i]=response[response.size()-i];
  }
  response.resize(response.size()-2);
  
  return rc;
  
}

int KCardReader::doCommand(KCardCommand command, KCardCommand& response) {
long rc = 0;
unsigned long resSize = MAX_BUFFER_SIZE*sizeof(unsigned char);
unsigned char res[MAX_BUFFER_SIZE];
SCARD_IO_REQUEST _out;        // hmm what to do with this

	if (command.size() == 0)
		return -2;


;

	rc = SCardTransmit(_card, 
			   SCARD_PCI_T0,
			   command.data(),
			   command.size(),
			   &_out,
			   res,
			   &resSize);


	response.duplicate(res, resSize);
	kdDebug()<<"main command " << KPCSC::decodeCommand(command) << endl;
	kdDebug()<<"main  resp " << KPCSC::decodeCommand(response) << endl;

	if (rc != SCARD_S_SUCCESS) {
		response.resize(0);
		return -1;
	}

	response.duplicate(res, resSize);
	
return 0;
}
