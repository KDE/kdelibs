#ifndef KABC_RESOURCE_H
#define KABC_RESOURCE_H

#include "addressbook.h"

#define RES_VCARD	0 // default
#define RES_BINARY	1
#define RES_SQL		2


namespace KABC {

/**
  @short Helper class for handling coordinated save of address books.
  
  This class is used as helper class for saving address book.
  @see requestSaveTicket(), save().
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
  @internal
*/
class Resource {
  public:
    Resource( AddressBook * );
    virtual ~Resource();
  
    virtual bool open();
    virtual void close();
  
    virtual Ticket *requestSaveTicket();
  
    virtual bool load();
    virtual bool save( Ticket * );

    AddressBook *addressBook() { return mAddressBook; }

    virtual QString identifier();

  protected:
    Ticket *createTicket( Resource * );

  private:
    AddressBook *mAddressBook;
};

}

#endif
