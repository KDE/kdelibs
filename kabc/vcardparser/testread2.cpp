#include "testutils.h"
#include <kabc/addressee.h>
#include <vcardconverter.h>
#include <kdebug.h>

using namespace KABC;

int
main()
{
    Addressee::List l = vCardsAsAddresseeList();
    QByteArray vcards = vCardsAsText();

    VCardConverter vct;

    Addressee::List parsed = vct.parseVCards( vcards );

    if ( l.size() != parsed.size() ) {
        kdDebug()<<"\tSize - FAILED : "<<l.size()<<" vs. parsed "<<parsed.size()<<endl;
    } else {
        kdDebug()<<"\tSize - PASSED"<<endl;
    }

    Addressee::List::iterator itr1;
    Addressee::List::iterator itr2;
    for ( itr1 = l.begin(), itr2 = parsed.begin();
          itr1 != l.end(); ++itr1, ++itr2 ) {
        if ( (*itr1).fullEmail() == (*itr2).fullEmail() &&
             (*itr1).organization() == (*itr2).organization() &&
             (*itr1).phoneNumbers() == (*itr2).phoneNumbers()  &&
             (*itr1).emails() == (*itr2).emails() &&
             (*itr1).role() == (*itr2).role()  ) {
            kdDebug()<<"\tAddressee  - PASSED"<<endl;
            kdDebug()<<"\t\t"<< (*itr1).fullEmail() << " VS. " << (*itr2).fullEmail()<<endl;
        } else {
            kdDebug()<<"\tAddressee  - FAILED"<<endl;
            (*itr1).dump();
            (*itr2).dump();
            //kdDebug()<<"\t\t"<< (*itr1).fullEmail() << " VS. " << (*itr2).fullEmail()<<endl;
        }
    }
}
