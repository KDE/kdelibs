#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <kabc/addressee.h>
#include <qstring.h>

KABC::Addressee vcard1();
KABC::Addressee vcard2();
KABC::Addressee vcard3();
KABC::Addressee::List vCardsAsAddresseeList();
QString   vCardAsText( const QString& location );
QString   vCardsAsText();

#endif
