/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
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


#ifndef _KCARDIMPLEMENTATION_H
#define _KCARDIMPLEMENTATION_H

#include <qstring.h>
#include <kpcsc.h>


#define KCARD_TYPE_UNKNOWN   "Unknown"
#define KCARD_TYPE_PROCESSOR "Processor"
#define KCARD_TYPE_GSM       "GSM"
#define KCARD_TYPE_MEMORY    "Memory"
#define KCARD_TYPE_JAVA      "Java"


class KCardReader;

class KCardImplementation {


 public:

  /**
   *Initialises internally the class to work with the card in reader
   *selectedReader. This method MUST be invoked before performing any other
   *operation; as a security rule use the KCardImplementation objects returned
   *by KCardFactory, which will be always properly initialised.
   */

  int init(const QString & selectedReader);

  /**
   *Implementation of the select file command, you should overload this method
   *so that the proper APDU are sent to the card to select the file
   *Default implementation returns -1
   *@param fileId contains the identifier of the file to be selected
   *@return 0 in case of success and -1 in case of error
   *Use @ref lastErrorMessage to retrieve a readable explanation
   */

  virtual int selectFile(const QString & fileId);
  /**
   *This method behaves like the previous one... it will disappear one day
   */

  
  virtual int selectDirectory(const QString & fileId);


  /**
   *Retrieve card serial number, default implementation returns an empty 
   *KCardCommand
   *@returns card serial number or an empty KCardCommand in error case
   */
  
  virtual KCardCommand getCardSerialNumber();

  /**
   *Return the card type managed by this KCardImplementation
   */

  virtual const QString& getType() const;

  /**
   *Return the card subType managed by this KCardImplementation
   */
  virtual const QString& getSubType() const;

  /**
   *Return the card subSubType managed by this KCardImplementation
   */
  virtual const QString& getSubSubType() const;

  /** Attempt to match the ATR.  This uses a heuristic to determine
   * if the given ATR is for a card with this type/subtype/subsubtype.
   * It should return 0 or -1 for a failure, and a number up to 100 for
   * the percentage of probability of successful match.
   *@param atr indicates the ATR to match
   */
  virtual int matchATR(KCardCommand atr);


  /**
   *Internally used to keep track of last error code
   */
  virtual int lastError() const;
  /**
   *Reset the error value member to cero
   */
  virtual void clearError();

  /**
   *You should implement this method to return to the user readable error
   *messages in error case
   *Default implementation returns QString::null
   */
  virtual QString lastErrorMessage () const {return QString::null;};
 private:

  class KCardImplementationPrivate;
  KCardImplementationPrivate *d;

  /**
   *KPCSC object used internally to send APDU to the card through 
   *@ref _kcardreader protected member
   */
  KPCSC * pcscInt;

 protected:


  /**
   *Default constructor... creates a KCardImplementation that implements
   *no type, subType, subSubType
   */
  KCardImplementation (const QString & type=KCARD_TYPE_UNKNOWN, 
		       const QString & subType=KCARD_TYPE_UNKNOWN,
		       const QString & subSubType=KCARD_TYPE_UNKNOWN);

  virtual ~KCardImplementation();

  
  KCardReader * _kcardreader;  
  
  QString _type;
  QString _subType;
  QString _subSubType;
  int _errno;


};

#endif
