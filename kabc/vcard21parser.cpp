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

#include "vcard21parser.h"
#include <qregexp.h>
using namespace KABC;


static QValueList<QString> tokenizeBy(const QString& str, const QRegExp& tok, bool keepEmpties = false) {
QValueList<QString> tokens;
unsigned int head, tail;
unsigned int length = str.length();

  if (length < 1) return tokens;

  if (length == 1) {
    tokens.append(str);
    return tokens;
  }

  for(head = 0, tail = 0; tail < length-1; head = tail+1) {
    QString thisline;

    tail = str.find(tok, head);

    if (tail > length)           // last token - none at end
      tail = length;

    if (tail-head > 0 || keepEmpties) {    // it has to be at least 1 long!
      thisline = str.mid(head, tail-head);
      tokens.append(thisline);
    }
  }
return tokens;
}


bool VCardLine::isValid() const {

  // invalid: if it is "begin:vcard" or "end:vcard"
  if (name == VCARD_BEGIN_N || name == VCARD_END_N)
    return false;

  if (name[0] == 'x' && name[1] == '-')      // a custom x- line
    return true;

  // this is long but it makes it a bit faster (and saves me from using
  // a trie which is probably the ideal situation, but a bit memory heavy)
  switch(name[0]) {
  case 'a':
    // GS - this seems to not be necessary? - netscape doesn't do it
//     if (name == VCARD_ADR && qualified &&
//                             (qualifiers.contains(VCARD_ADR_DOM)    ||
//                              qualifiers.contains(VCARD_ADR_INTL)   ||
//                              qualifiers.contains(VCARD_ADR_POSTAL) ||
//                              qualifiers.contains(VCARD_ADR_HOME)   ||
//                              qualifiers.contains(VCARD_ADR_WORK)   ||
//                              qualifiers.contains(VCARD_ADR_PREF)
//                             ))
    if (name == VCARD_ADR)
      return true;
    if (name == VCARD_AGENT)
      return true;
   break;
  case 'b':
    if (name == VCARD_BDAY)
      return true;
   break;
  case 'c':
    if (name == VCARD_CATEGORIES)
      return true;
    if (name == VCARD_CLASS && qualified &&
                              (qualifiers.contains(VCARD_CLASS_PUBLIC)      ||
                               qualifiers.contains(VCARD_CLASS_PRIVATE)     ||
                               qualifiers.contains(VCARD_CLASS_CONFIDENTIAL)
                              ))
      return true;
   break;
  case 'd':
   break;
  case 'e':
    if (name == VCARD_EMAIL && qualified &&
                              (qualifiers.contains(VCARD_EMAIL_INTERNET) ||
                               qualifiers.contains(VCARD_EMAIL_PREF)     ||
                               qualifiers.contains(VCARD_EMAIL_X400)
                              ))
      return true;
   break;
  case 'f':
    if (name == VCARD_FN)
      return true;
   break;
  case 'g':
    if (name == VCARD_GEO)
      return true;
   break;
  case 'h':
   break;
  case 'i':
   break;
  case 'j':
   break;
  case 'k':
    if (name == VCARD_KEY && qualified &&
                            (qualifiers.contains(VCARD_KEY_X509) ||
                             qualifiers.contains(VCARD_KEY_PGP)
                            ))
      return true;
   break;
  case 'l':
    if (name == VCARD_LABEL)
      return true;
    if (name == VCARD_LOGO)
      return true;
   break;
  case 'm':
    if (name == VCARD_MAILER)
      return true;
   break;
  case 'n':
    if (name == VCARD_N)
      return true;
    if (name == VCARD_NAME)
      return true;
    if (name == VCARD_NICKNAME)
      return true;
    if (name == VCARD_NOTE)
      return true;
   break;
  case 'o':
    if (name == VCARD_ORG)
      return true;
   break;
  case 'p':
    if (name == VCARD_PHOTO)
      return true;
    if (name == VCARD_PROFILE)
      return true;
    if (name == VCARD_PRODID)
      return true;
   break;
  case 'q':
   break;
  case 'r':
    if (name == VCARD_ROLE)
      return true;
    if (name == VCARD_REV)
      return true;
   break;
  case 's':
    if (name == VCARD_SOURCE)
      return true;
    if (name == VCARD_SOUND)
      return true;
   break;
  case 't':
    if (name == VCARD_TEL && qualified &&
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
                            ))
      return true;
    if (name == VCARD_TZ)
      return true;
    if (name == VCARD_TITLE)
      return true;
   break;
  case 'u':
    if (name == VCARD_URL)
      return true;
    if (name == VCARD_UID)
      return true;
   break;
  case 'v':
    if (name == VCARD_VERSION)
      return true;
   break;
  case 'w':
   break;
  case 'x':
   break;
  case 'y':
   break;
  case 'z':
   break;
  default:
   break;
  }

return false;
}


void VCardLine::qpDecode(QString& x) {
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

enum { Home = 1, Work = 2, Msg = 4, Pref = 8, Voice = 16, Fax = 32,
           Cell = 64, Video = 128, Bbs = 256, Modem = 512, Car = 1024,
           Isdn = 2048, Pcs = 4096, Pager = 8192 };
		
	
	//set the addressees name and formated name
	QStringList tmpList = mVCard->getValues(VCARD_N);
	int tmpInt = 0;
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
	PhoneNumber *tmpNumber;
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_HOME);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Home);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_WORK);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Work);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_MSG);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Msg);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_PREF);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Pref);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_VOICE);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Voice);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_FAX);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Fax);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_CELL);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Cell);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_VIDEO);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Video);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_BBS);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Bbs);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_MODEM);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Modem);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_CAR);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Car);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_MODEM);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Pager);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_ISDN);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Isdn);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_PCS);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Pcs);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	tmpStr = mVCard->getValue(VCARD_TEL,VCARD_TEL_PAGER);
	if (!tmpStr.isEmpty()) {
	tmpNumber = new PhoneNumber(tmpStr,Pager);
	addressee.insertPhoneNumber(*tmpNumber);
	}
	//set the addressee's addresses

	if (!mVCard->getValues(VCARD_ADR).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR),64));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_DOM).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_DOM),1));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_INTL).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_INTL),2));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_POSTAL).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_POSTAL),4));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_PARCEL).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_PARCEL),8));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_HOME).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_HOME),16));
	}
	if (!mVCard->getValues(VCARD_ADR,VCARD_ADR_WORK).isEmpty()) {
	addressee.insertAddress(readAddressFromQStringList(mVCard->getValues(VCARD_ADR,VCARD_ADR_WORK),32));
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



KABC::Address VCard21Parser::readAddressFromQStringList (const QStringList &data, const int type)
{
      KABC::Address mAddress;
      mAddress.setType(type);
      if (data.count() > 0)
        mAddress.setPostOfficeBox(data[0]);
      if (data.count() > 1)
        mAddress.setExtended(data[1]);
      if (data.count() > 2)
        mAddress.setStreet(data[2]);
      if (data.count() > 3)
        mAddress.setLocality(data[3]);
      if (data.count() > 4)
        mAddress.setRegion(data[4]);
      if (data.count() > 5)
        mAddress.setPostalCode(data[5]);
      if (data.count() > 6)
        mAddress.setCountry(data[6]);
      return mAddress;
}




VCard21ParserImpl* VCard21ParserImpl::parseVCard(const QString& vc, int *err)
{
int _err = 0;
int _state = VC_STATE_BEGIN;
QValueList<VCardLine> *_vcdata;
QValueList<QString> lines;

  _vcdata = new QValueList<VCardLine>;

  // lines = tokenizeBy(vc, '\n');
  lines = tokenizeBy(vc, QRegExp("[\x0d\x0a]"));

  // for each line in the vCard
  for (QValueListIterator<QString> j = lines.begin();
                                   j != lines.end(); ++j) {
    VCardLine _vcl;

    // take spaces off the end - ugly but necessary hack
    for (int g = (*j).length()-1; g > 0 && (*j)[g].isSpace(); g++)
      (*j)[g] = 0;

    //        first token:
    //              verify state, update if necessary
    if (_state & VC_STATE_BEGIN) {
      if (!qstricmp((*j).latin1(), VCARD_BEGIN)) {
        _state = VC_STATE_BODY;
        continue;
      } else {
        _err = VC_ERR_NO_BEGIN;
        break;
      }
    } else if (_state & VC_STATE_BODY) {
      if (!qstricmp((*j).latin1(), VCARD_END)) {
        _state |= VC_STATE_END;
        break;
      }

      // split into two tokens
      // QValueList<QString> linetokens = tokenizeBy(*j, QRegExp(":"));
      unsigned int tail = (*j).find(':', 0);
      if (tail > (*j).length()) {  // invalid line - no ':'
        _err = VC_ERR_INVALID_LINE;
        break;
      }

      QValueList<QString> linetokens;
      QString tmplinetoken;
      tmplinetoken = (*j);
      tmplinetoken.truncate(tail);
      linetokens.append(tmplinetoken);
      tmplinetoken = (*j).mid(tail+1);
      linetokens.append(tmplinetoken);

      // check for qualifiers and
      // set name, qualified, qualifier(s)
      //QValueList<QString> nametokens = tokenizeBy(linetokens[0], ';');
      QValueList<QString> nametokens = tokenizeBy(linetokens[0], QRegExp(";"));
      bool qp = false, first_pass = true;
      bool b64 = false;

      if (nametokens.count() > 0) {
        _vcl.qualified = false;
        _vcl.name = nametokens[0];
        _vcl.name = _vcl.name.lower();
        for (QValueListIterator<QString> z = nametokens.begin();
                                         z != nametokens.end();
                                         ++z) {
          QString zz = (*z).lower();
          if (zz == VCARD_QUOTED_PRINTABLE || zz == VCARD_ENCODING_QUOTED_PRINTABLE) {
            qp = true;
          } else if (zz == VCARD_BASE64) {
            b64 = true;
	  } else if (!first_pass) {
            _vcl.qualified = true;
            _vcl.qualifiers.append(zz);
          }
          first_pass = false;
	}
      } else {
        _err = VC_ERR_INVALID_LINE;
      }

      if (_err != 0) break;

      if (_vcl.name == VCARD_VERSION)
        _state |= VC_STATE_HAVE_VERSION;

      if (_vcl.name == VCARD_N || _vcl.name == VCARD_FN)
        _state |= VC_STATE_HAVE_N;

      // second token:
      //    split into tokens by ;
      //    add to parameters vector
      //_vcl.parameters = tokenizeBy(linetokens[1], ';');
      if (b64) {
        if (linetokens[1][linetokens[1].length()-1] != '=')
          do {
            linetokens[1] += *(++j);
          } while ((*j)[(*j).length()-1] != '=');
      } else {
        if (qp) {        // join any split lines
          while (linetokens[1][linetokens[1].length()-1] == '=') {
            linetokens[1].remove(linetokens[1].length()-1, 1);
            linetokens[1].append(*(++j));
          }
        }
        _vcl.parameters = tokenizeBy(linetokens[1], QRegExp(";"), true);
        if (qp) {        // decode the quoted printable
          for (QValueListIterator<QString> z = _vcl.parameters.begin();
                                           z != _vcl.parameters.end();
	                                   ++z) {
            _vcl.qpDecode(*z);
          }
        }
      }
    } else {
      _err = VC_ERR_INTERNAL;
      break;
    }

    // validate VCardLine
    if (!_vcl.isValid()) {
      _err = VC_ERR_INVALID_LINE;
      break;
    }

    // add to vector
    _vcdata->append(_vcl);
  }

  // errors to check at the last minute (exit state related)
  if (_err == 0) {
    if (!(_state & VC_STATE_END))         // we have to have an end!!
      _err = VC_ERR_NO_END;

    if (!(_state & VC_STATE_HAVE_N) ||    // we have to have the mandatories!
        !(_state & VC_STATE_HAVE_VERSION))
      _err = VC_ERR_MISSING_MANDATORY;
  }

  // set the error message if we can, and only return an object
  // if the vCard was valid.

  if (err)
    *err = _err;

  if (_err != 0) {
    delete _vcdata;
    return NULL;
  }

  return new VCard21ParserImpl(_vcdata);
}


VCard21ParserImpl::VCard21ParserImpl(QValueList<VCardLine> *_vcd) : _vcdata(_vcd) {

}


QString VCard21ParserImpl::getValue(const QString& name, const QString& qualifier) {
QString failed = "";
const QString lowname = name.lower();
const QString lowqualifier = qualifier.lower();

  for (QValueListIterator<VCardLine> i = _vcdata->begin();
                                     i != _vcdata->end();
                                     ++i) {
    if ((*i).name == lowname && (*i).qualified && (*i).qualifiers.contains(lowqualifier)) {
      if ((*i).parameters.count() > 0)
        return (*i).parameters[0];
      else return failed;
    }
  }
return failed;
}


QString VCard21ParserImpl::getValue(const QString& name) {
QString failed = "";
const QString lowname = name.lower();

  for (QValueListIterator<VCardLine> i = _vcdata->begin();
                                     i != _vcdata->end();
                                     ++i) {
    if ((*i).name == lowname && !(*i).qualified) {
      if ((*i).parameters.count() > 0)
        return (*i).parameters[0];
      else return failed;
    }
  }
return failed;
}


QStringList VCard21ParserImpl::getValues(const QString& name) {
//QString failedstr = "";
QStringList failed;
const QString lowname = name.lower();
  for (QValueListIterator<VCardLine> i = _vcdata->begin();
                                     i != _vcdata->end();
                                     ++i) {
    if ((*i).name == lowname && !(*i).qualified) {
      return (*i).parameters;
    }
  }
//failed.append(failedstr);
return failed;
}

QStringList VCard21ParserImpl::getValues(const QString& name, const QString& qualifier) {
//QString failedstr = "";
QStringList failed;
const QString lowname = name.lower();
const QString lowqualifier = qualifier.lower();
  for (QValueListIterator<VCardLine> i = _vcdata->begin();
                                     i != _vcdata->end();
                                     ++i) {
    if ((*i).name == lowname && (*i).qualified && (*i).qualifiers.contains(lowqualifier)) {
      return (*i).parameters;
    }
  }
//failed.append(failedstr);
return failed;
}



