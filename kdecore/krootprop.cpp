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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qwidget.h>

#include "config.h"
#ifdef Q_WS_X11 // not needed anyway :-)

#include "krootprop.h"
#include "kglobal.h"
#include "klocale.h"
#include "kcharsets.h"
#include "kapplication.h"
#include <qtextstream.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

KRootProp::KRootProp(const QString& rProp )
{
  atom = 0;
  dirty = false;
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

  QString propString;
  if ( !propDict.isEmpty() )
  {
    QMap<QString,QString>::Iterator it = propDict.begin();
    QString keyvalue;

    while ( it != propDict.end() )
    {
      keyvalue = QString( "%1=%2\n").arg(it.key()).arg(it.data());
      propString += keyvalue;
      ++it;
    }
  }

  XChangeProperty( qt_xdisplay(), qt_xrootwin(), atom,
                  XA_STRING, 8, PropModeReplace,
                  (const unsigned char *)propString.utf8().data(),
                  propString.length());
  XFlush( qt_xdisplay() );
}

void KRootProp::setProp( const QString& rProp )
{
  Atom type;
  int format;
  unsigned long nitems;
  unsigned long bytes_after;
  long offset;

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
    unsigned char *buf = 0; 
    if (XGetWindowProperty( qt_xdisplay(), qt_xrootwin(), atom, offset, 256,
                        False, XA_STRING, &type, &format, &nitems, &bytes_after,
                        &buf) == Success && buf) 
    {
      s += QString::fromUtf8((const char*)buf);
      offset += nitems/4;
      XFree(buf);
    }
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
    dirty = false;
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

  if ( !aRetFont.fromString( aValue ) && pDefault )
    aRetFont = aDefFont;

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

  // Support #ffffff style color naming.
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
    dirty = true;
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
  return writeEntry( rKey, rFont.toString() );
}

QString KRootProp::writeEntry( const QString& rKey, const QColor& rColor )
{
  QString aValue = QString( "%1,%2,%3").arg(rColor.red()).arg(rColor.green()).arg(rColor.blue() );

  return writeEntry( rKey, aValue );
}

QString KRootProp::removeEntry(const QString& rKey)
{
    if (propDict.contains(rKey)) {
	dirty = true;
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

	QMap<QString,QString>::ConstIterator end(propDict.end());
    for (it=propDict.begin(); it!=end; ++it)
	list += it.key();

    return list;
}
#endif
