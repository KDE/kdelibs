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

  /**
   * @li @p next - next record for reading/writing operations
   * @li @p previous - previous record for reading/writing operations
   * @li @p absolute - record number indicated in the command
   */

  enum accessMode {next, previous, absolute};

    /**
       @li @p  beginForward
       @li @p	 endBackward
       @li @p	 nextLocationForward
       @li @p  prevLocationBackward
   */

  enum seekMode {beginForward,
		 endBackward,
		 nextLocationForward,
		 prevLocationBackward};

  KCardGsmImplementation ();
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
   *  @see readBinary 
   */

  int readBinary   (QString & fileContent);
  
  /**
   *  Read a the content of a GSM card transparent file
   *  No check is done if access conditions are fulfilled
   *
   *  @param fileContent will contain the file content after the reading operation
   *  @param offset contains the offset of the first byte to read
   *
   *  @param lenght contains the length of the data to be read
   *
   *  @returns -1 in case of error and 0 if the file content has been read
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   *  @see readBinary 
   */


  int readBinary   (QString & fileContent, unsigned short offset, unsigned char length);


  /**
   *  Update the content of a transparent file.
   *  No check is done if access conditions are fulfilled.
   *  @param newContent contains the value to be written in the file
   *  @param offset contains the offset of the first byte to write
   *  @param length contains the length of the data to write, this may be different from 'newContent' length value.
   *
   *  @returns -1 in case of error and 0 if the file content has been updated
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int updateBinary (const QString & newContent, unsigned short offset, unsigned char length=0);
  

  /**
   *  Update the content of a transparent file.
   *  No check is done if access conditions are fulfilled.
   *  @param newContent contains the value to be written in the file
   *  This function copies the number of bytes in 'newContent' corresponding to the  the transparent file length (from left to right)
   *
   *  @returns -1 in case of error and 0 if the file content has been updated
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */
  
  int updateBinary (const QString & newContent);
  




  /**
   *  Read one record from a linear GSM file.
   *  No check is done if access conditions are fulfilled 
   *  @param recordContent will contain the read value
   *  @param recordNumber will contain the record number to read if accessMode is set to 'absolute'
   *  
   *  @returns -1 in case of error and 0 if the file content has been read
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   */

  int readRecord ( QString & recordContent, unsigned char recordNumber, accessMode mode=absolute);


  /**
   *  Update one record from a linear GSM file.
   *  No check is done if access conditions are fulfilled 
   *  @param recordContent will contain the value to update
   *  @param recordNumber will contain the record number to write if accessMode is set to 'absolute'
   *  @param mode contains the access mode use to update the record
   *  This function copies the number of bytes from 'recordContent' QString corresponding to the record length (from left to right)
   *  @returns -1 in case of error and 0 if the file content has been updated
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   **/

  int updateRecord (const QString & recordContent, unsigned char recordNumber, accessMode mode=absolute);
  
  

  /**
   * This method searches trough the current linear fixed file to find a record starting
   * with the given patter
   * @param pattern is the patter to look for, max length is 16 bytes (see ETSI 11.11)
   * @param mode is the mode used for the search
   * @param recordNumber will contain the record where the patter is found or -1 if no pattern was found
   * Type 2 search is used by default (see ETSI 11.11); so the pointer in the record file will be set to the record were the patter is found.
   *  @returns -1 in case of error and 0 if the search was successfull
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   */

  int seek (const QString & pattern, const seekMode mode, char & recordNumber);

  
  /**
   * This methods verify the CHV1 of the GSM card. This secret code value has fixed value of 8 bytes, so only the first 8 bytes of CHV1 will be presented to the card.
   * @param CHV1 contains the value to be presented to the card
   *
   *  @returns -1 in case of error and 0 if the code was successfully verified
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */
  int verifyCHV1  (const QString & CHV1);


  /**
   * This methods verify the CHV1 of the GSM card. This secret code value has fixed value of 8 bytes, so only the first 8 bytes of CHV1 will be presented to the card.
   * @param CHV1 contains the value to be presented to the card
   *
   *  @returns -1 in case of error and 0 if the code was successfully verified
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int verifyCHV2  (const QString & CHV2);

  /**
   * This method unblocks the CHV1 code of the GSM card.
   * @param unblockCHV1 contains the unblock CHV1 code
   * @param newCHV1 contains the new value the CHV1 will get after sucessfull unblocking
   *
   *Those secret codes length is fixed to of 8 bytes, so only the first 8 bytes of both QString parameters  will be presented to the card.
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int unblockCHV1 (const QString & unblockCHV1, const QString & newCHV1);

  /**
   * This method unblocks the CHV2 code of the GSM card.
   * @param unblockCHV2 contains the unblock CHV2 code
   * @param newCHV2 contains the new value the CHV2 will get after sucessfull unblocking
   *
   *Those secret codes length is fixed to of 8 bytes, so only the first 8 bytes of both QString parameters  will be presented to the card.
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int unblockCHV2 (const QString & unblockCHV2, const QString & newCHV2);


  /**
   * This methods disable the CHV1 of the GSM card. This secret code value has fixed value of 8 bytes, so only the first 8 bytes of CHV1 will be presented to the card.
   * @param CHV1 contains the value to be presented to the card
   *
   *  @returns -1 in case of error and 0 if the code was successfully disabled
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */
  int disableCHV1  (const QString & CHV1);

  /**
   * This methods enable the CHV1 of the GSM card. This secret code value has fixed value of 8 bytes, so only the first 8 bytes of CHV1 will be presented to the card.
   * @param CHV1 contains the value to be presented to the card
   *
   *  @returns -1 in case of error and 0 if the code was successfully enabled
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */
  int enableCHV1  (const QString & CHV1);  


  /**
   * This method changes the CHV1 code of the GSM card.
   * @param currentCHV1 contains the current CHV1 code
   * @param newCHV1 contains the new value the CHV1 will get
   *
   *Those secret codes length is fixed to of 8 bytes, so only the first 8 bytes of both QString parameters  will be presented to the card.
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int changeCHV1 (const QString & currentCHV1, const QString & newCHV1);

  /**
   * This method changes the CHV2 code of the GSM card.
   * @param currentCHV2 contains the current CHV2 code
   * @param newCHV2 contains the new value the CHV2 will get
   *
   *Those secret codes length is fixed to of 8 bytes, so only the first 8 bytes of both QString parameters  will be presented to the card.
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   *
   */

  int changeCHV2 (const QString & currentCHV2, const QString & newCHV2);

  /**
   *This method invalidates the currently selected file. No check is done if access conditions are fulfilled
   *
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   */

  int invalidate();
  
  /**
   *This method rehabilitates the currently selected file. No check is done if access conditions are fulfilled
   *
   *  @returns -1 in case of error and 0 if the code was successfully unblocked
   *  In error case use @ref lastErrorMessage to retrieve a human readable
   *  error explanation.
   */

  int rehabilitate();



  /**
   * Provides a text describing the last error appeared while accesing the card. This error may be due to problems communicating with the card or more high level problems as trying to update a record on a transparent files.
   This second kind of errors can be differenciated looking @ref lastStatus
   */

  QString lastErrorMessage () const {return _errorMessage;};

  /**
   * Provides the last status received from the card, it will be mainly used to trace logical errors or transmissions problems with the reader. It is a short value because status words are usually described in hexa as: '90 00', '6B00'
   */
  unsigned short lastStatus() const {return _status;};

 private:

  class  KCardGSMImplementationPrivate;
  KCardGSMImplementationPrivate *d;
  QString _errorMessage;
  QString _fileHeader;
  unsigned short _status;
  
  static QString getStatusString (const QString & status);
};

#endif

