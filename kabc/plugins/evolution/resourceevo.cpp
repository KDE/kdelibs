#include <qdir.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <stdio.h>

#include "dbwrapper.h"
#include "vcardconverter.h"
#include "resourceevo.h"

using namespace Evolution;
using namespace KABC;

extern "C"{
    Resource* resource( const KConfig* conf ) {
        KGlobal::locale()->insertCatalogue("kabc_evo");
        return new ResourceEvolution( conf );
    }
}

ResourceEvolution::ResourceEvolution( const KConfig* conf )
    : Resource( conf ), mWrap(0l)
{
}
ResourceEvolution::~ResourceEvolution() {
    delete mWrap;
}
bool ResourceEvolution::doOpen() {
    mWrap = new DBWrapper;
    if (!mWrap->open( QDir::homeDirPath() + "/evolution/local/Contacts/addressbook.db" ) ) {
        return false;
    }

    QString val;
    if (!mWrap->find( "PAS-DB-VERSION", val ) ) {
        return false;
    }
    if (!val.startsWith("0.2") ) {
        return false;
    }

    return true;
}
void ResourceEvolution::doClose() {
    delete mWrap;
    mWrap = 0l;
}
Ticket* ResourceEvolution::requestSaveTicket() {
    if ( !addressBook() ) return 0;
    return createTicket( this );
}
/*
 * skip the first key
 */

bool ResourceEvolution::load() {
    /* doOpen never get's called :( */
    if (!doOpen()) return false;
    if (!mWrap ) return false; // open first!

    DBIterator it = mWrap->begin();
    // skip the "PAS-DB-VERSION"
    VCardConverter conv;
    for ( ; it != mWrap->end(); ++it ) {
        if ( it.key().startsWith("PAS-DB-VERSION") )
            continue;

        Addressee adr;
        if (!conv.vCardToAddressee( it.value(), adr,  VCardConverter::v2_1 ) ) {
            kdDebug() << "Could not convert " << it.value() << endl;
            continue;
        }

        adr.setResource( this );
        addressBook()->insertAddressee( adr );
    }
    return true;
}
bool ResourceEvolution::save( Ticket* ticket ) {
    delete ticket;
    return true;
}
void ResourceEvolution::removeAddressee( const Addressee& ) {

}
