/*
    This file is part of libkabc.
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General  Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "ldif.h"

using namespace KABC;

LDIF::LDIF()
{
  startParsing();
}

LDIF::~LDIF()
{
}

QByteArray LDIF::assembleLine( const QString &fieldname, const QByteArray &value,
  uint linelen, bool url )
{
  bool safe = false;
  bool isDn;
  QByteArray result;
  int i;

  if ( url ) {
    result = fieldname.toUtf8() + ":< " + value;
  } else {
    isDn = fieldname.toLower() == "dn";
    //SAFE-INIT-CHAR
    if ( value.size() > 0 && value[0] > 0 && value[0] != '\n' &&
      value[0] != '\r' && value[0] != ':' && value[0] != '<' ) safe = true;

    //SAFE-CHAR
    if ( safe ) {
      for ( i=1; i < value.size(); i++ ) {
      //allow utf-8 in Distinguished Names
        if ( ( isDn && value[i] == 0 ) ||
             ( !isDn && value[i] <= 0 ) ||
             value[i] == '\r' || value[i] == '\n' ) {
          safe = false;
          break;
        }
      }
    }

    if ( value.size() == 0 ) safe = true;

    if( safe ) {
      result = fieldname.toUtf8() + ": " + value;
    } else {
      result = fieldname.toUtf8() + ":: " + value.toBase64();
    }

    if ( linelen > 0 ) {
      i = (uint)(fieldname.length()+2) > linelen ? fieldname.length()+2 : linelen;
      while ( i < result.length() ) {
        result.insert( i, "\n " );
        i += linelen+2;
      }
    }
  }
  return result;
}

QByteArray LDIF::assembleLine( const QString &fieldname, const QString &value,
  uint linelen, bool url )
{
  return assembleLine( fieldname, value.toUtf8(), linelen, url );
}

bool LDIF::splitLine( const QByteArray &line, QString &fieldname, QByteArray &value )
{
  int position;
  QByteArray tmp;
  int linelen;

//  kdDebug(5700) << "splitLine line: " << QString::fromUtf8(line) << endl;

  position = line.find( ":" );
  if ( position == -1 ) {
    // strange: we did not find a fieldname
    fieldname = "";
    value = line.trimmed();
//    kdDebug(5700) << "value : " << value[0] << endl;
    return false;
  }

  linelen = line.size();
  fieldname = QString::fromUtf8( line.left( position ).trimmed() );

  if ( linelen > ( position + 1 ) && line[ position + 1 ] == ':' ) {
    // String is BASE64 encoded -> decode it now.
    if ( linelen <= ( position + 3 ) ) {
      value.resize( 0 );
      return false;
    }
    value = QByteArray::fromBase64( line.mid( position + 3 ) );
    return false;
  }

  if ( linelen > ( position + 1 ) && line[ position + 1 ] == '<' ) {
    // String is an URL.
    if ( linelen <= ( position + 3 ) ) {
      value.resize( 0 );
      return false;
    }
    value = QByteArray::fromBase64( line.mid( position + 3 ) );
    return true;
  }

  if ( linelen <= ( position + 2 ) ) {
    value.resize( 0 );
    return false;
  }
  value = line.mid( position + 2 );
  return false;
}

bool LDIF::splitControl( const QByteArray &line, QString &oid, bool &critical, 
  QByteArray &value )
{
  QString tmp;
  critical = false;
  bool url = splitLine( line, tmp, value );
  
  kdDebug(5700) << "splitControl: value: " << QString::fromUtf8(value, value.size()) << endl;
  if ( tmp.isEmpty() ) {
    tmp = QString::fromUtf8( value, value.size() );
    value.resize( 0 );
  }
  if ( tmp.right( 4 ) == "true" ) {
    critical = true;
    tmp.truncate( tmp.length() - 5 );
  } else  if ( tmp.right( 5 ) == "false" ) {
    critical = false;
    tmp.truncate( tmp.length() - 6 );
  }
  oid = tmp;
  return url;
}

LDIF::ParseVal LDIF::processLine() 
{

  if ( mIsComment ) return None;

  ParseVal retval = None;
  if ( mLastParseVal == EndEntry ) mEntryType = Entry_None;

  mUrl = splitLine( line, mAttr, mVal );

  QString attrLower = mAttr.toLower();

  switch ( mEntryType ) {
    case Entry_None:
      if ( attrLower == "version" ) {
        if ( !mDn.isEmpty() ) retval = Err;
      } else if ( attrLower == "dn" ) {
        kdDebug(5700) << "ldapentry dn: " << QString::fromUtf8( mVal, mVal.size() ) << endl;
        mDn = QString::fromUtf8( mVal, mVal.size() );
        mModType = Mod_None;
        retval = NewEntry;
      } else if ( attrLower == "changetype" ) {
        if ( mDn.isEmpty() )
          retval = Err;
        else {
          QString tmpval = QString::fromUtf8( mVal, mVal.size() );
          kdDebug(5700) << "changetype: " << tmpval << endl;
          if ( tmpval == "add" ) mEntryType = Entry_Add;
          else if ( tmpval == "delete" ) mEntryType = Entry_Del;
          else if ( tmpval == "modrdn" || tmpval == "moddn" ) {
            mNewRdn = "";
            mNewSuperior = "";
            mDelOldRdn = true;
            mEntryType = Entry_Modrdn;
          }
          else if ( tmpval == "modify" ) mEntryType = Entry_Mod;
          else retval = Err;
        }
      } else if ( attrLower == "control" ) {
        mUrl = splitControl( mVal, mOid, mCritical, mVal );
        retval = Control;
      } else if ( !mAttr.isEmpty() && mVal.size() > 0 ) {
        mEntryType = Entry_Add;
        retval = Item;
      }
      break;
    case Entry_Add:
      if ( mAttr.isEmpty() && mVal.size() == 0 )
        retval = EndEntry;
      else
        retval = Item;
      break;
    case Entry_Del:
      if ( mAttr.isEmpty() && mVal.size() == 0 )
        retval = EndEntry;
      else
        retval = Err;
      break;
    case Entry_Mod:
      if ( mModType == Mod_None ) {
        kdDebug(5700) << "kio_ldap: new modtype " << mAttr << endl;
        if ( mAttr.isEmpty() && mVal.size() == 0 ) {
          retval = EndEntry;
        } else if ( attrLower == "add" ) {
          mModType = Mod_Add;
        } else if ( attrLower == "replace" ) {
          mModType = Mod_Replace;
          mAttr = QString::fromUtf8( mVal, mVal.size() );
          mVal.resize( 0 );
          retval = Item;
        } else if ( attrLower == "delete" ) {
          mModType = Mod_Del;
          mAttr = QString::fromUtf8( mVal, mVal.size() );
          mVal.resize( 0 );
          retval = Item;
        } else {
          retval = Err;
        }
      } else {
        if ( mAttr.isEmpty() ) {
          if ( QString::fromUtf8( mVal, mVal.size() ) == "-" ) {
            mModType = Mod_None;
          } else if ( mVal.size() == 0 ) {
            retval = EndEntry;
          } else
            retval = Err;
        } else
          retval = Item;
      }
      break;
    case Entry_Modrdn:
      if ( mAttr.isEmpty() && mVal.size() == 0 )
        retval = EndEntry;
      else if ( attrLower == "newrdn" )
        mNewRdn = QString::fromUtf8( mVal, mVal.size() );
      else if ( attrLower == "newsuperior" )
        mNewSuperior = QString::fromUtf8( mVal, mVal.size() );
      else if ( attrLower == "deleteoldrdn" ) {
        if ( mVal.size() > 0 && mVal[0] == '0' )
          mDelOldRdn = false;
        else if ( mVal.size() > 0 && mVal[0] == '1' )
          mDelOldRdn = true;
        else
          retval = Err;
      } else
        retval = Err;
      break;
  }
  return retval;
}

LDIF::ParseVal LDIF::nextItem()
{
  ParseVal retval = None;
  char c=0;

  while( retval == None ) {
    if ( mPos < (uint)mLdif.size() ) {
      c = mLdif[mPos];
      mPos++;
      if ( mIsNewLine && c == '\r' ) continue; //handle \n\r line end
      if ( mIsNewLine && ( c == ' ' || c == '\t' ) ) { //line folding
        mIsNewLine = false;
        continue;
      }
      if ( mIsNewLine ) {
        mIsNewLine = false;
        retval = processLine();
        mLastParseVal = retval;
        line.resize( 0 );
        mIsComment = ( c == '#' );
      }
      if ( c == '\n' || c == '\r' ) {
        mLineNo++;
        mIsNewLine = true;
        continue;
      }
    } else {
      retval = MoreData;
      break;
    }

    if ( !mIsComment ) line += c;
  }
  return retval;
}

void LDIF::endLDIF()
{
  QByteArray tmp( 3 );
  tmp[ 0 ] = '\n';
  tmp[ 1 ] = '\n';
  tmp[ 2 ] = '\n';
  mLdif = tmp;
  mPos = 0;
}

void LDIF::startParsing()
{
  mPos = mLineNo = 0;
  mDelOldRdn = false;
  mEntryType = Entry_None;
  mModType = Mod_None;
  mDn = mNewRdn = mNewSuperior = "";
  line = "";
  mIsNewLine = false;
  mIsComment = false;
  mLastParseVal = None;
}
