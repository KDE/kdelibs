#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <QByteArray>
#include <QString>

#include <kabc/addressee.h>

KABC::Addressee vcard1();
KABC::Addressee vcard2();
KABC::Addressee vcard3();
KABC::Addressee::List vCardsAsAddresseeList();
QByteArray vCardAsText( const QString& location );
QByteArray vCardsAsText();

#endif
