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

#include "kcardgsmimplementation.h"

KCardGsmImplementation::KCardGsmImplementation(KCardReader * reader)
  :KCardImplementation (reader){


}


KCardGsmImplementation::~KCardGsmImplementation(){
}


int KCardGsmImplementation::readTransparentFile (QString & fileContent){

  return -1;
}

int KCardGsmImplementation::readTransparentFile (QString & fileContent, unsigned char offset, unsigned char length){

  return -1;

}
int KCardGsmImplementation::updateTransparentFile (QString & fileContent, unsigned char offset, unsigned char length){

  return -1;
}

int KCardGsmImplementation::readLinearFixedFile    (QStringList& fileContent){

  return -1;

}
int KCardGsmImplementation::readLinearFixedFileRecord    (QStringList& fileContent,
							  unsigned char record){

  return -1;
}

int KCardGsmImplementation::readCyclicFile (QStringList & fileContent){

  return -1;
}
  
int KCardGsmImplementation::verifyCHV1  (const QString & pin1){

  return -1;
}
int KCardGsmImplementation::verifyCHV2  (const QString & pin2){

  return -1;
}
int KCardGsmImplementation::verifyUnblockCHV1 (const QString & puk1){

  return -1;
}
int KCardGsmImplementation::verifyUnblockCHV2 (const QString & puk2){

  return -1;
}

int KCardGsmImplementation::runGsmAlgorithm(const QString & random, QString & result){

  return -1;
}
