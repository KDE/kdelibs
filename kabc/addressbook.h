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
#ifndef KABC_ADDRESSBOOK_H
#define KABC_ADDRESSBOOK_H
// $Id$

#include <qobject.h>
#include <qptrlist.h>

#include "addressee.h"
#include "field.h"

namespace KABC {

class Resource;
class Ticket;

/**
  @short Address Book
  
  This class provides access to a collection of address book entries.
*/
class AddressBook : public QObject
{
    Q_OBJECT

    friend QDataStream &operator<<( QDataStream &, const AddressBook & );
    friend QDataStream &operator>>( QDataStream &, AddressBook & );
    friend class Resource;

  public:
    /**
      @short Address Book Iterator
      
      This class provides an iterator for address book entries.
    */
    class Iterator
    {
      public:
        Iterator();
        Iterator( const Iterator & );
      	~Iterator();

      	Iterator &operator=( const Iterator & );
        const Addressee & operator*() const;
        Addressee &operator*();
        Iterator &operator++();
        Iterator &operator++(int);
        Iterator &operator--();
        Iterator &operator--(int);
        bool operator==( const Iterator &it );
        bool operator!=( const Iterator &it );

      	struct IteratorData;
	IteratorData *d;
    };

    /**
      @short Address Book Const Iterator
      
      This class provides a const iterator for address book entries.
    */
    class ConstIterator
    {
      public:
        ConstIterator();
        ConstIterator( const ConstIterator & );
      	~ConstIterator();
      
      	ConstIterator &operator=( const ConstIterator & );
        const Addressee & operator*() const;
        ConstIterator &operator++();
        ConstIterator &operator++(int);
        ConstIterator &operator--();
        ConstIterator &operator--(int);
        bool operator==( const ConstIterator &it );
        bool operator!=( const ConstIterator &it );

      	struct ConstIteratorData;
	ConstIteratorData *d;
    };
    
    /**
      Construct address book object.
      
      @param format File format class.
    */
    AddressBook();
    virtual ~AddressBook();

    /**
      Request a ticket for saving the addressbook. Calling this function locks
      the addressbook for all other processes. If the address book is already
      locked the function returns 0. You need the returned @ref Ticket object
      for calling the @ref save() function.
      
      @see save()
    */
    Ticket *requestSaveTicket( Resource *resource=0 );
    
    /**
      Load address book from file.
    */
    bool load();
    /**
      Save address book. The address book is saved to the file, the Ticket
      object has been requested for by @ref requestSaveTicket().
    
      @param ticket a ticket object returned by @ref requestSaveTicket()
    */
    bool save( Ticket *ticket );

    /**
      Return iterator for first entry of address book.
    */
    Iterator begin();
    /**
      Return const iterator for first entry of address book.
    */
    ConstIterator begin() const;
    /**
      Return iterator for first entry of address book.
    */
    Iterator end();
    /**
      Return const iterator for first entry of address book.
    */
    ConstIterator end() const;

    /**
      Remove all entries from address book.
    */
    void clear();
    
    /**
      Insert an Addressee object into address book. If an object with the same
      unique id already exists in the address book it it replaced by the new
      one. If not the new object is appended to the address book.
    */
    void insertAddressee( const Addressee & );
    /**
      Remove entry from the address book.
    */
    void removeAddressee( const Addressee & );
    /**
      Remove entry from the address book.
    */
    void removeAddressee( const Iterator & );

    /**
      Find the specified entry in address book. Returns end(), if the entry
      couldn't be found.
    */
    Iterator find( const Addressee & );

    /**
      Find the entry specified by an unique id. Returns an empty Addressee
      object, if the address book does not contain an entry with this id.
    */
    Addressee findByUid( const QString & );

    /**
      Find all entries with the specified name in the address book. Returns
      an empty list, if no entries couldn't be found.
    */
    Addressee::List findByName( const QString & );
    /**
      Find all entries with the specified email address  in the address book.
      Returns an empty list, if no entries couldn't be found.
    */
    Addressee::List findByEmail( const QString & );
    /**
      Find all entries wich have the specified category in the address book.
      Returns an empty list, if no entries couldn't be found.
    */
    Addressee::List findByCategory( const QString & );

    /**
      Return a string identifying this addressbook.
    */
    virtual QString identifier();

    /**
      Debug output.
    */
    void dump() const;

    void emitAddressBookLocked() { emit addressBookLocked( this ); }
    void emitAddressBookUnlocked() { emit addressBookUnlocked( this ); }
    void emitAddressBookChanged() { emit addressBookChanged( this ); }

    /**
      Return list of all Fields known to the address book which are associated
      with the given field category.
    */
    Field::List fields( int category = Field::All );

    /**
      Add custom field to address book.
      
      @param label    User visible label of the field.
      @param category Ored list of field categories.
      @param key      Identifier used as key for reading and writing the field.
      @param app      String used as application key for reading and writing
                      the field.
    */
    bool addCustomField( const QString &label, int category = Field::All,
                         const QString &key = QString::null,
                         const QString &app = QString::null );

  signals:
    /**
      Emitted, when the address book has changed on disk.
    */
    void addressBookChanged( AddressBook * );
    /**
      Emitted, when the address book has been locked for writing.
    */
    void addressBookLocked( AddressBook * );
    /**
      Emitted, when the address book has been unlocked.
    */
    void addressBookUnlocked( AddressBook * );

  private:
    QPtrList<Resource> mResources;

    struct AddressBookData;
    AddressBookData *d;

  protected:
    /**
      Add address book resource.
    */
    bool addResource( Resource * );

    /**
      Remove address book resource.
    */
    bool removeResource( Resource * );
};

QDataStream &operator<<( QDataStream &, const AddressBook & );
QDataStream &operator>>( QDataStream &, AddressBook & );

}

#endif
