#ifndef KABC_RESOURCE_H
#define KABC_RESOURCE_H

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
class Resource : public Plugin
{
public:
  /**
   * Constructor
   */
  Resource( AddressBook *ab );

  /**
   * Destructor.
   */
  virtual ~Resource();

  /**
   * Open the resource and returns if it was successfully
   */
  virtual bool open();

  /**
   * Close the resource and returns if it was successfully
   */
  virtual void close();
  
  /**
   * Request a ticket, you have to pass through @ref save() to
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
   * @param ticket The ticket you get by @ref requestSaveTicket()
   */
  virtual bool save( Ticket *ticket );

  /**
   * Returns a pointer to the addressbook.
   */
  AddressBook *addressBook() { return mAddressBook; }

  /**
   * Returns a unique identifier.
   */
  virtual QString identifier() const;

  /**
   * Removes a addressee from resource. This method is mainly
   * used by record-based resources like LDAP or SQL.
   */
  virtual void removeAddressee( const Addressee& addr );

  /**
   * Mark the resource to read-only.
   */
  virtual void setReadOnly( bool value );

  /**
   * Returns, if the resource is read-only.
   */
  virtual bool readOnly() const;

  /**
   * Mark the resource as fast. Only fast resources will be
   * loaded when creating a StdAddressBook::self( true ).
   */
  virtual void setFastResource( bool value );

  /**
   * Returns, if the resource is fast.
   */
  virtual bool fastResource() const;

  /**
   * Set the name of resource.
   */
  virtual void setName( const QString &name );

  /**
   * Returns the name of resource.
   */
  virtual QString name() const;

  /**
   * This method can be used by all resources to encrypt
   * their passwords for storing in a config file.
   */
  static QString cryptStr( const QString & );

protected:
  Ticket *createTicket( Resource * );

private:
  AddressBook *mAddressBook;
  bool mReadOnly;
  bool mFastResource;
  QString mName;
};

}
#endif
