#include <kdebug.h>

#include "resource.h"

using namespace KABC;

Resource::Resource( AddressBook *ab ) :
  mAddressBook( ab )
{
}

Resource::~Resource()
{
}

bool Resource::open()
{
  return true;
}

void Resource::close()
{
}

Ticket *Resource::requestSaveTicket()
{
  return 0;
}

bool Resource::load( AddressBook *ab )
{
  mAddressBook = ab;
  return true;
}

bool Resource::save( Ticket * )
{
  return false;
}

Ticket *Resource::createTicket( Resource *resource )
{
  return new Ticket( resource );
}

void Resource::setAddressBook( AddressBook *ab )
{
  mAddressBook = ab;
}

QString Resource::identifier()
{
    return "NoIdentifier";
}
