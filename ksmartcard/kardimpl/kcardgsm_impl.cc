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
  KCardGsmImplementation *create_gsm() {
    return new KCardGsmImplementation ();
  }
  
}

KCardGsmImplementation::KCardGsmImplementation()
  :KCardImplementation (KCARD_TYPE_PROCESSOR,KCARD_TYPE_GSM,KCARD_TYPE_GSM){

	_errorMessage=QString::null;
	_errno=0;
	_fileHeader=QString::null;
}


KCardGsmImplementation::~KCardGsmImplementation(){
}


int KCardGsmImplementation::selectFile (const QString fileID){
  
  QString result,status;
  int rc=0;
  QString selectGSMfile("A0A4000002");
  QString getResponse  ("A0C00000");


  //This is the standard file selection process for gsm cards...
  selectGSMfile+=fileID;
  _errno = _kcardreader->doCommand(selectGSMfile,result,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      return -1;
    }
  else if (status.left(2)!= "9F") {
    _errorMessage= i18n("Error when selecting GSM file: ")+fileID;
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
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      _fileHeader=QString::null;
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {

    _errorMessage= i18n("Error when retrieving GSM file information: ")+fileID;
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    _fileHeader=QString::null;
    return -1;

  }
  
  return 0;
}


QString  KCardGsmImplementation::getResponse ()const {

  return _fileHeader;

}

int KCardGsmImplementation::readBinary (QString & fileContent){
  
  QString readBinaryCommand="A0B0";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  //_fileHeader.mid(6,2) is the file length coded in the file header
  //see 11.11 paragraph 9.2.1
  readBinaryCommand += QString("0000") + _fileHeader.mid(6,2);
  
  _errno = _kcardreader->doCommand(readBinaryCommand,fileContent,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when reading GSM card transparent file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;
}

int KCardGsmImplementation::readBinary   (QString & fileContent, unsigned short offset, unsigned char length){

  QString readBinaryCommand="A0B0";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  readBinaryCommand += QString::number(offset,16) +QString::number(length,16);
  
  _errno = _kcardreader->doCommand(readBinaryCommand,fileContent,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when reading GSM card transparent file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;
}

  

int KCardGsmImplementation:: updateBinary (const QString & newContent, 
					   unsigned short offset, 
					   unsigned char length=0){

  
  QString updateBinaryCommand="A0DC";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  updateBinaryCommand += QString::number(offset,16) +QString::number(length,16) + newContent.left(length*2);
  
  _errno = _kcardreader->doCommand(updateBinaryCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when updating GSM card transparent file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;

}

int KCardGsmImplementation::updateBinary (const QString & newContent){

  
  QString updateBinaryCommand="A0DC0000";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  bool ok;
  unsigned short length = _fileHeader.mid(6,4).toUShort(&ok,16);
  updateBinaryCommand +=  _fileHeader.mid(8,2)+newContent.left(length);
  
  
  _errno = _kcardreader->doCommand(updateBinaryCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when updating GSM card transparent file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;

}

int KCardGsmImplementation::readRecord ( QString & recordContent, unsigned char recordNumber, accessMode mode=absolute){



  QString readRecordCommand="A0B2";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  switch (mode){

  case absolute:
    readRecordCommand += QString::number(recordNumber,16) +"04";
    break;
  case previous:
    readRecordCommand += "0003";
    break;
  case next:
    readRecordCommand += "0002";
    break;
  }
  //Record length is in byte 15 of select response
  readRecordCommand += _fileHeader.mid(30,2);
  _errno = _kcardreader->doCommand(readRecordCommand,recordContent,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when reading GSM card record file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}


int KCardGsmImplementation::updateRecord (const QString & recordContent, unsigned char recordNumber, accessMode mode=absolute){

  QString updateRecordCommand="A0DC";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  switch (mode){

  case absolute:
    updateRecordCommand += QString::number(recordNumber,16) +"04";
    break;
  case previous:
    updateRecordCommand += "0003";
    break;
  case next:
    updateRecordCommand += "0002";
    break;
  }
  //Record length is in byte 15 of select response
    bool ok;
  unsigned short recLength= _fileHeader.mid(30,2).toUShort(&ok, 16);
  updateRecordCommand += _fileHeader.mid(30,2) + recordContent.left(recLength*2);
  _errno = _kcardreader->doCommand(updateRecordCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when updating GSM card record file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;

}

int KCardGsmImplementation::seek (const QString & pattern, const seekMode mode, char & recordNumber){

  QString seekCommand="A0A200";
  QString status=QString::null;
  QString result=QString::null;
  recordNumber=-1;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }

		 
		 
		
  switch (mode){
    
  case beginForward:
    seekCommand+= "10";
    break;

  case endBackward:
    seekCommand+= "11";
    break;

  case nextLocationForward:
    seekCommand+= "12";
    break;

  case  prevLocationBackward:
    seekCommand+= "13";
    break;
  }
  
  
  seekCommand += QString::number(pattern.length()/2,16) + pattern;
  _errno = _kcardreader->doCommand(seekCommand,result,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when seeking for pattern in GSM record file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  bool ok;
  recordNumber=result.toUShort(&ok,16);
  return 0;



}

int KCardGsmImplementation::verifyCHV1  (const QString & CHV1){


  QString verifyCommand="A0200001";
  QString status=QString::null;
  
  
  
  verifyCommand += CHV1.left(16);
  
  _errno = _kcardreader->doCommand(verifyCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when verifying CHV1 ");
    _errorMessage+=getStatusString (status);
    return -1;

  }
  return 0;



}

int KCardGsmImplementation::verifyCHV2  (const QString & CHV2){
  QString verifyCommand="A0200002";
  QString status=QString::null;
  
  
  
  verifyCommand += CHV2.left(16);
  
  _errno = _kcardreader->doCommand(verifyCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when verifying CHV2 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}

int KCardGsmImplementation::unblockCHV1 (const QString & unblockCHV1, const QString & newCHV1){

  QString unblockCommand="A02C0000";
  QString status=QString::null;
  
  
  
  unblockCommand += unblockCHV1.left(16)+newCHV1.left(16);
  
  _errno = _kcardreader->doCommand(unblockCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when unblocking CHV1 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}


int KCardGsmImplementation::unblockCHV2 (const QString & unblockCHV2, const QString & newCHV2){

  QString unblockCommand="A02C0002";
  QString status=QString::null;
  
  
  
  unblockCommand += unblockCHV2.left(16)+newCHV2.left(16);
  
  _errno = _kcardreader->doCommand(unblockCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when unblocking CHV2 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}


int KCardGsmImplementation::disableCHV1  (const QString & CHV1){



  QString disableCommand="A026000108";
  QString status=QString::null;
  
  
  
  disableCommand += CHV1.left(16);
  
  _errno = _kcardreader->doCommand(disableCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when disabling CHV1 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;




}

int KCardGsmImplementation::enableCHV1  (const QString & CHV1){


  QString enableCommand="A028000108";
  QString status=QString::null;
  
  
  
  enableCommand += CHV1.left(16);
  
  _errno = _kcardreader->doCommand(enableCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when enabling CHV1 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;





}


int KCardGsmImplementation::changeCHV1 (const QString & currentCHV1, const QString & newCHV1){


  QString changeCommand="A024000110";
  QString status=QString::null;
  
  
  
  changeCommand += currentCHV1.left(16)+newCHV1.left(16);
  
  _errno = _kcardreader->doCommand(changeCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when changing CHV1 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;





}


int KCardGsmImplementation::changeCHV2 (const QString & currentCHV2, const QString & newCHV2){

  QString changeCommand="A024000210";
  QString status=QString::null;
  
  
  
  changeCommand += currentCHV2.left(16)+newCHV2.left(16);
  
  _errno = _kcardreader->doCommand(changeCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when changing CHV2 ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}

int KCardGsmImplementation::invalidate(){


  QString invalidateCommand="A004000000";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  _errno = _kcardreader->doCommand(invalidateCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when invalidating GSM file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;


}




int KCardGsmImplementation::rehabilitate(){

 QString rehabCommand="A044000000";
  QString status=QString::null;
  
  if (_fileHeader==QString::null) {
    _errorMessage= i18n("No GSM file selected ");
    return -1;

  }
  
  _errno = _kcardreader->doCommand(rehabCommand,status);
  
  if ( _errno )
    
    {
      _errorMessage= i18n("Error sending APDU command: ")+KPCSC::translateError(_errno);
      
      return -1;
    }

  else if (status.left(2)!= "90" && status.left(2)!= "91") {
    
    _errorMessage= i18n("Error when rehabilitating GSM file ");
    _errorMessage+=KCardGsmImplementation::getStatusString (status);
    return -1;

  }
  return 0;



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
