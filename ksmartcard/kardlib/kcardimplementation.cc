/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat < >
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


#include "kcardimplementation.h"
#include "kcardreader.h"

#include <kdebug.h>
KCardImplementation::KCardImplementation(const QString & type, 
					 const QString & subType,
					 const QString & subSubType)
{

 
  _type = type;
  _subType =subType ;
  _subSubType =subSubType ;
  _errno = 0;
  _kcardreader = NULL;
  pcscInt = new KPCSC(TRUE);
}


KCardImplementation::~KCardImplementation() {
  if  (_kcardreader) delete _kcardreader;
  delete pcscInt;
  
}

int KCardImplementation::init ( const QString & selectedReader){

  
  _kcardreader = pcscInt->getReader(selectedReader);

  if (_kcardreader==NULL) return 1;
  
  return 0;
}

int KCardImplementation::matchATR(KCardCommand atr) {
return -1;
}


int KCardImplementation::selectFile(const QString) {
return -1;
}

int KCardImplementation::selectDirectory(const QString) {
return -1;
}

KCardCommand KCardImplementation::getCardSerialNumber() {
return KCardCommand();
}


const QString& KCardImplementation::getType() const {
	return _type;
}

const QString& KCardImplementation::getSubType() const {
	return _subType;
}

const QString& KCardImplementation::getSubSubType() const {
	return _subSubType;
}

int KCardImplementation::lastError() const {
	return _errno;
}

void KCardImplementation::clearError() {
	_errno = 0;
}

