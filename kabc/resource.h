/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_RESOURCE_H
#define KABC_RESOURCE_H

#include <kresources/resource.h>

#include "addressbook.h"
#include "plugin.h"

namespace KABC {

/**
 * @short Helper class for handling coordinated save of address books.
 * 
 * This class is used as helper class for saving address book.
 * @see requestSaveTicket(), save().
 */
class Ticket
{
    friend class Resource;
  public:
    Resource *resource() { return mResource; }

  private:    
    Ticket( Resource *resource ) : mResource( resource ) {}
    
    Resource *mResource;
};

/**
 * @internal
 */
class Resource : public KRES::Resource
{
public:
  /**
   * Constructor
   */
  Resource( const KConfig *config );

  /**
   * Destructor.
   */
  virtual ~Resource();

  /**
   * Sets the address book of the resource.
   */
  void setAddressBook( AddressBook* );

  /**
   * Returns a pointer to the addressbook.
   */
  AddressBook *addressBook();

  /**
   * Writes the resource specific config to file.
   */
  virtual void writeConfig( KConfig *config );

  /**
   * Open the resource and returns if it was successfully
   */
  virtual bool doOpen();

  /**
   * Close the resource and returns if it was successfully
   */
  virtual void doClose();
  
  /**
   * Request a ticket, you have to pass through save() to
   * allow locking.
   */
  virtual Ticket *requestSaveTicket();
  
  /**
   * Load all addressees to the addressbook
   */
  virtual bool load();

  /**
   * Save all addressees to the addressbook.
   *
   * @param ticket The ticket you get by requestSaveTicket()
   */
  virtual bool save( Ticket *ticket );

  /**
   * Removes a addressee from resource. This method is mainly
   * used by record-based resources like LDAP or SQL.
   */
  virtual void removeAddressee( const Addressee& addr );

  /**
   * This method is called by an error handler if the application
   * crashed
   */
  virtual void cleanUp();

protected:
  Ticket *createTicket( Resource * );

private:
  AddressBook *mAddressBook;
};

}
#endif
