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

#include <sys/types.h>

#include <qobject.h>

#include "addressee.h"

class QTimer;

namespace KABC {

class Format;
class AddressBookPrivate;

/**
  @short Address Book
  
  This class provides access to a collection of address book entries.
*/
class AddressBook : public QObject
{
    Q_OBJECT
  public:
    /**
      @short Address Book Iterator
      
      This class provides an iterator for address book entries.
    */
    class Iterator
    {
      public:
        Iterator() {}
        Iterator( const Addressee::List::Iterator &it ) : mIt( it ) {}
      
        const Addressee & operator*() const { return *mIt; }
        Addressee &operator*() { return *mIt; }
        Iterator &operator++() { mIt++; return *this; }
        Iterator &operator++(int) { mIt++; return *this; }
        Iterator &operator--() { mIt--; return *this; }
        Iterator &operator--(int) { mIt--; return *this; }
        bool operator==( const Iterator &it ) { return ( mIt == it.mIt ); }
        bool operator!=( const Iterator &it ) { return ( mIt != it.mIt ); }

        Addressee::List::Iterator mIt;
    };
    
    /**
      @short Address Book Const Iterator
      
      This class provides a const iterator for address book entries.
    */
    class ConstIterator
    {
      public:
        ConstIterator() {}
        ConstIterator( const Addressee::List::ConstIterator &it ) : mIt( it ) {}
      
        const Addressee & operator*() const { return *mIt; }
        ConstIterator &operator++() { mIt++; return *this; }
        ConstIterator &operator++(int) { mIt++; return *this; }
        ConstIterator &operator--() { mIt--; return *this; }
        ConstIterator &operator--(int) { mIt--; return *this; }
        bool operator==( const ConstIterator &it ) { return ( mIt == it.mIt ); }
        bool operator!=( const ConstIterator &it ) { return ( mIt != it.mIt ); }

        Addressee::List::ConstIterator mIt;
    };
    
    /**
      @short Helper class for handling coordinated save of address books.
      
      This class is used as helper class for saving address book. @See
      requestSaveTicket(), save().
    */
    class Ticket
    {
        friend class AddressBook;
    
        Ticket( const QString &_fileName ) : fileName( _fileName ) {}
        
        QString fileName;
    };

    /**
      Construct address book object.
      
      @param format File format class.
    */
    AddressBook( Format *format=0 );
    virtual ~AddressBook();

    /**
      Request a ticket for saving the addressbook. Calling this function locks
      the addressbook for all other processes. If the address book is already
      locked the function returns 0. You need the returned @ref Ticket object
      for calling the @ref save() function.
      
      @param fileName The file name the addres book is to be saved. If this
                      parameter is omitted or null, the file is used, the
                      address book has been loaded from.
    
      @see save()
    */
    Ticket *requestSaveTicket( const QString &fileName = QString::null );
    
    /**
      Load address book from file.
      
      @param fileName name of file to be loaded.
    */
    bool load( const QString &fileName );
    /**
      Save address book. The address book is saved to the file, the Ticket
      object has been requested for by @ref requestSaveTicket().
    
      @param ticket a ticket object returned by @ref requestSaveTicket()
    */
    bool save( Ticket *ticket );

    /**
      Reload currently loaded addressbook.
    */
    bool reload();
    
    /**
      Return iterator for first entry of address book.
    */
    Iterator begin() { return Iterator( mAddressees.begin() ); }
    /**
      Return const iterator for first entry of address book.
    */
    ConstIterator begin() const { return ConstIterator( mAddressees.begin() ); }
    /**
      Return iterator for first entry of address book.
    */
    Iterator end() { return Iterator( mAddressees.end() ); }
    /**
      Return const iterator for first entry of address book.
    */
    ConstIterator end() const { return ConstIterator( mAddressees.end() ); }
    
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
      Set name of file to be used for saving.
    */
    void setFileName( const QString & );
    /**
      Return name of file used for loading and saving the address book.
    */
    QString fileName() const;

    /**
      Debug output.
    */
    void dump() const;

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

  protected slots:
    void checkFile();

  protected:
    bool lock( const QString &fileName );
    void unlock( const QString &fileName );

  private:
    Addressee::List mAddressees;

    Format *mFormat;

    QString mFileName;
    
    QString mLockUniqueName;
    
    QTimer *mFileCheckTimer;
    time_t mChangeTime;

    AddressBookPrivate *d;
};

}

#endif
