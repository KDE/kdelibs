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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kmdcodec.h>

#include "ldif.h"

using namespace KABC;

LDIF::LDIF()
{
  startParsing(); 
}
    
LDIF::~LDIF()
{
}

QCString LDIF::assembleLine( const QString &fieldname, const QByteArray &value, 
  uint linelen, bool url )
{
  bool safe = false;
  bool isDn;
  QCString result;
  uint i;

  if ( url ) {    
    result = fieldname.utf8() + ":< " + QCString( value.data(), value.size()+1 );
  } else {
    isDn = fieldname.lower() == "dn";
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
      result = fieldname.utf8() + ": " + QCString( value.data(), value.size()+1 );
    } else {
      result = fieldname.utf8() + ":: " + KCodecs::base64Encode( value, false );
    }
  
    if ( linelen > 0 ) {
      i = (fieldname.length()+2) > linelen ? fieldname.length()+2 : linelen;
      while ( i < result.length() ) {
        result.insert( i, "\n " );
        i += linelen+2;
      }
    }
  }
  return result;
}

QCString LDIF::assembleLine( const QString &fieldname, const QCString &value, 
  uint linelen, bool url )
{
  QCString ret;
  QByteArray tmp;
  uint valuelen = value.length();
  tmp.setRawData( value, valuelen );
  ret = assembleLine( fieldname, tmp, linelen, url );
  tmp.resetRawData( value, valuelen );
  return ret;
  
}

QCString LDIF::assembleLine( const QString &fieldname, const QString &value, 
  uint linelen, bool url )
{
  return assembleLine( fieldname, value.utf8(), linelen, url );
}

bool LDIF::splitLine( const QCString &line, QString &fieldname, QByteArray &value )
{
  int position;
  QByteArray tmp;
  int linelen;

//  kdDebug(7125) << "splitLine line: " << QString::fromUtf8(line) << endl;
  
  position = line.find( ":" );
  if ( position == -1 ) {
    // strange: we did not find a fieldname
    fieldname = "";
    QCString str;
    str = line.stripWhiteSpace();
    linelen = str.length();
    tmp.setRawData( str.data(), linelen );
    value = tmp.copy();
    tmp.resetRawData( str.data(), linelen );
//    kdDebug(7125) << "value : " << value[0] << endl;
    return false;
  }
  
  linelen = line.length();
  
  if ( linelen > ( position + 1 ) && line[ position + 1 ] == ':' ) {
    // String is BASE64 encoded -> decode it now.
    fieldname = QString::fromUtf8( 
      line.left( position ).stripWhiteSpace() );
    if ( linelen <= ( position + 3 ) ) {
      value.resize( 0 );
      return false;
    }
    tmp.setRawData( &line.data()[ position + 3 ], linelen - position - 3 );
    KCodecs::base64Decode( tmp, value );
    tmp.resetRawData( &line.data()[ position + 3 ], linelen - position - 3 );
    return false;
  }
  
  if ( linelen > ( position + 1 ) && line[ position + 1 ] == '<' ) {
    // String is an URL.
    fieldname = QString::fromUtf8(
      line.left( position ).stripWhiteSpace() );
    if ( linelen <= ( position + 3 ) ) {
      value.resize( 0 );
      return false;
    }
    tmp.setRawData( &line.data()[ position + 3 ], linelen - position - 3 );
    value = tmp.copy();
    tmp.resetRawData( &line.data()[ position + 3 ], linelen - position - 3 );
    return true;
  }

  fieldname = QString::fromUtf8(line.left( position ).stripWhiteSpace());
  if ( linelen <= ( position + 2 ) ) {
    value.resize( 0 );
    return false;
  }
  tmp.setRawData( &line.data()[ position + 2 ], linelen - position - 2 );
  value = tmp.copy();
  tmp.resetRawData( &line.data()[ position + 2 ], linelen - position - 2 );
  return false;
}

LDIF::ParseVal LDIF::processLine() 
{
  
  if ( mIsComment ) return None;
  
  ParseVal retval = None;
  if ( mLastParseVal == EndEntry ) mEntryType = Entry_None;
      
  splitLine( line, mAttr, mVal );
  
  switch ( mEntryType ) {
    case Entry_None:
      if ( mAttr.lower() == "version" ) {
        if ( !mDn.isEmpty() ) retval = Err;
      } else if ( mAttr.lower() == "dn" ) {
        kdDebug(7125) << "ldapentry dn: " << QString::fromUtf8( mVal, mVal.size() ) << endl;
        mDn = QString::fromUtf8( mVal, mVal.size() ); 
        mModType = Mod_None;
        retval = NewEntry;
      } else if ( mAttr.lower() == "changetype" ) {
        if ( mDn.isEmpty() )
          retval = Err;
        else {
          QString tmpval = QString::fromUtf8( mVal, mVal.size() );
          kdDebug(7125) << "changetype: " << tmpval << endl;
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
      } else if ( mAttr.lower() == "control" ) {
      //TODO
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
        kdDebug(7125) << "kio_ldap: new modtype " << mAttr << endl;
        if ( mAttr.isEmpty() && mVal.size() == 0 ) {
          retval = EndEntry;
        } else if ( mAttr.lower() == "add" ) {
          mModType = Mod_Add; 
        } else if ( mAttr.lower() == "replace" ) {
          mModType = Mod_Replace;
          mAttr = QString::fromUtf8( mVal, mVal.size() );
          mVal.resize( 0 );
          retval = Item;
        } else if ( mAttr.lower() == "delete" ) {
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
      else if ( mAttr.lower() == "newrdn" ) 
        mNewRdn = QString::fromUtf8( mVal, mVal.size() ); 
      else if ( mAttr.lower() == "newsuperior" ) 
        mNewSuperior = QString::fromUtf8( mVal, mVal.size() ); 
      else if ( mAttr.lower() == "deleteoldrdn" ) {
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
    if ( mPos < mLdif.size() ) {
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
