/*
    This file is part of libkabc.

    Copyright (c) 2002 Mark Westcott <mark@houseoffish.org>
    Copyright (c) 2000 George Staikos <staikos@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_VCARD21FORMAT_H
#define KABC_VCARD21FORMAT_H

#include <qdatetime.h>
#include <kdebug.h>
#include <qregexp.h>
#include <qstring.h>
#include <kurl.h>
#include <qvaluelist.h>

#include "addressee.h"
#include "addressbook.h"
#include "phonenumber.h"


#define VCARD_BEGIN          "begin:vcard"
#define VCARD_END            "end:vcard"
#define VCARD_BEGIN_N        "begin"
#define VCARD_END_N          "end"
#define VCARD_VERSION        "version"

#define VCARD_FN             "fn"
#define VCARD_N              "n"

// optional
#define VCARD_NAME           "name"
#define VCARD_NICKNAME       "nickname"
#define VCARD_PHOTO          "photo"
#define VCARD_BDAY           "bday"
#define VCARD_ADR            "adr"
  
// types
#define VCARD_ADR_DOM      "dom"
#define VCARD_ADR_INTL     "intl"
#define VCARD_ADR_POSTAL   "postal"
#define VCARD_ADR_PARCEL   "parcel"
#define VCARD_ADR_HOME     "home"
#define VCARD_ADR_WORK     "work"
#define VCARD_ADR_PREF     "pref"
// values
#define VCARD_ADR_POBOX    "PO Box"
#define VCARD_ADR_EXTADR   "Extended Address"
#define VCARD_ADR_STREET   "Street"
#define VCARD_ADR_LOCALITY "Locality"
#define VCARD_ADR_REGION   "Region"
#define VCARD_ADR_POSTCODE "Postal Code"
#define VCARD_ADR_COUNTRY  "Country Name"
#define VCARD_LABEL          "label"
#define VCARD_PROFILE        "profile"
#define VCARD_SOURCE         "source"
#define VCARD_TEL            "tel"
// types
#define VCARD_TEL_HOME     "home"
#define VCARD_TEL_WORK     "work"
#define VCARD_TEL_PREF     "pref"
#define VCARD_TEL_VOICE    "voice"
#define VCARD_TEL_FAX      "fax"
#define VCARD_TEL_MSG      "msg"
#define VCARD_TEL_CELL     "cell"
#define VCARD_TEL_PAGER    "pager"
#define VCARD_TEL_BBS      "bbs"
#define VCARD_TEL_MODEM    "modem"
#define VCARD_TEL_CAR      "car"
#define VCARD_TEL_ISDN     "isdn"
#define VCARD_TEL_VIDEO    "video"
#define VCARD_TEL_PCS      "pcs"
#define VCARD_EMAIL        "email"
// types
#define VCARD_EMAIL_PREF    "pref"
#define VCARD_EMAIL_INTERNET "internet"
#define VCARD_EMAIL_X400   "x400"
#define VCARD_TZ             "tz"
#define VCARD_GEO            "geo"
#define VCARD_MAILER         "mailer"
#define VCARD_TITLE          "title"
#define VCARD_ROLE           "role"
#define VCARD_LOGO           "logo"
#define VCARD_AGENT          "agent"
#define VCARD_ORG            "org"
#define VCARD_CATEGORIES     "categories"
#define VCARD_NOTE           "note"
#define VCARD_PRODID         "prodid"
#define VCARD_REV            "rev"
#define VCARD_SOUND          "sound"
#define VCARD_UID            "uid"
#define VCARD_URL            "url"
#define VCARD_CLASS          "class"
#define VCARD_CLASS_PUBLIC "public"
#define VCARD_CLASS_PRIVATE "private"
#define VCARD_CLASS_CONFIDENTIAL "confidential"
#define VCARD_KEY            "key"
// types
#define VCARD_KEY_X509     "x509"
#define VCARD_KEY_PGP      "pgp"

#define VCARD_QUOTED_PRINTABLE "quoted-printable"
// this one is a temporary hack until we support TYPE=VALUE
#define VCARD_ENCODING_QUOTED_PRINTABLE "encoding=quoted-printable"
#define VCARD_BASE64           "base64"

#define VC_STATE_BEGIN          1
#define VC_STATE_BODY           2
#define VC_STATE_END            4
#define VC_STATE_HAVE_N         8
#define VC_STATE_HAVE_VERSION  16

#define VC_ERR_NO_BEGIN            1
#define VC_ERR_NO_END              2
#define VC_ERR_INVALID_LINE        3
#define VC_ERR_INTERNAL            4
#define VC_ERR_INVALID_NAME        5
#define VC_ERR_MISSING_MANDATORY   6

namespace KABC {

class AddressBook;

class VCard21Parser
{
public:

  /**
   * Constructor.
   */
  VCard21Parser();

  /**
   * Destructor.
   */
  virtual ~VCard21Parser();

  /**
   * Parses a string in vcard2.1 format and saves the single addressees
   * to the address book.
   *
   * @param ab  The address book.
   * @param str The vcard string.
   */
  void readFromString( KABC::AddressBook *ab, const QString &str );

  /**
   * FIXME: we need a writeToString method
   * QString writeToString (KABC::AddressBook *);
   */

  /**
   * Parses a string in vcard2.1 format and returns the inherent addressee.
   */
  KABC::Addressee readFromString( const QString &data);

  /**
   * Helper method to store a address.
   *
   * @param data  A string list, that is filled with 'street', 'house number' ...
   * @param type  The type of the returned address.
   */
  static KABC::Address readAddressFromQStringList (const QStringList &data, const int type);
};

}

/**
 * @short Helper class
 */
class VCardLine
{
public:
  QString name;
  bool qualified;
  QValueList<QString> qualifiers;
  QValueList<QString> parameters;
  bool isValid() const;
  void qpEncode(QString& x);
  void qpDecode(QString& x);
};

/**
 * @short Helper class
 */
class VCard21ParserImpl
{
  friend class VCardLine;

public:
  VCard21ParserImpl() {  };
  virtual ~VCard21ParserImpl() {  };
  static VCard21ParserImpl *parseVCard(const QString& vc, int *err = NULL);
  QString getValue(const QString& name, const QString& qualifier);
  QString getValue(const QString& name);
  QStringList getValues(const QString& name, const QString& qualifier);
  QStringList getValues(const QString& name);

  QValueList<VCardLine> *_vcdata;

private:
  VCard21ParserImpl (QValueList<VCardLine> *_vcd);
};

#endif
