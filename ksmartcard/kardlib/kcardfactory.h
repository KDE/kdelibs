/* This file is part of the KDE project
 *
 * Copyright (C) 2001 Fernando Llobregat <fernando.llobregat@free.fr >
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


#ifndef _KCARDFACTORY_H
#define _KCARDFACTORY_H


#include <kservice.h>
#include <qmap.h>

class KCardReader;
class KCardImplementation;
class QStringList;


// FIXME: Change this to use the strings instead.  Makes it easier to extend
// later.


//typedef  enum {KCardGSMType, KCardProcessorType} KCardType;


 /**
  *This class shall be used to retrieve KCardImplementation objects, just provi  ing the slot in which the card is inserted, it will return an object
  *implementing KCardImplementation. This object can be used to send/receive
  *data from the *card
  *
  *@short generic factory for KCardImplementation objects
  *
  */




class KCardFactory {

 public:

  /**
   *Returns a KCardImplementation object assigned to the card in slot
   *@param slot slot name in which the card is inserted
   *@param cardATR is used to retrieve the library implementing
   *KCardImplementation that will be used.
   *
   *  @returns NULL if no suitable implementation is found or if an error ocurred
   */
  KCardImplementation * getCard ( const QString & slot,
				  const QString & cardATR);

  /**
   *The same method as before, but the ATR for the card in slot is retrieved
   *automatically by this function
   *@param slot slot name in which the card is inserted
   *@returns NULL if no suitable implementation is found or if an error ocurred
   */
  KCardImplementation * getCard ( const QString & slot );

  /**
   *Use this method to retrieve the instance of the KCardFactory class
   */
  static KCardFactory *self();

  /**
   *This method returns a QStringlist with all the implementations currently
   *installed in the local PC
   *@returns all the KCardImplementations libraries currently installed, and the type
   *subType and subSubType supported by each one
   */
  QStringList & getImplementationList() const ;

 protected:

  virtual ~KCardFactory();
 private:

  KCardFactory();

  int loadModules();
  void *loadModule(KService::Ptr svc);

  static KCardFactory *_self;


 typedef QMap< QString, QMap< QString, QMap< QString, KService::Ptr > > > _modulesMap;

 _modulesMap _modules;

 static QStringList _implementationList ;


 class KCardFactoryPrivate;
 KCardFactoryPrivate *d;

};

#endif
