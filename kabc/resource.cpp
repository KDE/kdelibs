#include <kdebug.h>

#include "resource.h"

using namespace KABC;

Resource::Resource( AddressBook *ab ) :
  mAddressBook( ab )
{
    mAddressBook->addResource( this );
    mReadOnly = true;
    mFastResource = true;
    mName = "NoName";
}

Resource::~Resource()
{
    mAddressBook->removeResource( this );
}

bool Resource::open()
{
    kdDebug() << "resource: standard" << endl;
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

void Resource::removeAddressee( const Addressee& )
{
    // do nothing
    kdDebug() << "resource: removeAddressee" << endl;
}

void Resource::setReadOnly( bool value )
{
	mReadOnly = value;
}

bool Resource::readOnly()
{
	return mReadOnly;
}

void Resource::setFastResource( bool value )
{
	mFastResource = value;
}

bool Resource::fastResource()
{
	return mFastResource;
}

void Resource::setName( const QString& name )
{
    mName = name;
}

QString Resource::name()
{
    return mName;
}
