/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
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


#ifndef __KCARD_GSM_TYPE_H
#define __KCARD_GSM_TYPE_H

#include <qstringlist.h>

#include "kcardimplementation.h"


/**
 *This class implements the more common operations to be done on a GSM card, 
 *like reading, updating file contents as well as secret code verifying
 *@short class for accessing GSM cards content
 *@author Fernando Llobregat <fernando.llobregat@free.fr>  
 *
 */

class KCardGsmImplementation:public KCardImplementation{


 public:

  KCardGsmImplementation (KCardReader *);
  ~KCardGsmImplementation();

  /**
   *   Selects a file in the smart card
   *   
   *   @param fileID For GSM cards this fileID is a two byte identifier
   *   @returns Returns -1 in case of error, and 0 if the file has been
   *   succesfully selected.
   *   In error case use @ref lastErrorMessage to retrieve a human readable
   *   error explanation.
   *   
   */
  int selectFile(const QString fileID);



  /**
   *   Returns the card response after successfully selecting a file.
   *   This corresponds to the GET RESPONSE command in ETSI GSM 11.11
   *   
   *   @returns a string containing the the last selected file information header or 
   *   QString::null if there is no a current selected file.
   *   In error case use @ref lastErrorMessage to retrieve a human readable
   *   error explanation.
   */
  
  QString   getResponse ()const ;

  /**
   *  Read a the content of a GSM card transparent file
   *  No check is done if access conditions are fulfilled
   *
   *  @param fileContent will contain the file content after the reading operation
   *
   *  @returns -1 in case of error and 0 if the file content has been read
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   *  @see readTransparentFile (QString &, unsigned char offset, unsigned char length);
   *
   */

  int readTransparentFile   (QString & fileContent);
  int readTransparentFile   (QString &, unsigned char offset, unsigned char length);
  int updateTransparentFile (QString &, unsigned char offset, unsigned char length);

  int readLinearFixedFile    (QStringList&);
  int readLinearFixedFileRecord    (QStringList&, unsigned char record);

  int readCyclicFile (QStringList &);
  
  int verifyCHV1  (const QString &);
  int verifyCHV2  (const QString &);
  int verifyUnblockCHV1 (const QString &);
  int verifyUnblockCHV2 (const QString &);

  int runGsmAlgorithm(const QString &, QString &);


  QString lastErrorMessage () const;

 private:

  class  KCardGSMImplementationPrivate;
  KCardGSMImplementationPrivate *d;
  QString _errorMessage;
  QString _fileHeader;
  
  static QString getStatusString (const QString & status);
};

#endif

