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

KCardImplementation::KCardImplementation( KCardReader * selectedReader){

  _kcardreader = selectedReader;
  _type = KCARD_TYPE_UNKNOWN;
  _subType = KCARD_TYPE_UNKNOWN;
  _subSubType = KCARD_TYPE_UNKNOWN;

}


KCardImplementation::~KCardImplementation() {
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



