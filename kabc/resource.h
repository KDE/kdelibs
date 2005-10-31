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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
class KABC_EXPORT Ticket
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
class KABC_EXPORT Resource : public KRES::Resource
{
  Q_OBJECT

  public:

    /**
      @short Resource Iterator

      This class provides an iterator for resource entries.
      By default it points to a QValueList<Addressee>::Iterator,
      but you can reimplement this class to fit your own needs.
    */
    class KABC_EXPORT Iterator
    {
      public:
        /**
         * Default constructor
         */
        Iterator();
        /**
         * Copy constructor
         */
        Iterator( const Iterator & );
        virtual ~Iterator();

        /**
         * Assignment operator. Assignes the given iterator to
         * @c *this.
         *
         * @return this iterator, @c *this
         */
        virtual Iterator &operator=( const Iterator & );
        /**
         * Constant Dereference operator.
         * @note For invalid iterators, the result is undefined.
         *
         * @return the @c const Addressee object the iterator points to.
         */
        virtual const Addressee &operator*() const;
        /**
         * Dereference operator.
         * @note For invalid iterators, the result is undefined.
         *
         * @return the Addressee object the iterator points to.
         */
        virtual Addressee &operator*();
        /**
         * Preincrement operator. Advances the iterator by one.
         *
         * @return this iterator, @c *this
         */
        virtual Iterator &operator++();
        /**
         * Postincrement operator. Advances the iterator by one.
         * @note This function does not copy the iterator object.
         *
         * @return this iterator, @c *this
         */
        virtual Iterator &operator++( int );
        /**
         * Predecrement operator. Decreases the iterator by one.
         *
         * @return this iterator, @c *this
         */
        virtual Iterator &operator--();
        /**
         * Postdecrement operator. Decreases the iterator by one.
         * @note This function does not copy the iterator object.
         *
         * @return this iterator, @c *this
         */
        virtual Iterator &operator--( int );
        /**
         * Equality operator. Compares this iterator to @p it
         *
         * @param it the iterator to compare this iterator to
         * @return @c true if both iterators are equal,
         *         @c false otherwise
         */
        virtual bool operator==( const Iterator &it ) const;
        /**
         * Inequality operator. Compares this iterator to @p it
         *
         * @param it the iterator to compare this iterator to
         * @return @c true if the iterators are not equal,
         *         @c false otherwise
         */
        virtual bool operator!=( const Iterator &it ) const;

        struct IteratorData;
        IteratorData *d;
    };

    /**
      @short Resource Const Iterator

      This class provides a const iterator for resource entries.
    */
    class KABC_EXPORT ConstIterator
    {
      public:
        /**
         * Default constructor
         */
        ConstIterator();
        /**
         * Copy constructor
         */
        ConstIterator( const ConstIterator & );
        /**
         * Copy constructor. Constructs a ConstIterator from
         * an non-@c const Iterator
         */
        ConstIterator( const Iterator & );
        virtual ~ConstIterator();

        /**
         * Assignment operator. Assignes the given iterator to
         * @c *this.
         *
         * @return this iterator, @c *this
         */
        virtual ConstIterator &operator=( const ConstIterator & );
        /**
         * Constant Dereference operator.
         * @note For invalid iterators, the result is undefined.
         * @note Unlike in Iterator, there is no non-constant
         *       dereference operator.
         *
         * @return the @c const Addressee object the iterator points to.
         */
        virtual const Addressee &operator*() const ;
        /**
         * Preincrement operator. Advances the iterator by one.
         *
         * @return this iterator, @c *this
         */
        virtual ConstIterator &operator++();
        /**
         * Postincrement operator. Advances the iterator by one.
         * @note This function does not copy the iterator object.
         *
         * @return this iterator, @c *this
         */
        virtual ConstIterator &operator++( int );
        /**
         * Predecrement operator. Decreases the iterator by one.
         *
         * @return this iterator, @c *this
         */
        virtual ConstIterator &operator--();
        /**
         * Postdecrement operator. Decreases the iterator by one.
         * @note This function does not copy the iterator object.
         *
         * @return this iterator, @c *this
         */
        virtual ConstIterator &operator--( int );
        /**
         * Equality operator. Compares this iterator to @p it
         *
         * @param it the iterator to compare this iterator to
         * @return @c true if both iterators are equal,
         *         @c false otherwise
         */
        virtual bool operator==( const ConstIterator &it ) const;
        /**
         * Inequality operator. Compares this iterator to @p it
         *
         * @param it the iterator to compare this iterator to
         * @return @c true if the iterators are not equal,
         *         @c false otherwise
         */
        virtual bool operator!=( const ConstIterator &it ) const;

        struct ConstIteratorData;
        ConstIteratorData *d;
    };

    /**
      Constructor.

      @param config The config object where the derived classes can
                    read out their settings.
     */
    Resource( const KConfig *config );

    /**
      Destructor.
     */
    virtual ~Resource();

    /**
      Returns an iterator pointing to the first addressee in the resource.
      This iterator equals end() if the resource is empty.
     */
    virtual ConstIterator begin() const;

    /**
      This is an overloaded member function, provided for convenience. It
      behaves essentially like the above function.
     */
    virtual Iterator begin();

    /**
      Returns an iterator pointing to the last addressee in the resource.
      This iterator equals begin() if the resource is empty.
     */
    virtual ConstIterator end() const;

    /**
      This is an overloaded member function, provided for convenience. It
      behaves essentially like the above function.
     */
    virtual Iterator end();

    /**
      Returns a pointer to the addressbook.
     */
    AddressBook *addressBook();

    /**
      Writes the resource specific config to file.
     */
    virtual void writeConfig( KConfig *config );

    /**
      Request a ticket, you have to pass through save() to
      allow locking. The resource has to create its locks
      in this function.
    */
    virtual Ticket *requestSaveTicket() = 0;

    /**
      Releases the ticket previousely requested with requestSaveTicket().
      The resource has to remove its locks in this function.
	  This function is also responsible for deleting the ticket.
     */
    virtual void releaseSaveTicket( Ticket* ) = 0;

    /**
      Loads all addressees synchronously.

      @returns Whether the loading was successfully.
     */
    virtual bool load() = 0;

    /**
      Loads all addressees asyncronously. You have to make sure that either
      the loadingFinished() or loadingError() signal is emitted from within
      this function.

      The default implementation simply calls the synchronous load.

      @return Whether the synchronous part of loading was successfully.
     */
    virtual bool asyncLoad();

    /**
      Insert an addressee into the resource.
     */
    virtual void insertAddressee( const Addressee& );

    /**
      Removes an addressee from resource.
     */
    virtual void removeAddressee( const Addressee& addr );

    /**
      Saves all addressees synchronously.

      @param ticket You have to release the ticket later with
                    releaseSaveTicket() explicitely.
      @return Whether the saving was successfully.
     */
    virtual bool save( Ticket *ticket ) = 0;

    /**
      Saves all addressees asynchronously. You have to make sure that either
      the savingFinished() or savingError() signal is emitted from within
      this function.

      The default implementation simply calls the synchronous save.

      @param ticket You have to release the ticket later with
                    releaseSaveTicket() explicitely.
      @return Whether the saving was successfully.
     */
    virtual bool asyncSave( Ticket *ticket );

    /**
      Searches an addressee with the specified unique identifier.

      @param uid The unique identifier you are looking for.
      @return The addressee with the specified unique identifier or an
              empty addressee.
     */
    virtual Addressee findByUid( const QString &uid );

    /**
      Searches all addressees which match the specified name.

      @param name The name you are looking for.
      @return A list of all matching addressees.
     */
    virtual Addressee::List findByName( const QString &name );

    /**
      Searches all addressees which match the specified email address.

      @param email The email address you are looking for.
      @return A list of all matching addressees.
     */
    virtual Addressee::List findByEmail( const QString &email );

    /**
      Searches all addressees which belongs to the specified category.

      @param category The category you are looking for.
      @return A list of all matching addressees.
     */
    virtual Addressee::List findByCategory( const QString &category );

    /**
      Removes all addressees from the resource.
     */
    virtual void clear();

    /**
      @internal

      Sets the address book of the resource.
     */
    void setAddressBook( AddressBook* );

  signals:
    /**
      This signal is emitted when the resource has finished the loading of all
      addressees from the backend to the internal cache.

      @param resource The pointer to the resource which emitted this signal.
     */
    void loadingFinished( Resource *resource );

    /**
      This signal is emitted when an error occured during loading the
      addressees from the backend to the internal cache.

      @param resource The pointer to the resource which emitted this signal.
      @param msg A translated error message.
     */
    void loadingError( Resource *resource, const QString &msg );

    /**
      This signal is emitted when the resource has finished the saving of all
      addressees from the internal cache to the backend.

      @param resource The pointer to the resource which emitted this signal.
     */
    void savingFinished( Resource *resource );

    /**
      This signal is emitted when an error occured during saving the
      addressees from the internal cache to the backend.

      @param resource The pointer to the resource which emitted this signal.
      @param msg A translated error message.
     */
    void savingError( Resource *resource, const QString &msg );

  protected:
    Ticket *createTicket( Resource * );
    Addressee::Map mAddrMap;

  private:
    AddressBook *mAddressBook;

    class ResourcePrivate;
    ResourcePrivate *d;
};

}

#endif
