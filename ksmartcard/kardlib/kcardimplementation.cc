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


}

KCardImplementation::~KCardImplementation( ){

}


int KCardImplementation::selectFile (const QString & fileID){

  //For the moment i send a select command with the GSM format, anyway i will
  //overload the class in the KGSMCardImplementation
  QString result;
  QString selectGSMfile("A0A40002");
  selectGSMfile+=fileID;
  int rc = _kcardreader->doCommand(selectGSMfile,result);
  if (rc) return rc;

  
  
}
