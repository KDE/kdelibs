#include <kdebug.h>

#include "resource.h"

using namespace KABC;

Resource::Resource( AddressBook *ab ) :
  mAddressBook( ab )
{
    mAddressBook->addResource( this );
}

Resource::~Resource()
{
    mAddressBook->removeResource( this );
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

bool Resource::load()
{
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

QString Resource::identifier()
{
    return "NoIdentifier";
}
