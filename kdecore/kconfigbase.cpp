/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
// $Id$
//
// $Log$
// Revision 1.41  1999/03/09 23:38:16  kulow
// moved the QString wrapper to the header file as inline function
//
// Revision 1.40  1999/03/08 16:05:16  rbeutler
// added writeEntry( const QString &, const char *, ... ) method to fix the problem with the implicitely conversion to bool
//
// Revision 1.39  1999/03/02 16:22:20  kulow
// i18n is no longer a macro, but a function defined in klocale.h. So you
// don't need to include kapp.h when you want to use i18n. I see klocale->translate
// as obsolute (actually I seded it all over KDE :)
// I wanted to remove the #include <klocale.h> from kapp.h, but this broke
// too much, so I readded it after fixing half of kdeui. I guess I will
// write a script once that fixed compilation problems (I like the qt20fix way :),
// but for now it's only important to know that i18n works without kapp.h
//
// Revision 1.38  1999/03/01 23:33:20  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.37.2.3  1999/02/21 20:55:22  kulow
// more porting to Qt 2.0. It compiles and links. Jucheisassa :)
//
// Revision 1.37.2.2  1999/02/14 02:05:42  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.37.2.1  1999/01/30 20:18:54  kulow
// start porting to Qt2.0 beta
//
// Revision 1.37  1999/01/18 10:56:16  kulow
// .moc files are back in kdelibs. Built fine here using automake 1.3
//
// Revision 1.36  1999/01/15 09:30:34  kulow
// it's official - kdelibs builds with srcdir != builddir. For this I
// automocifized it, the generated rules are easier to maintain than
// selfwritten rules. I have to fight with some bugs of this tool, but
// generally it's better than keeping them updated by hand.
//
// Revision 1.35  1998/10/09 20:37:10  kalle
// KPanel works again
//
// Revision 1.34  1998/10/07 06:49:21  kalle
// Correctly read double dollar signs (patch by Harri Porten)
// Dollar expansion can be turned off with setDollarExpansion( false ).
// \sa isDollarExpansion
//
// Revision 1.33  1998/08/23 15:58:34  kulow
// fixed some more advanced warnings
//
// Revision 1.32  1998/07/29 09:07:48  ssk
// Fixed a whole lot of -Wall -ansi -pedantic warnings.
//
// Revision 1.31  1998/05/26 14:14:57  kalle
// Two bugfixes in KConfig:
//
// - security hole when saving to symlinks in SUID mode
// - trailing comma was needed in string list
//
// Revision 1.30  1998/05/04 20:08:13  ettrich
// Matthias: \n, \t, \r are stored as \\n, \\t and \\r now.
//
// Revision 1.29  1998/04/12 08:52:40  jacek
//
// * Updtated KCharset class documentation.
//
// * Chnged font entry in config files to store charset as string, not numerical
//   id (which worked only for iso-8859-* charsets)
//
// * added KCharset::xCharset method for getting charset name to use for X font
//   names.
//
// Revision 1.28  1998/03/31 10:13:29  mark
// MD: The Qt default font has rawMode set and KConfigBase::readFontEntry()
// never turned this setting off. Fixed this oversight and added a little more
// protection against future (unlikely) changes to the default font.
//
// Revision 1.27  1998/03/31 03:47:31  tibirna
// CT little typo in readNumEntry ( = instead of == )
//
// Revision 1.26  1998/03/30 17:30:51  kalle
// Accept "true" and "on" as 1 in readNumEntry
//
// Revision 1.25  1998/03/30 15:40:08  kalle
// Accepting non-null numerical values as true when reading bool entries
//
// Revision 1.24  1998/03/29 19:07:29  kalle
// Methods for reading and writing bool, unsigned int, long, unsigned long,
// double, QRect, QSize, QPoint
//
// Revision 1.23  1998/03/28 10:48:45  kulow
// reverted to the return aValue. It's not possible to return a copy, since
// the temporary object is destroyed in any case. What you need, is a kind of
// anchor for the QString object or a return value of const QString& 
//
// Revision 1.22  1998/03/27 23:41:34  torben
// Torben: Changed return aValue.copy() to
//          return QString( aValue.data() );
//
//         Hope it works now.
//
// Revision 1.21  1998/03/27 15:51:57  torben
// Torben: Bug fix. It was possible to modify internal data of kconfig.
//
// Revision 1.20  1998/03/11 11:23:28  kulow
// if the color entry starts with a '#', use QColor::setNamedColor
//
// Revision 1.19  1998/03/05 19:45:18  wuebben
// Bernd: several methods in kconfigbase.cpp did not the provided default
// values on parse error. -- fixed.
//
// Revision 1.18  1998/02/03 18:52:03  kulow
// added a static_cast
//
// Revision 1.17  1998/01/25 22:20:44  kulow
// applied patch by Kalle
//
// Revision 1.16  1998/01/18 14:38:50  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.14  1997/12/27 22:57:28  kulow
// I was a little bit nerved by the QFile warnings caused by the KApplication
// constructor, so I investigated a little bit ;) Fixed now
//
// Revision 1.13  1997/12/18 20:51:28  kalle
// Some patches by Alex and me
//
// Revision 1.12  1997/10/21 20:44:44  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.11  1997/10/16 21:51:15  torben
// Torben: Beta1, binary incompatible
//
// Revision 1.10  1997/10/16 11:14:30  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.8  1997/10/11 19:41:30  kalle
// Small bug fix for people who use KConfig without KApplication (not recommended!)
// Patch sent in by Jan Kneschke
//
// Revision 1.7  1997/10/10 19:24:11  kulow
// removed mutable and replace const_cast with a more portable way.
//
// Revision 1.6  1997/10/10 15:40:21  kulow
// forgot one default value
//
// Revision 1.5  1997/10/10 15:31:58  kulow
// removed default values from the implementation.
//
// Revision 1.4  1997/10/10 14:40:17  kulow
// reverted patch
//
// Revision 1.3  1997/10/09 11:46:25  kalle
// Assorted patches by Fritz Elfert, Rainer Bawidamann, Bernhard Kuhn and Lars Kneschke
//
// Revision 1.2  1997/10/05 02:31:10  jones
// MRJ: Changed const QString& readEntry( ... ) to QString readEntry( ... )
// I had to do this - almost everything was broken.
//
// Revision 1.1  1997/10/04 19:51:00  kalle
// new KConfig
//

#include <kconfigbase.h>

// Unix includes
#include <stdlib.h>
#include <ctype.h>

// Qt includes
#include <qfile.h>

// KDE includes
#include "kdebug.h"
#include "kapp.h"
#include "kcharsets.h"
#include "klocale.h"

static QString printableToString(const QString& s){
  if (!s.contains('\\'))
    return s;
  QString result="";
  unsigned int i = 0;
  if (s.length()>1){ // remember: s.length() is unsigned....
    for (i=0;i<s.length()-1;i++){
      if (s[i] == '\\'){
	i++;
	if (s[i] == '\\')
	  result.insert(result.length(), s[i]);
	else if (s[i] == 'n')
	  result.append("\n");
	else if (s[i] == 't')
	  result.append("\t");
	else if (s[i] == 'r')
	  result.append("\r");
	else {
	  result.append("\\");
	  result.insert(result.length(), s[i]);
	}
      }
      else
	result.insert(result.length(), s[i]);
    }
  }
  if (i<s.length())
    result.insert(result.length(), s[i]);
  return result;
}

KConfigBase::KConfigBase()
{
  pData = new KConfigBaseData();

  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  data()->aGroupDict.insert( "<default>", pDefGroup );
}


KConfigBase::~KConfigBase()
{
  delete pData;
}


void KConfigBase::setLocale()
{
  data()->bLocaleInitialized = true;

  KApplication *app = KApplication::getKApplication();
  
  if (app)
    pData->aLocaleString = app->getLocale()->language();
  else
    pData->aLocaleString = "C";
}

void KConfigBase::parseOneConfigFile( QFile& rFile, 
				      KGroupDict* pWriteBackDict,
				      bool bGlobal )
{
    if (!rFile.isOpen()) // come back, if you have real work for us ;->
      return;

  QString aCurrentLine;
  QString aCurrentGroup = "";

  QDict<KEntryDict> *pDict;
  if( pWriteBackDict )
	// write back mode - don't mess up the normal dictionary
	pDict = pWriteBackDict;
  else
	// normal mode - use the normal dictionary
	pDict = &(data()->aGroupDict);

  KEntryDict* pCurrentGroupDict = (*pDict)[ "<default>" ];
  
  // reset the stream's device
  rFile.at(0);
  QTextStream aStream( &rFile );
  while( !aStream.eof() )
    {
      aCurrentLine = aStream.readLine();

      // check for a group
      int nLeftBracket = aCurrentLine.find( '[' );
      int nRightBracket = aCurrentLine.find( ']', 1 );
      if( nLeftBracket == 0 && nRightBracket != -1 )
	{
	  // group found; get the group name by taking everything in  
	  // between the brackets  
	  aCurrentGroup = 
	    aCurrentLine.mid( 1, nRightBracket-1 );

	  // check if there already is such a group in the group
	  // dictionary
	  pCurrentGroupDict = (*pDict)[ aCurrentGroup ];
	  if( !pCurrentGroupDict )
	    {
	      // no such group -> create a new entry dictionary
	      KEntryDict* pNewDict = new KEntryDict( 37, false );
	      pNewDict->setAutoDelete( true );
	      (*pDict).insert( aCurrentGroup, pNewDict );

	      // this is now the current group
	      pCurrentGroupDict = pNewDict;
	    }
	  continue;
	};

      if( aCurrentLine[0] == '#' )
	// comment character in the first column, skip the line
	continue;
      
      int nEqualsPos = aCurrentLine.find( '=' );
      if( nEqualsPos == -1 )
		// no equals sign: incorrect or empty line, skip it
		continue;
      
      // insert the key/value line into the current dictionary
	  KEntryDictEntry* pEntry = new KEntryDictEntry;
	  pEntry->aValue = 
	    printableToString(aCurrentLine.right( aCurrentLine.length()-nEqualsPos-1 )
			      ).stripWhiteSpace(); 
	  pEntry->bDirty = false;
	  pEntry->bGlobal = bGlobal;
	  pEntry->bNLS = false;

      pCurrentGroupDict->insert( aCurrentLine.left( nEqualsPos ).stripWhiteSpace(),
								 pEntry );
    }
}


void KConfigBase::setGroup( const QString& pGroup )
{
  if( !pGroup )
    data()->aGroup = "<default>";
  else
    data()->aGroup = pGroup;
}


const QString KConfigBase::group() const
{
  static QString aEmptyStr = "";
  if( data()->aGroup == "<default>" )
    return aEmptyStr;
  else
    return data()->aGroup;
}


const QString KConfigBase::readEntry( const QString& pKey, 
				      const QString& pDefault ) const
{
  if( !data()->bLocaleInitialized && kapp && kapp->localeConstructed() ) 
      {
	  KConfigBase *that = const_cast<KConfigBase*>(this);
	  that->setLocale();
      }
  // const_cast<KConfigBase*>(this)->setLocale();

  QString aValue;
  // retrieve the current group dictionary
  KEntryDict* pCurrentGroupDict = data()->aGroupDict[ data()->aGroup.data() ];
  
  if( pCurrentGroupDict )
    {
      // try the localized key first
      QString aLocalizedKey = QString( pKey );
	  aLocalizedKey += "[";
	  aLocalizedKey += data()->aLocaleString;
	  aLocalizedKey += "]";
      // find the value for the key in the current group

      KEntryDictEntry* pEntryData = (*pCurrentGroupDict)[ aLocalizedKey.data() ];

      if( !pEntryData )
		// next try with the non-localized one
		pEntryData = (*pCurrentGroupDict)[ pKey ];
	  
      if( pEntryData )
	aValue = pEntryData->aValue;
      else if( pDefault )
		{
		  aValue = pDefault;
		}
    }
  else if( pDefault )
	aValue = pDefault;


  // only do dollar expansion if so desired
  if( data()->bExpand ) {
	  // check for environment variables and make necessary translations
	  int nDollarPos = aValue.find( '$' );
	  
	  while( nDollarPos != -1 && nDollarPos+1 < static_cast<int>(aValue.length()))
		  {
			  // there is at least one $
			  if( (aValue)[nDollarPos+1] != '$' )
				  {
					  uint nEndPos = nDollarPos;
					  // the next character is no $
					  do
						  {
							  nEndPos++;
						  } while ( isalnum( (aValue)[nEndPos] ) || 
									nEndPos > aValue.length() );
					  QString aVarName = aValue.mid( nDollarPos+1, 
													 nEndPos-nDollarPos-1 );
					  char* pEnv = getenv( aVarName );
					  if( pEnv )
						  aValue.replace( nDollarPos, nEndPos-nDollarPos, pEnv );
					  else
						  aValue.remove( nDollarPos, nEndPos-nDollarPos );
				  }
			  else {
				  // remove one of the dollar signs
				  aValue.remove( nDollarPos, 1 );
				  nDollarPos++;
			  }
			  nDollarPos = aValue.find( '$', nDollarPos );
		  };
  }

  if( !aValue )
    aValue = pDefault;

  return aValue;
}

int KConfigBase::readListEntry( const QString& pKey, QStrList &list,  
								char sep  ) const
{
  if( !hasKey( pKey ) )
    return 0;
  QString str_list, value;
  str_list = readEntry( pKey );
  if( str_list.isEmpty() )
    return 0; 
  list.clear();
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
  return list.count();
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


unsigned int KConfigBase::readUnsignedNumEntry( const QString& pKey, 
												unsigned int nDefault) const
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


unsigned long KConfigBase::readUnsignedLongNumEntry( const QString& pKey, 
													 unsigned long nDefault) const
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


double KConfigBase::readDoubleNumEntry( const QString& pKey, 
										double nDefault) const
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
	  if( aValue == "true" || aValue == "on" )
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

	  aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, 
													nIndex-nOldIndex-1 ).toUInt() );

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
	 else if (kapp){
           if (chStr=="default")
              if( kapp->localeConstructed() ) 
	          chStr=klocale->charset();
	      else chStr="iso-8859-1"; 
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
  else {
    if( pDefault )
	aRetFont = *pDefault;
  }

  return aRetFont;
}


QRect KConfigBase::readRectEntry( const QString& pKey,
								  const QRect* pDefault ) const
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


const QString KConfigBase::writeEntry( const QString& pKey, const QString& value,
				     bool bPersistent, 
				     bool bGlobal,
				     bool bNLS )
{
  if( !data()->bLocaleInitialized && kapp && kapp->localeConstructed() )
    {
	  KConfigBase *that = (KConfigBase*)this;
	  that->setLocale();
      }
  // const_cast<KConfigBase*>(this)->setLocale();

  QString aValue;

  // retrieve the current group dictionary
  KEntryDict* pCurrentGroupDict = data()->aGroupDict[ data()->aGroup.data() ];

  if( !pCurrentGroupDict )
	{
	  // no such group -> create a new entry dictionary
	  KEntryDict* pNewDict = new KEntryDict( 37, false );
	  pNewDict->setAutoDelete( true );
	  data()->aGroupDict.insert( data()->aGroup.data(), pNewDict );
	  
	  // this is now the current group
	  pCurrentGroupDict = pNewDict;
	}

  // if this is localized entry, add the locale
  QString aLocalizedKey = pKey;
  if( bNLS )
	aLocalizedKey = aLocalizedKey + '[' + data()->aLocaleString + ']';

  // try to retrieve the current entry for this key
  KEntryDictEntry* pEntryData = (*pCurrentGroupDict)[ aLocalizedKey.data() ];
  if( pEntryData )
	{
	  // there already is such a key
	  aValue = pEntryData->aValue; // save old key as return value
	  pEntryData->aValue = value; // set new value
	  pEntryData->bGlobal = bGlobal;
	  pEntryData->bNLS = bNLS;
	  if( bPersistent )
		pEntryData->bDirty = TRUE;
	}
  else
	{
	  // the key currently does not exist
	  KEntryDictEntry* pEntry = new KEntryDictEntry;
	  pEntry->bGlobal = bGlobal;
	  pEntry->bNLS = bNLS;
	  pEntry->aValue = value;
	  if( bPersistent )
		pEntry->bDirty = TRUE;

	  // insert the new entry into group dictionary
	  pCurrentGroupDict->insert( aLocalizedKey, pEntry );
	}


  // the KConfig object is dirty now
  if( bPersistent )
	data()->bDirty = true;
  return aValue;
}


void KConfigBase::writeEntry ( const QString& pKey, QStrList &list, 
			       char sep , bool bPersistent, 
			       bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, QString(""), bPersistent );      
      return;
    }
  QString str_list, value;
  int i;
  for( value = list.first(); !value.isNull() ; value = list.next() )
    {
      for( i = 0; i < (int) value.length(); i++ )
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


const QString KConfigBase::writeEntry( const QString& pKey, int nValue,
									 bool bPersistent, bool bGlobal,
									 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


const QString KConfigBase::writeEntry( const QString& pKey, unsigned int nValue,
									 bool bPersistent, bool bGlobal,
									 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


const QString KConfigBase::writeEntry( const QString& pKey, long nValue,
									 bool bPersistent, bool bGlobal,
									 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


const QString KConfigBase::writeEntry( const QString& pKey, unsigned long nValue,
									 bool bPersistent, bool bGlobal,
									 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


const QString KConfigBase::writeEntry( const QString& pKey, double nValue,
									 bool bPersistent, bool bGlobal,
									 bool bNLS )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


const QString KConfigBase::writeEntry( const QString& pKey, bool bValue,
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


const QString KConfigBase::writeEntry( const QString& pKey, const QFont& rFont, 
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

  if (kapp){
    aCharset=kapp->getCharsets()->name(rFont);
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
  list.insert( 0, tempstr.setNum( rRect.left() ) );
  list.insert( 1, tempstr.setNum( rRect.top() ) );
  list.insert( 2, tempstr.setNum( rRect.width() ) );
  list.insert( 3, tempstr.setNum( rRect.height() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QPoint& rPoint, 
							  bool bPersistent, bool bGlobal,
							  bool bNLS )
{
  QStrList list;
  QString tempstr;
  list.insert( 0, tempstr.setNum( rPoint.x() ) );
  list.insert( 1, tempstr.setNum( rPoint.y() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void KConfigBase::writeEntry( const QString& pKey, const QSize& rSize, 
							  bool bPersistent, bool bGlobal,
							  bool bNLS )
{
  QStrList list;
  QString tempstr;
  list.insert( 0, tempstr.setNum( rSize.width() ) );
  list.insert( 1, tempstr.setNum( rSize.height() ) );

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


void KConfigBase::rollback( bool bDeep )
{
  // clear the global bDirty flag in all cases
  data()->bDirty = false;

  // if bDeep is true, clear the bDirty flags of all the entries
  if( !bDeep )
	return;
  QDictIterator<KEntryDict> aIt( data()->aGroupDict );
  // loop over all the groups
  QString pCurrentGroup;
  while( (pCurrentGroup = aIt.currentKey()) )
	{
	  QDictIterator<KEntryDictEntry> aInnerIt( *aIt.current() );
	  // loop over all the entries
	  KEntryDictEntry* pCurrentEntry;
	  while( (pCurrentEntry = aInnerIt.current()) )
		{
		  pCurrentEntry->bDirty = false;
		  ++aInnerIt;
		}
	  ++aIt;
	}
}


bool KConfigBase::hasKey( const QString& pKey ) const
{
  QString aValue = readEntry( pKey );
  return !aValue.isNull();
}


KEntryIterator* KConfigBase::entryIterator( const QString& pGroup )
{
  // find the group
  KEntryDict* pCurrentGroupDict = data()->aGroupDict[ pGroup ];

  if( !pCurrentGroupDict )
	return 0L; // that group does not exist

  return new KEntryIterator( *pCurrentGroupDict );
}


void KConfigBase::reparseConfiguration()
{
  data()->aGroupDict.clear();

  // setup a group entry for the default group
  KEntryDict* pDefGroup = new KEntryDict( 37, false );
  pDefGroup->setAutoDelete( true );
  data()->aGroupDict.insert( "<default>", pDefGroup );

  // the next three lines are indeed important.
  // no longer (Kalle, 04/10/97)
  //  data()->pAppStream->device()->close();
  //  if (!data()->pAppStream->device()->open( IO_ReadWrite ))
  //    data()->pAppStream->device()->open( IO_ReadOnly );

  parseConfigFiles();
}


void KConfigBase::setDollarExpansion( bool bExpand )
{
	data()->bExpand = bExpand;
}


bool KConfigBase::isDollarExpansion() const
{
	return data()->bExpand;
}

#include "kconfigbase.moc"

