/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)

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

#include "krootprop.h"
#include "kglobal.h"
#include "klocale.h"
#include "kcharsets.h"
#include <qtextstream.h>

KRootProp::KRootProp(const QString& rProp )
{
  atom = 0;
  dirty = FALSE;
  setProp( rProp );
}

KRootProp::~KRootProp()
{
  sync();
  propDict.clear();
}

void KRootProp::sync()
{
  if ( !dirty )
      return;
  if ( !propDict.isEmpty() )
  {
    QMap<QString,QString>::Iterator it = propDict.begin();
    QString propString;
    QString keyvalue;

    while ( it != propDict.end() )
    {
      keyvalue = QString( "%1=%2\n").arg(it.key()).arg(it.data());
      propString += keyvalue;
      ++it;
    }

    XChangeProperty( qt_xdisplay(), qt_xrootwin(), atom,
                    XA_STRING, 8, PropModeReplace,
                    (const unsigned char *)propString.utf8().data(),
                    propString.length());
    kapp->flushX();
  }
}

void KRootProp::setProp( const QString& rProp )
{
  Atom type;
  int format;
  unsigned long nitems;
  unsigned long bytes_after;
  long offset;
  char *buf;
	
  // If a property has already been opened write
  // the dictionary back to the root window
	
  if( atom )
    sync();

  property_ = rProp;
  if( rProp.isEmpty() )
    return;

  atom = XInternAtom( qt_xdisplay(), rProp.utf8(), False);
		
  QString s;
  offset = 0; bytes_after = 1;
  while (bytes_after != 0)
  {
    XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), atom, offset, 256,
                        False, XA_STRING, &type, &format, &nitems, &bytes_after,
                        (unsigned char **)&buf);
    s += QString::fromUtf8(buf);
    offset += nitems/4;
    XFree(buf);
  }
			
  // Parse through the property string stripping out key value pairs
  // and putting them in the dictionary
		
  QString keypair;
  int i=0;
  QString key;
  QString value;
		
  while(s.length() >0 )
  {
    // parse the string for first key-value pair separator '\n'

    i = s.find("\n");
    if(i == -1)
      i = s.length();
		
    // extract the key-values pair and remove from string
			
    keypair = s.left(i);
    s.remove(0,i+1);
			
    // split key and value and add to dictionary
			
    keypair.simplifyWhiteSpace();
			
    i = keypair.find( "=" );
    if( i != -1 )
    {
      key = keypair.left( i );
      value = keypair.mid( i+1 );
      propDict.insert( key, value );
    }
  }
}


QString KRootProp::prop() const
{
    return property_;
}

void KRootProp::destroy()
{
    dirty = FALSE;
    propDict.clear();
    if( atom ) {
	XDeleteProperty( qt_xdisplay(), qt_xrootwin(), atom );
	atom = 0;
    }
}

QString KRootProp::readEntry( const QString& rKey,
			    const QString& pDefault ) const
{
  if( propDict.contains( rKey ) )
      return propDict[ rKey ];
  else
      return pDefault;
}

int KRootProp::readNumEntry( const QString& rKey, int nDefault ) const
{

  QString aValue = readEntry( rKey );
  if( !aValue.isNull() )
  {
    bool ok;

    int rc = aValue.toInt( &ok );
    if (ok)
      return rc;
  }
  return nDefault;
}


QFont KRootProp::readFontEntry( const QString& rKey,
                                const QFont* pDefault ) const
{
  QFont aRetFont;
  QFont aDefFont;

  if (pDefault)
    aDefFont = *pDefault;

  QString aValue = readEntry( rKey );
  if( aValue.isNull() )
    return aDefFont; // Return default font

  // find first part (font family)
  int nIndex = aValue.find( ',' );
  if( nIndex == -1 )
    return aDefFont; // Return default font

  aRetFont.setFamily( aValue.left( nIndex ) );
	
  // find second part (point size)
  int nOldIndex = nIndex;
  nIndex = aValue.find( ',', nOldIndex+1 );
  if( nIndex == -1 )
    return aDefFont; // Return default font
  aRetFont.setPointSize( aValue.mid( nOldIndex+1,
                         nIndex-nOldIndex-1 ).toUInt() );

  // find third part (style hint)
  nOldIndex = nIndex;
  nIndex = aValue.find( ',', nOldIndex+1 );
  if( nIndex == -1 )
    return aDefFont; // Return default font
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
  else if (kapp) {
      if (chStr == "default")
	  if (KGlobal::locale())
	      chStr = KGlobal::locale()->charset();
	  else chStr = "iso-8859-1";
      KGlobal::charsets()->setQFont(aRetFont,chStr);
  }
  // find fifth part (weight)
  nOldIndex = nIndex;
  nIndex = aValue.find( ',', nOldIndex+1 );
  if( nIndex == -1 )
    return aDefFont; // Return default font

  aRetFont.setWeight( aValue.mid( nOldIndex+1,
                      		  nIndex-nOldIndex-1 ).toUInt() );
  // find sixth part (font bits)
  uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();
  if( nFontBits & 0x01 )
    aRetFont.setItalic( true );
  if( nFontBits & 0x02 )
    aRetFont.setUnderline( true );
  if( nFontBits & 0x04 )
    aRetFont.setStrikeOut( true );
  if( nFontBits & 0x08 )
    aRetFont.setFixedPitch( true );
  if( nFontBits & 0x20 )
    aRetFont.setRawMode( true );

  return aRetFont;
}


QColor KRootProp::readColorEntry( const QString& rKey,
								const QColor* pDefault ) const
{
  QColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  if( pDefault )
    aRetColor = *pDefault;

  QString aValue = readEntry( rKey );
  if( aValue.isNull() )
    return aRetColor;

  // Support #ffffff style colour naming.
  // Help ease transistion from legacy KDE setups
  if( aValue.find("#") == 0 ) {
    aRetColor.setNamedColor( aValue );
    return aRetColor;
  }
		
  // Parse "red,green,blue"
  // find first comma
  int nIndex1 = aValue.find( ',' );
  if( nIndex1 == -1 )
    return aRetColor;
  // find second comma
  int nIndex2 = aValue.find( ',', nIndex1+1 );
  if( nIndex2 == -1 )
    return aRetColor;

  bool bOK;
  nRed = aValue.left( nIndex1 ).toInt( &bOK );
  nGreen = aValue.mid( nIndex1+1,
                       nIndex2-nIndex1-1 ).toInt( &bOK );
  nBlue = aValue.mid( nIndex2+1 ).toInt( &bOK );

  aRetColor.setRgb( nRed, nGreen, nBlue );

  return aRetColor;
}

QString KRootProp::writeEntry( const QString& rKey, const QString& rValue )
{
    dirty = TRUE;
    if ( propDict.contains( rKey ) ) {
	QString aValue = propDict[ rKey ];
	propDict.replace( rKey, rValue );
	return aValue;
    }
    else {
	propDict.insert( rKey, rValue );
	return QString::null;
    }
}

QString KRootProp::writeEntry( const QString& rKey, int nValue )
{
  QString aValue;

  aValue.setNum( nValue );

  return writeEntry( rKey, aValue );
}

QString KRootProp::writeEntry( const QString& rKey, const QFont& rFont )
{
  QString aValue;
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

  QString aCharset = "default";
  if( rFont.charSet() != QFont::AnyCharSet )
      aCharset.setNum( rFont.charSet() );
  
  QTextIStream ts( &aValue );
  ts << rFont.family() << "," << rFont.pointSize() << "," 
     << rFont.styleHint() << "," << aCharset << "," << rFont.weight() << "," 
     << nFontBits;
  return writeEntry( rKey, aValue );
}

QString KRootProp::writeEntry( const QString& rKey, const QColor& rColor )
{
  QString aValue = QString( "%1,%2,%3").arg(rColor.red()).arg(rColor.green()).arg(rColor.blue() );

  return writeEntry( rKey, aValue );
}

QString KRootProp::removeEntry(const QString& rKey)
{
    if (propDict.contains(rKey)) {
	dirty = TRUE;
	QString aValue = propDict[rKey];
	propDict.remove(rKey);
	return aValue;
    } else
	return QString::null;
}
  
QStringList KRootProp::listEntries() const
{
    QMap<QString,QString>::ConstIterator it;
    QStringList list;

    for (it=propDict.begin(); it!=propDict.end(); it++)
	list += it.key();
	
    return list;
}
