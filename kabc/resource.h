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
    ~Ticket();

    Resource *resource();

  private:    
    Ticket( Resource *resource );
   
    Resource *mResource;
};

/**
 * @internal
 */
class Resource : public KRES::Resource
{
  Q_OBJECT

  public:

    /**
      @short Resource Iterator
      
      This class provides an iterator for resource entries.
    */
    class Iterator
    {
      public:
        Iterator();
        Iterator( const Iterator & );
        virtual ~Iterator();

        virtual Iterator &operator=( const Iterator & );
        virtual const Addressee &operator*() const;
        virtual Addressee &operator*();
        virtual Iterator &operator++();
        virtual Iterator &operator++( int );
        virtual Iterator &operator--();
        virtual Iterator &operator--( int );
        virtual bool operator==( const Iterator &it );
        virtual bool operator!=( const Iterator &it );

        struct IteratorData;
        IteratorData *d;
    };

    /**
      @short Resource Const Iterator
      
      This class provides a const iterator for resource entries.
    */
    class ConstIterator
    {
      public:
        ConstIterator();
        ConstIterator( const ConstIterator & );
        ConstIterator( const Iterator & );
        virtual ~ConstIterator();

        virtual ConstIterator &operator=( const ConstIterator & );
        virtual const Addressee &operator*() const ;
        virtual ConstIterator &operator++();
        virtual ConstIterator &operator++( int );
        virtual ConstIterator &operator--();
        virtual ConstIterator &operator--( int );
        virtual bool operator==( const ConstIterator &it );
        virtual bool operator!=( const ConstIterator &it );

        struct ConstIteratorData;
        ConstIteratorData *d;
    };

    /**
      Constructor
     */
    Resource( const KConfig *config );

    /**
      Destructor.
     */
    virtual ~Resource();

    virtual Iterator begin();
    virtual ConstIterator begin() const;

    virtual Iterator end();
    virtual ConstIterator end() const;

    /**
      Sets the address book of the resource.
     */
    void setAddressBook( AddressBook* );

    /**
      Returns a pointer to the addressbook.
     */
    AddressBook *addressBook();

    /**
      Writes the resource specific config to file.
     */
    virtual void writeConfig( KConfig *config );

    /**
      Open the resource and returns if it was successfully
     */
    virtual bool doOpen() = 0;

    /**
      Close the resource and returns if it was successfully
     */
    virtual void doClose() = 0;
  
    /**
      Request a ticket, you have to pass through save() to
      allow locking.
    */
    virtual Ticket *requestSaveTicket() = 0;
  
    /**
      Releases the ticket previousely requested with requestSaveTicket().
      You have to call this function if there is no way to call save()
      with this ticket. The resource has to remove any locks which are
      assocciated with this ticket here...
     */
    virtual void releaseSaveTicket( Ticket* ) = 0;

    /**
      Loads all addressees.
     */
    virtual bool load() = 0;

    /**
      Loads asyncronous all addressees. It will return immediatley and
      emit the loadingFinished() signal when finished loading.
     */
    virtual bool asyncLoad() = 0;

    /**
      Insert a addressee into the resource.
     */
    virtual void insertAddressee( const Addressee& );

    /**
      Removes a addressee from resource.
     */
    virtual void removeAddressee( const Addressee& addr );

    /**
      Save all addressees to the addressbook.

      @param ticket The ticket you get by requestSaveTicket(). It will
                    be released automatically.
     */
    virtual bool save( Ticket *ticket ) = 0;

    virtual bool asyncSave( Ticket *ticket ) = 0;

    /**
      This method is called by an error handler if the application
      crashed
     */
    virtual void cleanUp();

    /**
      Removes all addressees from the resource.
     */
    virtual void clear();

  signals:
    void loadingFinished( Resource* );
    void loadingError( Resource*, const QString& );
    void savingFinished( Resource* );
    void savingError( Resource*, const QString& );

  protected:
    Ticket *createTicket( Resource * );
    Addressee::List mAddressees;

  private:
    AddressBook *mAddressBook;

    class ResourcePrivate;
    ResourcePrivate *d;
};

}

#endif
