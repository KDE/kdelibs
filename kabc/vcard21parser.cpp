/*
    This file is part of libkabc.
    Copyright (c) 2001 Mark Westcott <mark@houseoffish.org>

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

#include <qmap.h>
#include <qregexp.h>

#include "vcard21parser.h"

using namespace KABC;

bool VCardLine::isValid() const
{
  // Invalid: if it is "begin:vcard" or "end:vcard"
  if ( name == VCARD_BEGIN_N || name == VCARD_END_N )
    return false;

  if ( name[0] == 'x' && name[1] == '-' ) // A custom x- line
    return true;

  // This is long but it makes it a bit faster (and saves me from using
  // a trie which is probably the ideal situation, but a bit memory heavy)
  switch( name[0] ) {
    case 'a':
      if ( name == VCARD_ADR && qualified &&
                   (qualifiers.contains(VCARD_ADR_DOM)    ||
                    qualifiers.contains(VCARD_ADR_INTL)   ||
                    qualifiers.contains(VCARD_ADR_POSTAL) ||
                    qualifiers.contains(VCARD_ADR_HOME)   ||
                    qualifiers.contains(VCARD_ADR_WORK)   ||
                    qualifiers.contains(VCARD_ADR_PREF)
         ) )
        return true;

      if ( name == VCARD_AGENT )
        return true;
      break;

    case 'b':
      if ( name == VCARD_BDAY )
        return true;
      break;

    case 'c':
      if ( name == VCARD_CATEGORIES )
        return true;
      if ( name == VCARD_CLASS && qualified &&
                   (qualifiers.contains(VCARD_CLASS_PUBLIC)      ||
                    qualifiers.contains(VCARD_CLASS_PRIVATE)     ||
                    qualifiers.contains(VCARD_CLASS_CONFIDENTIAL)
         ) )
        return true;
      break;

    case 'e':
      if ( name == VCARD_EMAIL && qualified &&
                  (qualifiers.contains(VCARD_EMAIL_INTERNET) ||
                   qualifiers.contains(VCARD_EMAIL_PREF)     ||
                   qualifiers.contains(VCARD_EMAIL_X400)
         ) )
        return true;
      break;

    case 'f':
      if ( name == VCARD_FN )
        return true;
      break;

    case 'g':
      if ( name == VCARD_GEO )
        return true;
      break;

    case 'k':
      if ( name == VCARD_KEY && qualified &&
                   (qualifiers.contains(VCARD_KEY_X509) ||
                    qualifiers.contains(VCARD_KEY_PGP)
         ) )
        return true;
      break;

    case 'l':
      if ( name == VCARD_LABEL )
        return true;
      if ( name == VCARD_LOGO )
        return true;
      break;

    case 'm':
      if ( name == VCARD_MAILER )
        return true;
      break;

    case 'n':
      if ( name == VCARD_N )
        return true;
      if ( name == VCARD_NAME )
        return true;
      if ( name == VCARD_NICKNAME )
        return true;
      if ( name == VCARD_NOTE )
        return true;
      break;

    case 'o':
      if ( name == VCARD_ORG )
        return true;
      break;

    case 'p':
      if ( name == VCARD_PHOTO )
        return true;
      if ( name == VCARD_PROFILE )
        return true;
      if ( name == VCARD_PRODID )
        return true;
      break;

    case 'r':
      if ( name == VCARD_ROLE )
        return true;
      if ( name == VCARD_REV )
        return true;
      break;

    case 's':
      if ( name == VCARD_SOURCE )
        return true;
      if ( name == VCARD_SOUND )
        return true;
      break;

    case 't':
      if ( name == VCARD_TEL && qualified &&
                   (qualifiers.contains(VCARD_TEL_HOME)  ||
                    qualifiers.contains(VCARD_TEL_WORK)  ||
                    qualifiers.contains(VCARD_TEL_PREF)  ||
                    qualifiers.contains(VCARD_TEL_VOICE) ||
                    qualifiers.contains(VCARD_TEL_FAX)   ||
                    qualifiers.contains(VCARD_TEL_MSG)   ||
                    qualifiers.contains(VCARD_TEL_CELL)  ||
                    qualifiers.contains(VCARD_TEL_PAGER) ||
                    qualifiers.contains(VCARD_TEL_BBS)   ||
                    qualifiers.contains(VCARD_TEL_MODEM) ||
                    qualifiers.contains(VCARD_TEL_CAR)   ||
                    qualifiers.contains(VCARD_TEL_ISDN)  ||
                    qualifiers.contains(VCARD_TEL_VIDEO) ||
                    qualifiers.contains(VCARD_TEL_PCS)
         ) )
        return true;
      if ( name == VCARD_TZ )
        return true;
      if ( name == VCARD_TITLE )
        return true;
      break;

    case 'u':
      if ( name == VCARD_URL )
        return true;
      if ( name == VCARD_UID )
        return true;
      break;

    case 'v':
      if ( name == VCARD_VERSION )
        return true;
      break;
    default:
      break;
  }

  return false;
}


void VCardLine::qpDecode( QString& x )
{
  QString y = x;
  int c;

  x = "";
  c = y.length();

  for (int i = 0; i < c; i++) {
    if (y[i] == '=') {
      char p = y[++i].latin1();
      char q = y[++i].latin1();
      x += (char) ((p <= '9' ? p - '0': p - 'A' + 10)*16 +
                  (q <= '9' ? q - '0': q - 'A' + 10));
    } else {
      x += y[i];
    }
  }
}


VCard21Parser::VCard21Parser()
{
}

VCard21Parser::~VCard21Parser()
{
}

void VCard21Parser::readFromString(KABC::AddressBook *addressbook, const QString &data)
{
  KABC::Addressee mAddressee = readFromString(data);
  addressbook->insertAddressee(mAddressee);
}

KABC::Addressee VCard21Parser::readFromString( const QString &data)
{
  KABC::Addressee addressee;
  VCard21ParserImpl *mVCard = VCard21ParserImpl::parseVCard(data);
  QString tmpStr;

  // Check if parsing failed
  if (mVCard == 0)
  {
     kdDebug() << "Parsing failed" << endl;
     return addressee;
  }         
  //set the addressees name and formated name
  QStringList tmpList = mVCard->getValues(VCARD_N);
  QString formattedName = "";
  if (tmpList.count() > 0)
    addressee.setFamilyName(tmpList[0]);
  if (tmpList.count() > 1)
    addressee.setGivenName(tmpList[1]);
  if (tmpList.count() > 2)
    addressee.setAdditionalName(tmpList[2]);
  if (tmpList.count() > 3)
    addressee.setPrefix(tmpList[3]);
  if (tmpList.count() > 4)
    addressee.setSuffix(tmpList[4]);

  tmpStr = (mVCard->getValue(VCARD_FN));
  if (!tmpStr.isEmpty())
    addressee.setFormattedName(tmpStr);

  //set the addressee's nick name
  tmpStr = mVCard->getValue(VCARD_NICKNAME);
  addressee.setNickName(tmpStr);
  //set the addressee's organisation
  tmpStr = mVCard->getValue(VCARD_ORG);
  addressee.setOrganization(tmpStr);
  //set the addressee's title
  tmpStr = mVCard->getValue(VCARD_TITLE);
  addressee.setTitle(tmpStr);
  //set the addressee's email - we can only deal with two.  The preferenced one and one other.
  tmpStr = mVCard->getValue(VCARD_EMAIL, VCARD_EMAIL_INTERNET);
  addressee.insertEmail(tmpStr, false);
  tmpStr = mVCard->getValue(VCARD_EMAIL,VCARD_EMAIL_PREF);
  addressee.insertEmail(tmpStr, true);
  //set the addressee's url
  tmpStr = mVCard->getValue(VCARD_URL);

  if (!tmpStr.isEmpty()) {
    KURL url(tmpStr);
    addressee.setUrl(url);
  }

  //set the addressee's birthday
  tmpStr = mVCard->getValue(VCARD_BDAY);
  addressee.setBirthday(QDateTime::fromString(tmpStr,Qt::ISODate));

  //set the addressee's phone numbers
  for ( QValueListIterator<VCardLine> i = mVCard->_vcdata->begin();i != mVCard->_vcdata->end(); ++i ) {
    if ( (*i).name == VCARD_TEL ) {
      int type = 0;
      if ( (*i).qualified ) {
        if ( (*i).qualifiers.contains( VCARD_TEL_HOME ) )
          type |= PhoneNumber::Home;
        if ( (*i).qualifiers.contains( VCARD_TEL_WORK ) )
          type |= PhoneNumber::Work;
        if ( (*i).qualifiers.contains( VCARD_TEL_PREF ) )
          type |= PhoneNumber::Pref;
        if ( (*i).qualifiers.contains( VCARD_TEL_VOICE ) )
          type |= PhoneNumber::Voice;
        if ( (*i).qualifiers.contains( VCARD_TEL_FAX ) )
          type |= PhoneNumber::Fax;
        if ( (*i).qualifiers.contains( VCARD_TEL_MSG ) )
          type |= PhoneNumber::Msg;
        if ( (*i).qualifiers.contains( VCARD_TEL_CELL ) )
          type |= PhoneNumber::Cell;
        if ( (*i).qualifiers.contains( VCARD_TEL_PAGER ) )
          type |= PhoneNumber::Pager;
        if ( (*i).qualifiers.contains( VCARD_TEL_BBS ) )
          type |= PhoneNumber::Bbs;
        if ( (*i).qualifiers.contains( VCARD_TEL_MODEM ) )
          type |= PhoneNumber::Modem;
        if ( (*i).qualifiers.contains( VCARD_TEL_CAR ) )
          type |= PhoneNumber::Car;
        if ( (*i).qualifiers.contains( VCARD_TEL_ISDN ) )
          type |= PhoneNumber::Isdn;
        if ( (*i).qualifiers.contains( VCARD_TEL_VIDEO ) )
          type |= PhoneNumber::Video;
        if ( (*i).qualifiers.contains( VCARD_TEL_PCS ) )
          type |= PhoneNumber::Pcs;
      }
      addressee.insertPhoneNumber( PhoneNumber( (*i).parameters[ 0 ], type ) );
    }
  }

  //set the addressee's addresses
  for ( QValueListIterator<VCardLine> i = mVCard->_vcdata->begin();i != mVCard->_vcdata->end(); ++i ) {
    if ( (*i).name == VCARD_ADR ) {
      int type = 0;
      if ( (*i).qualified ) {
        if ( (*i).qualifiers.contains( VCARD_ADR_DOM ) )
          type |= Address::Dom;
        if ( (*i).qualifiers.contains( VCARD_ADR_INTL ) )
          type |= Address::Intl;
        if ( (*i).qualifiers.contains( VCARD_ADR_POSTAL ) )
          type |= Address::Postal;
        if ( (*i).qualifiers.contains( VCARD_ADR_PARCEL ) )
          type |= Address::Parcel;
        if ( (*i).qualifiers.contains( VCARD_ADR_HOME ) )
          type |= Address::Home;
        if ( (*i).qualifiers.contains( VCARD_ADR_WORK ) )
          type |= Address::Work;
        if ( (*i).qualifiers.contains( VCARD_ADR_PREF ) )
          type |= Address::Pref;
      }
      addressee.insertAddress( readAddressFromQStringList( (*i).parameters, type ) );
    }
  }

  //set the addressee's delivery label
  tmpStr = mVCard->getValue(VCARD_LABEL);
  if (!tmpStr.isEmpty()) {
    tmpStr.replace(QRegExp("\\r\\n"),"\n");
    Address tmpAddress;
    tmpAddress.setLabel(tmpStr);
    addressee.insertAddress(tmpAddress);
  }

  //set the addressee's notes
  tmpStr = mVCard->getValue(VCARD_NOTE);
  tmpStr.replace(QRegExp("\\r\\n"),"\n");
  addressee.setNote(tmpStr);

  //set the addressee's timezone
  tmpStr = mVCard->getValue(VCARD_TZ);
  TimeZone tmpZone(tmpStr.toInt());
  addressee.setTimeZone(tmpZone);

  //set the addressee's geographical position
  tmpList = mVCard->getValues(VCARD_GEO);
  if (tmpList.count()==2)
  {
    tmpStr = tmpList[0];
    float glat = tmpStr.toFloat();
    tmpStr = tmpList[1];
    float glong = tmpStr.toFloat();
    Geo tmpGeo(glat,glong);
    addressee.setGeo(tmpGeo);
  }

  //set the last revision date
  tmpStr = mVCard->getValue(VCARD_REV);
  addressee.setRevision(QDateTime::fromString(tmpStr,Qt::ISODate));

  //set the role of the addressee
  tmpStr = mVCard->getValue(VCARD_ROLE);
  addressee.setRole(tmpStr);

  return addressee;
}



KABC::Address VCard21Parser::readAddressFromQStringList ( const QStringList &data, const int type )
{
  KABC::Address mAddress;
  mAddress.setType( type );

  if ( data.count() > 0 )
    mAddress.setPostOfficeBox( data[0] );
  if ( data.count() > 1 )
    mAddress.setExtended( data[1] );
  if ( data.count() > 2 )
    mAddress.setStreet( data[2] );
  if ( data.count() > 3 )
    mAddress.setLocality( data[3] );
  if ( data.count() > 4 )
    mAddress.setRegion( data[4] );
  if ( data.count() > 5 )
    mAddress.setPostalCode( data[5] );
  if ( data.count() > 6 )
    mAddress.setCountry( data[6] );

  return mAddress;
}


VCard21ParserImpl *VCard21ParserImpl::parseVCard( const QString& vc, int *err ) 
{
  int _err = 0;
  int _state = VC_STATE_BEGIN;

  QValueList<VCardLine> *_vcdata;
  QValueList<QString> lines;

  _vcdata = new QValueList<VCardLine>;

  lines = QStringList::split( QRegExp( "[\x0d\x0a]" ), vc );
  
  // for each line in the vCard
  for ( QStringList::Iterator j = lines.begin(); j != lines.end(); ++j ) {
    VCardLine _vcl;

    // take spaces off the end - ugly but necessary hack
    for ( int g = (*j).length()-1; g > 0 && (*j)[g].isSpace(); --g )
      (*j)[g] = 0;

    // first token:
    //   verify state, update if necessary
    if ( _state & VC_STATE_BEGIN) {
      if ( !qstricmp( (*j).latin1(), VCARD_BEGIN ) ) {
        _state = VC_STATE_BODY;
        continue;
      } else {
        _err = VC_ERR_NO_BEGIN;
        break;
      }
    } else if ( _state & VC_STATE_BODY ) {
      if ( !qstricmp( (*j).latin1(), VCARD_END ) ) {
        _state |= VC_STATE_END;
        break;
      }

      // split into two tokens
      int colon = (*j).find( ':' );
      if ( colon < 0 ) {
        _err = VC_ERR_INVALID_LINE;
        break;
      }

      QString key = (*j).left( colon );
      QString value = (*j).mid( colon + 1 );

      // check for qualifiers and
      // set name, qualified, qualifier(s)
      QStringList keyTokens = QStringList::split( QRegExp(";"), key );
      bool qp = false, first_pass = true;
      bool b64 = false;

      if ( keyTokens.count() > 0 ) {
        _vcl.qualified = false;
        _vcl.name = keyTokens[ 0 ].lower();

        for ( QStringList::Iterator z = keyTokens.begin(); z != keyTokens.end(); ++z ) {
          QString zz = (*z).lower();
          if ( zz == VCARD_QUOTED_PRINTABLE || zz == VCARD_ENCODING_QUOTED_PRINTABLE ) {
            qp = true;
          } else if ( zz == VCARD_BASE64 ) {
            b64 = true;
          } else if ( !first_pass ) {
            _vcl.qualified = true;
            _vcl.qualifiers.append( zz );
          }
          first_pass = false;
        }
      } else {
        _err = VC_ERR_INVALID_LINE;
      }

      if ( _err != 0 )
        break;

      if ( _vcl.name == VCARD_VERSION )
        _state |= VC_STATE_HAVE_VERSION;

      if ( _vcl.name == VCARD_N || _vcl.name == VCARD_FN )
        _state |= VC_STATE_HAVE_N;

      // second token:
      //    split into tokens by ;
      //    add to parameters vector
      if ( b64 ) {
        if ( value[ value.length() - 1 ] != '=' )
          do {
            value += *( ++j );
          } while ( (*j)[ (*j).length() - 1 ] != '=' );
      } else {
        if ( qp ) { // join any split lines
          while ( value[ value.length() - 1 ] == '=' ) {
            value.remove( value.length() - 1, 1 );
            value.append(*( ++j ));
          }
        }
        _vcl.parameters = QStringList::split( QRegExp(";"), value, true );
        if ( qp ) { // decode the quoted printable
          for ( QStringList::Iterator z = _vcl.parameters.begin(); z != _vcl.parameters.end(); ++z )
            _vcl.qpDecode( *z );
        }
      }
    } else {
      _err = VC_ERR_INTERNAL;
      break;
    }

    // validate VCardLine
    if ( !_vcl.isValid() ) {
      _err = VC_ERR_INVALID_LINE;
      break;
    }

    // add to vector
    _vcdata->append( _vcl );
  }

  // errors to check at the last minute (exit state related)
  if ( _err == 0 ) {
    if ( !( _state & VC_STATE_END ) ) // we have to have an end!!
      _err = VC_ERR_NO_END;

    if ( !( _state & VC_STATE_HAVE_N ) || // we have to have the mandatories!
         !( _state & VC_STATE_HAVE_VERSION ) )
      _err = VC_ERR_MISSING_MANDATORY;
  }

  // set the error message if we can, and only return an object
  // if the vCard was valid.
  if ( err )
    *err = _err;

  if ( _err != 0 ) {
    delete _vcdata;
    return 0;
  }

  return new VCard21ParserImpl( _vcdata );
}

VCard21ParserImpl::VCard21ParserImpl(QValueList<VCardLine> *_vcd) : _vcdata(_vcd)
{
}


QString VCard21ParserImpl::getValue(const QString& name, const QString& qualifier)
{
  QString failed = "";
  const QString lowname = name.lower();
  const QString lowqualifier = qualifier.lower();

  for (QValueListIterator<VCardLine> i = _vcdata->begin();i != _vcdata->end();++i) {
   if ((*i).name == lowname && (*i).qualified && (*i).qualifiers.contains(lowqualifier)) {
    if ((*i).parameters.count() > 0)
     return (*i).parameters[0];
    else return failed;
    }
  }
  return failed;
}


QString VCard21ParserImpl::getValue(const QString& name)
{
  QString failed = "";
  const QString lowname = name.lower();

  for (QValueListIterator<VCardLine> i = _vcdata->begin();i != _vcdata->end();++i) {
    if ((*i).name == lowname && !(*i).qualified) {
      if ((*i).parameters.count() > 0)
        return (*i).parameters[0];
      else return failed;
    }
   }
  return failed;
}


QStringList VCard21ParserImpl::getValues(const QString& name)
{
  //QString failedstr = "";
  QStringList failed;
  const QString lowname = name.lower();
  for (QValueListIterator<VCardLine> i = _vcdata->begin();i != _vcdata->end();++i) {
    if ((*i).name == lowname && !(*i).qualified)
      return (*i).parameters;
  }
  //failed.append(failedstr);
  return failed;
}

QStringList VCard21ParserImpl::getValues(const QString& name, const QString& qualifier)
{
  //QString failedstr = "";
  QStringList failed;
  const QString lowname = name.lower();
  const QString lowqualifier = qualifier.lower();
  for (QValueListIterator<VCardLine> i = _vcdata->begin();i != _vcdata->end();++i) {
    if ((*i).name == lowname && (*i).qualified && (*i).qualifiers.contains(lowqualifier))
       return (*i).parameters;
  }
  //failed.append(failedstr);
  return failed;
}
