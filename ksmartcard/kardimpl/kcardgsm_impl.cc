/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr >
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

#include "kcardgsm_impl.h"
#include "kcardreader.h"
#include <klocale.h>

extern "C" {
  KCardGsmImplementation *create_gsm(KCardReader * reader) {
    return new KCardGsmImplementation (reader);
  }
  
}

KCardGsmImplementation::KCardGsmImplementation(KCardReader * reader)
  :KCardImplementation (reader){

  	_type = KCARD_TYPE_PROCESSOR;
	_subType = "GSM";
	_subSubType = "GSM";
	_errorMessage=QString::null;
	_errno=0;
	_fileHeader=QString::null;
}


KCardGsmImplementation::~KCardGsmImplementation(){
}


int KCardGsmImplementation::selectFile (const QString fileID){
  
  QString result,status;
  int rc=0;
  QString selectGSMfile("A0A40002");
  QString getResponse  ("A0C00000");


  //This is the standard file selection process for gsm cards...
  selectGSMfile+=fileID;
  _errno = _kcardreader->doCommand(selectGSMfile,result,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending command: ")+KPCSC::translateError(_errno);
      return -1;
    }
  else if (status.left(2)!= "9F") {
    _errorMessage= i18n("Error when selecting file: ")+fileID;
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;
  }




  //And then i send a 'getstatus' command to retrieve the file information:
  //file type, length, access conditions...

  //The LSB byte in status contains the data lenght we have to retrieve
  getResponse+=status.right(2);
  _errno = _kcardreader->doCommand(getResponse,_fileHeader,status);

  if ( _errno )

    {
      _errorMessage= i18n("Error sending command: ")+KPCSC::translateError(_errno);
      _fileHeader=QString::null;
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {

    _errorMessage= i18n("Error when retrieving file information: ")+fileID;
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    _fileHeader=QString::null;
    return -1;

  }
  
  return 0;
}


QString  KCardGsmImplementation::getResponse ()const {

  return _fileHeader;

}

int KCardGsmImplementation::readTransparentFile (QString & fileContent){
  
  QString readBinaryCommand="A0B0";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No file selected ");
    return -1;

  }
  //_fileHeader.mid(6,2) is the file length coded in the file header
  //see 11.11 paragraph 9.2.1
  readBinaryCommand += QString("0000") + _fileHeader.mid(6,2);
  
  _errno = _kcardreader->doCommand(readBinaryCommand,fileContent,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when reading transparent file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;
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


QString KCardGsmImplementation::getStatusString (const QString & status){

QString resultString;
bool * result= new bool;
ushort shortStatus = status.toUShort(result,16);
if (*result!=TRUE)    { delete result;return i18n("Not a valid status");}
delete result;
switch (shortStatus) {


//Responses to commands which are correctly executed

case (0x9000):
 resultString = ("Normal ending of the command");
break;



//Responses to commands which are postponed


case (0x9300):
resultString =" SIM Application Toolkit is busy. Command cannot be executed at present, further normal commands are allowed";

//	Memory management


case (0x9240):

resultString= "Memory problem";
break;

//Referencing management

case (0x9400):
resultString ="No EF selected";
break;

case (0x9402):
resultString="Out of range (invalid address)";
break;

case (0x9404):

resultString ="File ID not found or Pattern not found";

break;

case (0x9408):
resultString="File is inconsistent with the command";
break;

//	Security management


case (0x9802):
resultString ="No CHV initialized";
break;

case (0x9804):
resultString="	access condition not fulfilled \n unsuccessful CHV verification, at least one attempt left \nunsuccessful UNBLOCK CHV verification, at least one attempt left\n	authentication failed";
break;

case (0x9808):

resultString ="In contradiction with CHV status";
break;

case (0x9810):

resultString ="In contradiction with invalidation status";
break;

case (0x9840):
resultString ="-	unsuccessful CHV verification, no attempt left\n	-	unsuccessful UNBLOCK CHV verification, no attempt left\n-	CHV blocked\n	-	UNBLOCK CHV blocked\n";
break;

case (0x9850):
resultString="Increase cannot be performed, Max value reached";
break;


}


ushort auxshortStatus=shortStatus & 0xFF00;
//	Memory management
switch (auxshortStatus) {

case (0x9200):
	
	resultString =QString("Command successful but after using an internal update retry routine %1 times").arg(shortStatus & 0X000F);
break;

//Responses to commands which are correctly executed

case ( 0x9100):
resultString=QString ("Normal ending of the command, with extra information from the proactive SIM containing a command for the ME.\n Length %1 of the response data").arg(shortStatus &0x00FF);
break;

case (0x9E00):
resultString =QString("Length %1 of the response data given in case of a SIM data download error").arg(shortStatus &0x00FF);
break;

case (0x9F00):

resultString=QString("Length %1 of the response data").arg(shortStatus &0x00FF);
break;

//Application independent errors


//Error description
case (0x6700):

 if (shortStatus &0x00FF !=0x0000)
resultString=QString("Incorrect parameter P3, right value should be %1").arg(shortStatus &0x00FF);
else
resultString=QString("Incorrect parameter P3, no aditional information");
break;

case (0x6B00):

resultString =QString ("Incorrect parameter P1 or P2");
break;

case (0x6D00):

resultString = QString( "Unknown instruction code given in the command");
break;

case (0x6E00):

resultString =QString("Wrong instruction class given in the command");
break;

case (0x6F00):
resultString =QString ("Technical problem with no diagnostic given");
break;

//NOTE 1:	# These values of 'XX' are specified by ISO/IEC; at present the default value 'XX'='00' is the only one defined.
//NOTE 2:	## When the error in P1 or P2 is caused by the addressed record being out of range, then the return code '94 02' shall be used.
//
//NOTE:	'XX' gives the correct length or states that no additional information is given ('XX' = '00').


}
return QString(resultString);
}
