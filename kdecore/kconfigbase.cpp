/* 
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   
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

#include <stdlib.h>

#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include "kconfigbase.h"
#include "kconfigbackend.h"

KConfigBase::KConfigBase()
  : backEnd(0L), bDirty(false), bLocaleInitialized(false), 
    bReadOnly(false), bExpand(true)
{
}

KConfigBase::~KConfigBase()
{
}

void KConfigBase::setLocale()
{
  bLocaleInitialized = true;

  if (KGlobal::locale())
    aLocaleString = KGlobal::locale()->language();
  else
    aLocaleString = "C";
}

void KConfigBase::setGroup( const QString& pGroup )
{
  if ( pGroup.isNull() )
    aGroup = "<default>";
  else if (pGroup.find("Desktop Entry") != -1) {
    debug("warning, setting Desktop Entry group through KConfig::setGroup() is deprecated.");
    debug("please use KConfig::setDesktopGroup() instead.");
    abort();
    setDesktopGroup();
  } else
    aGroup = pGroup;
}
 
void KConfigBase::setDesktopGroup()
{
  // we maintain the first for backwards compatibility with
  // old .kdelnk files
  if (hasGroup("KDE Desktop Entry"))
    aGroup = "KDE Desktop Entry";
  else
    aGroup = "Desktop Entry";
}

QString KConfigBase::readEntry( const QString& aKey, 
				const QString& aDefault ) const
{
  // we need to access _locale instead of the method locale()
  // because calling locale() will create a locale object if it
  // doesn't exist, which requires KConfig, which will create a infinite
  // loop, and nobody likes those.
  if (!bLocaleInitialized && KGlobal::_locale) {
    // get around const'ness.
    KConfigBase *that = (KConfigBase *)this;
    that->setLocale();
  }

  QString aValue = QString::null;

  // construct a localized version of the key
  QString aLocalizedKey = QString( aKey );
  aLocalizedKey += "[";
  aLocalizedKey += aLocaleString;
  aLocalizedKey += "]";
  // find the value for the key in the current group

  // try the localized key first
  KEntry aEntryData;
  KEntryKey entryKey = { aGroup, aLocalizedKey };
  aEntryData = lookupData(entryKey);
  if (!aEntryData.aValue.isNull()) {
    aValue = aEntryData.aValue;
  } else {
    entryKey.key = aKey;
    aEntryData = lookupData(entryKey);
    if (!aEntryData.aValue.isNull()) {
      aValue = aEntryData.aValue;
    } else {
      aValue = aDefault;
    }
  }

  // only do dollar expansion if so desired
  if( bExpand )
    {
      // check for environment variables and make necessary translations
      int nDollarPos = aValue.find( '$' );
	  
      while( nDollarPos != -1 && nDollarPos+1 < static_cast<int>(aValue.length())) {
	// there is at least one $
	if( (aValue)[nDollarPos+1] != '$' ) {
	  uint nEndPos = nDollarPos;
	  // the next character is no $
	  do {
	    nEndPos++;
	  } while ( aValue[nEndPos].isNumber()
		    || aValue[nEndPos].isLetter() || nEndPos > aValue.length() );
	  QString aVarName = aValue.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
	  const char* pEnv = getenv( aVarName.ascii() );
	  if( pEnv )
	    aValue.replace( nDollarPos, nEndPos-nDollarPos, pEnv );
	  else
	    aValue.remove( nDollarPos, nEndPos-nDollarPos );
	} else {
	  // remove one of the dollar signs
	  aValue.remove( nDollarPos, 1 );
	  nDollarPos++;
	}
	nDollarPos = aValue.find( '$', nDollarPos );
      }
    }

  return aValue;
}

QProperty KConfigBase::readPropertyEntry( const QString& aKey,
					  QProperty::Type type ) const
{ 
  switch( type )
    {
    case QProperty::Empty:
      return QProperty();
    case QProperty::StringType:
      return QProperty( readEntry( aKey ) );
    case QProperty::StringListType:
      return QProperty( readListEntry( aKey ) );
    case QProperty::IntListType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::DoubleListType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::FontType:
      return QProperty( readFontEntry( aKey ) );
    case QProperty::PixmapType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::ImageType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::BrushType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::PointType:
      return QProperty( readPointEntry( aKey ) );
    case QProperty::RectType:
      return QProperty( readRectEntry( aKey ) );
    case QProperty::SizeType:
      return QProperty( readSizeEntry( aKey ) );
    case QProperty::ColorType:
      return QProperty( readColorEntry( aKey ) );
    case QProperty::PaletteType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::ColorGroupType:
      ASSERT( 0 );
      return QProperty();
    case QProperty::IntType:
      return QProperty( readNumEntry( aKey ) );
    case QProperty::BoolType:
      return QProperty( readBoolEntry( aKey ) );
    case QProperty::DoubleType:
      return QProperty( readDoubleNumEntry( aKey ) );
    default:
      ASSERT( 0 );
    }  

  // Never reached
  return QProperty();
}

int KConfigBase::readListEntry( const QString& pKey, 
				QStrList &list, char sep ) const
{
  if( !hasKey( pKey ) )
    return 0;
  
  QString str_list, value;
  str_list = readEntry( pKey );
  if (str_list.isEmpty())
    return 0;

  list.clear();
  int i;
  value = "";
  int len = str_list.length();

  for (i = 0; i < len; i++) {
    if (str_list[i] != sep && str_list[i] != '\\') {
      value += str_list[i];
      continue;
    }
    if (str_list[i] == '\\') {
      i++;
      value += str_list[i];
      continue;
    }
    // if we fell through to here, we are at a separator.  Append
    // contents of value to the list
    list.append( value.ascii() );
    value.truncate(0);
  }
  
  if ( str_list[len-1].latin1() != sep )
    list.append( value.ascii() );
  return list.count();
  }
  
QStringList KConfigBase::readListEntry( const QString& pKey, char sep ) const
{
  QStringList list;
  if( !hasKey( pKey ) )
    return list;
  QString str_list, value;
  str_list = readEntry( pKey );
  if( str_list.isEmpty() )
    return list; 
  int i;
  value = "";
  int len = str_list.length();
  for( i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
	{
	  value += str_list[i];
	  continue;
	}
      if( str_list[i] == '\\' )
	{
	  i++;
	  value += str_list[i];
	  continue;
	}
      list.append( value );
      value.truncate(0);
    }
  if ( str_list[len-1] != sep )
    list.append( value );
  return list;
}

int KConfigBase::readNumEntry( const QString& pKey, int nDefault) const
{
  bool ok;
  int rc;

  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else if( aValue == "true" )
    return 1;
  else if( aValue == "on" )
    return 1;
  else
    {
      rc = aValue.toInt( &ok );
      return( ok ? rc : 0 );
    }
}


unsigned int KConfigBase::readUnsignedNumEntry( const QString& pKey, unsigned int nDefault) const
{
  bool ok;
  unsigned int rc;
  
  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      rc = aValue.toUInt( &ok );
      return( ok ? rc : 0 );
    }
}


long KConfigBase::readLongNumEntry( const QString& pKey, long nDefault) const
{
  bool ok;
  long rc;

  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      rc = aValue.toLong( &ok );
      return( ok ? rc : 0 );
    }
}


unsigned long KConfigBase::readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault) const
{
  bool ok;
  unsigned long rc;

  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      rc = aValue.toULong( &ok );
      return( ok ? rc : 0 );
    }
}


double KConfigBase::readDoubleNumEntry( const QString& pKey, double nDefault) const
{
  bool ok;
  double rc;

  QString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      rc = aValue.toDouble( &ok );
      return( ok ? rc : 0 );
    }
}


bool KConfigBase::readBoolEntry( const QString& pKey, const bool bDefault ) const
{
  QString aValue = readEntry( pKey );

  if( aValue.isNull() )
    return bDefault;
  else
    {
      if( aValue == "true" || aValue == "on" || aValue == "1")
	return true;
      else
	{
	  bool bOK;
	  int val = aValue.toInt( &bOK );
	  if( bOK && val != 0 )
	    return true;
	  else
	    return false;
	}
    }
}

 


QFont KConfigBase::readFontEntry( const QString& pKey,
				  const QFont* pDefault ) const
{
  QFont aRetFont;

  QString aValue = readEntry( pKey );
  if( !aValue.isNull() )
    {
      // find first part (font family)
      int nIndex = aValue.find( ',' );
      if( nIndex == -1 ){
	if( pDefault )
	  aRetFont = *pDefault;
	return aRetFont;
      }
      aRetFont.setFamily( aValue.left( nIndex ) );
      
      // find second part (point size)
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if( nIndex == -1 ){
	if( pDefault )
	  aRetFont = *pDefault;
	return aRetFont;
      }
      
      aRetFont.setPointSize( aValue.mid( nOldIndex+1, 
					 nIndex-nOldIndex-1 ).toInt() );
      
      // find third part (style hint)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      
      if( nIndex == -1 ){
	if( pDefault )
	  aRetFont = *pDefault;
	return aRetFont;
      }
      
      aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toUInt() );
      
      // find fourth part (char set)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      
      if( nIndex == -1 ){
	if( pDefault )
	  aRetFont = *pDefault;
	return aRetFont;
      }
      
      QString chStr=aValue.mid( nOldIndex+1, 
				nIndex-nOldIndex-1 );
      bool chOldEntry;			 
      QFont::CharSet chId=(QFont::CharSet)aValue.mid( nOldIndex+1, 
						      nIndex-nOldIndex-1 ).toUInt(&chOldEntry);			   
      if (chOldEntry)  
	aRetFont.setCharSet( chId );
      else if (kapp) {
	if (chStr == "default")
	  if (KGlobal::locale())
	    chStr = KGlobal::locale()->charset();
	  else chStr = "iso-8859-1";
	kapp->getCharsets()->setQFont(aRetFont,chStr);
      }
      
      // find fifth part (weight)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      
      if( nIndex == -1 ){
	if( pDefault )
	  aRetFont = *pDefault;
	return aRetFont;
      }
      
      aRetFont.setWeight( aValue.mid( nOldIndex+1,
				      nIndex-nOldIndex-1 ).toUInt() );
      
      // find sixth part (font bits)
      uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();
      if( nFontBits & 0x01 )
	aRetFont.setItalic( true );
      else
	aRetFont.setItalic( false );
      
      if( nFontBits & 0x02 )
	aRetFont.setUnderline( true );
      else
	aRetFont.setUnderline( false );
      
      if( nFontBits & 0x04 )
	aRetFont.setStrikeOut( true );
      else
	aRetFont.setStrikeOut( false );
      
      if( nFontBits & 0x08 )
	aRetFont.setFixedPitch( true );
      else
	aRetFont.setFixedPitch( false );
      
      if( nFontBits & 0x20 )
	aRetFont.setRawMode( true );
      else
	aRetFont.setRawMode( false );
    }
  else
    {
      if( pDefault )
	aRetFont = *pDefault;
    }
  
  return aRetFont;
}


QRect KConfigBase::readRectEntry( const QString& pKey, const QRect* pDefault ) const
{
  QStrList list;
  
  if( !hasKey( pKey ) )
    {
      if( pDefault )
	return *pDefault;
      else
	return QRect();
    }
  
  int count = readListEntry( pKey, list, ',' );
  if( count != 4 )
    return QRect();
  else
    return QRect( QString( list.at( 0 ) ).toInt(), 
		  QString( list.at( 1 ) ).toInt(), 
		  QString( list.at( 2 ) ).toInt(), 
		  QString( list.at( 3 ) ).toInt() );
}


QPoint KConfigBase::readPointEntry( const QString& pKey,
				    const QPoint* pDefault ) const
{
  QStrList list;
  
  if( !hasKey( pKey ) )
    {
      if( pDefault )
	return *pDefault;
      else
	return QPoint();
    }
  
  int count = readListEntry( pKey, list, ',' );
  if( count != 2 )
    return QPoint();
  else
    return QPoint( QString( list.at( 0 ) ).toInt(), 
		   QString( list.at( 1 ) ).toInt() );
}


QSize KConfigBase::readSizeEntry( const QString& pKey,
				  const QSize* pDefault ) const
{
  QStrList list;
  
  if( !hasKey( pKey ) )
    {
      if( pDefault )
	return *pDefault;
      else
	return QSize();
    }
  
  int count = readListEntry( pKey, list, ',' );
  if( count != 2 )
    return QSize();
  else
    return QSize( QString( list.at( 0 ) ).toInt(), 
		  QString( list.at( 1 ) ).toInt() );
}


QColor KConfigBase::readColorEntry( const QString& pKey, 
				    const QColor* pDefault ) const
{
  QColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;
  
  QString aValue = readEntry( pKey );
  if( !aValue.isEmpty() )
    {
      if ( aValue.left(1) == "#" ) 
        {
	  aRetColor.setNamedColor(aValue);
	} 
      else
	{
	  
	  bool bOK;
      
	  // find first part (red)
	  int nIndex = aValue.find( ',' );
	  
	  if( nIndex == -1 ){
	    // return a sensible default -- Bernd
	    if( pDefault )
	      aRetColor = *pDefault;
	    return aRetColor;
	  }
	  
	  nRed = aValue.left( nIndex ).toInt( &bOK );
	  
	  // find second part (green)
	  int nOldIndex = nIndex;
	  nIndex = aValue.find( ',', nOldIndex+1 );
	  
	  if( nIndex == -1 ){
	    // return a sensible default -- Bernd
	    if( pDefault )
	      aRetColor = *pDefault;
	    return aRetColor;
	  }
	  nGreen = aValue.mid( nOldIndex+1,
			       nIndex-nOldIndex-1 ).toInt( &bOK );
	  
	  // find third part (blue)
	  nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &bOK );
	  
	  aRetColor.setRgb( nRed, nGreen, nBlue );
	}
    }
  else {
    
    if( pDefault )
      aRetColor = *pDefault;
  }

  return aRetColor;
}


QString KConfigBase::writeEntry( const QString& pKey, const QString& value,
				 bool bPersistent, 
				 bool bGlobal,
				 bool bNLS )
{
  // the KConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  if( bPersistent )
    bDirty = true;

  if (!bLocaleInitialized && KGlobal::locale())
    setLocale();
  
  // if this is localized entry, add the locale
  QString aLocalizedKey = pKey;
  if( bNLS )
    aLocalizedKey = aLocalizedKey + '[' + aLocaleString + ']';

  KEntryKey entryKey = { aGroup, aLocalizedKey };
  KEntry aEntryData;
  QString aValue;
  
  // try to retrieve the current entry for this key
  aEntryData = lookupData(entryKey);
  if (!aEntryData.aValue.isNull()) {
    // there already is such a key
    aValue = aEntryData.aValue; // save old key as return value
  } 
  
  aEntryData.aValue = value;  // set new value
  aEntryData.bGlobal = bGlobal;
  aEntryData.bNLS = bNLS;
  if (bPersistent)
    aEntryData.bDirty = true;
  
  // rewrite the new value
  putData(entryKey, aEntryData);
  
  return aValue;
}

void KConfigBase::writeEntry ( const QString& pKey, const QProperty &prop, 
			       bool bPersistent, 
			       bool bGlobal, bool bNLS )
{
  switch( prop.type() )
    {
    case QProperty::Empty:
      writeEntry( pKey, "", bPersistent, bGlobal, bNLS );
      break;
    case QProperty::StringType:
      writeEntry( pKey, prop.stringValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::StringListType:
      writeEntry( pKey, prop.stringListValue(), ',', bPersistent, bGlobal, bNLS );
      break;
    case QProperty::IntListType:
      ASSERT( 0 );
      break;
    case QProperty::DoubleListType:
      ASSERT( 0 );
      break;
    case QProperty::FontType:
      writeEntry( pKey, prop.fontValue(), bPersistent, bGlobal, bNLS );
      break;
      // case QProperty::MovieType:
      // return "QMovie";
    case QProperty::PixmapType:
      ASSERT( 0 );
      break;
    case QProperty::ImageType:
      ASSERT( 0 );
      break;
    case QProperty::BrushType:
      ASSERT( 0 );
      break;
    case QProperty::PointType:
      writeEntry( pKey, prop.pointValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::RectType:
      writeEntry( pKey, prop.rectValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::SizeType:
      writeEntry( pKey, prop.sizeValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::ColorType:
      writeEntry( pKey, prop.colorValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::PaletteType:
      ASSERT( 0 );
      break;
    case QProperty::ColorGroupType:
      ASSERT( 0 );
      break;
    case QProperty::IntType:
      writeEntry( pKey, prop.intValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::BoolType:
      writeEntry( pKey, prop.boolValue(), bPersistent, bGlobal, bNLS );
      break;
    case QProperty::DoubleType:
      writeEntry( pKey, prop.doubleValue(), bPersistent, bGlobal, bNLS );
      break;
    default:
      ASSERT( 0 );
    }  
}

void KConfigBase::writeEntry ( const QString& pKey, const QStrList &list, 
			       char sep , bool bPersistent, 
			       bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString(""), bPersistent );      
      return;
    }
  QString str_list;
  QStrListIterator it( list );
  for( ; it.current(); ++it )
    {
      uint i;
      QString value = it.current();
      for( i = 0; i < value.length(); i++ )
	{
	  if( value[i] == sep || value[i] == '\\' )
	    str_list += '\\';
	  str_list += value[i];
	}
      str_list += sep;
    }
  if( str_list.right(1).at(0) == sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS );
}

void KConfigBase::writeEntry ( const QString& pKey, const QStringList &list, 
			       char sep , bool bPersistent, 
			       bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString(""), bPersistent );      
      return;
    }
  QString str_list;
  QStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      QString value = *it;
      uint i;
      for( i = 0; i < value.length(); i++ )
	{
	  if( value[i] == sep || value[i] == '\\' )
	    str_list += '\\';
	  str_list += value[i];
	}
      str_list += sep;
    }
  if( str_list.right(1).at(0) == sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS );
}

QString KConfigBase::writeEntry( const QString& pKey, int nValue,
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, unsigned int nValue,
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, long nValue,
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, unsigned long nValue,
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, double nValue,
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, bool bValue,
				 bool bPersistent, 
				 bool bGlobal,
				 bool bNLS )
{
  QString aValue;

  if( bValue )
    aValue = "true";
  else 
    aValue = "false";

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


QString KConfigBase::writeEntry( const QString& pKey, const QFont& rFont, 
				 bool bPersistent, bool bGlobal,
				 bool bNLS )
{
  QString aValue;
  QString aCharset;
  UINT8 nFontBits = 0;
  // this mimics get_font_bits() from qfont.cpp
  if( rFont.italic() )
    nFontBits = nFontBits | 0x01;
  if( rFont.underline() )
    nFontBits = nFontBits | 0x02;
  if( rFont.strikeOut() )
    nFontBits = nFontBits | 0x04;
  if( rFont.fixedPitch() )
    nFontBits = nFontBits | 0x08;
  if( rFont.rawMode() )
    nFontBits = nFontBits | 0x20;

  if (kapp) {
    KGlobal::charsets()->name(rFont);
  }
  else aCharset="default";
  aValue.sprintf( "%s,%d,%d,%s,%d,%d", rFont.family().ascii(), rFont.pointSize(),
		  rFont.styleHint(), aCharset.ascii(),
		  rFont.weight(), nFontBits );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QRect& rRect, 
			      bool bPersistent, bool bGlobal,
			      bool bNLS )
{
  QStrList list;
  QString tempstr;
  list.insert( 0, tempstr.setNum( rRect.left() ).ascii() );
  list.insert( 1, tempstr.setNum( rRect.top() ).ascii() );
  list.insert( 2, tempstr.setNum( rRect.width() ).ascii() );
  list.insert( 3, tempstr.setNum( rRect.height() ).ascii() );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QPoint& rPoint, 
			      bool bPersistent, bool bGlobal,
			      bool bNLS )
{
  QStrList list;
  QString tempstr;
  list.insert( 0, tempstr.setNum( rPoint.x() ).ascii() );
  list.insert( 1, tempstr.setNum( rPoint.y() ).ascii() );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QSize& rSize, 
			      bool bPersistent, bool bGlobal,
			      bool bNLS )
{
  QStrList list;
  QString tempstr;
  list.insert( 0, tempstr.setNum( rSize.width() ).ascii() );
  list.insert( 1, tempstr.setNum( rSize.height() ).ascii() );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QColor& rColor, 
			      bool bPersistent, 
			      bool bGlobal,
			      bool bNLS  )
{
  QString aValue;
  aValue.sprintf( "%d,%d,%d", rColor.red(), rColor.green(), rColor.blue() );

  writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}

void KConfigBase::parseConfigFiles(void) 
{ 
  backEnd->parseConfigFiles(); 
} 

void KConfigBase::sync()
{
  if (isReadOnly())
    return;
  
  backEnd->sync(true);
  if (bDirty)
    rollback();
}  

#include "kconfigbase.moc"

