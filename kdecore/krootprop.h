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
#ifndef _KROOTPROP_H
#define _KROOTPROP_H_

#include <kapp.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <qdict.h>

/** 
* Access to KDE desktop resources stored on the root window.
*
* A companion to the KConfig class
*
* The KRootProp class is used for reading and writing configuration entries
* to properties on the root window.
*
* All configuration entries are of the form "key=value".
*
* @see KConfig::KConfig
* @author Mark Donohoe (donohe@kde.org)
* @version $Id$
*/
class KRootProp
{
private:	
  Display *kde_display;
  Window root;
  int screen;
  Atom at;
  QDict <QString> propDict;

protected:

public:
/** 
* Construct a KRootProp object. 
*
*/
   KRootProp();
   
/** 
* Destructor. 
*
* Writes back any dirty configuration entries.
*/
  ~KRootProp();

/** 
* Specify the property in which keys will be searched.
*
*/	
  void setProp(const QString& rProp="");

/**
* Read the value of an entry specified by rKey in the current property
*
* @param rKey	The key to search for.
* @param pDefault A default value returned if the key was not found.
* @return The value for this key or an empty string if no value
*	  was found.
*/	
  QString readEntry( const QString& rKey, 
  	  	  	  	  	  const QString& pDefault = QString::null ) const ;
					  
/**
* Read a numerical value. 
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it numerically.
*
* @param rKey The key to search for.
* @param nDefault A default value returned if the key was not found.
* @return The value for this key or 0 if no value was found.
*/
  int readNumEntry( const QString& rKey, int nDefault = 0 ) const;
  
/** 
* Read a QFont.
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it as a font object.
*
* @param rKey		The key to search for.
* @param pDefault	A default value returned if the key was not found.
* @return The value for this key or a default font if no value was found.
*/ 
  QFont readFontEntry( const QString& rKey, 
							  const QFont* pDefault = 0 ) const;

/** 
* Read a QColor.
*
* Read the value of an entry specified by rKey in the current property 
* and interpret it as a color.
*
* @param rKey		The key to search for.
* @param pDefault	A default value returned if the key was not found.
* @return The value for this key or a default color if no value
* was found.
*/					  
  QColor readColorEntry( const QString& rKey,
								const QColor* pDefault = 0 ) const;
							  
	
/** 
* writeEntry() overridden to accept a const QString& argument.
*
* This is stored to the current property when destroying the
* config object or when calling Sync().
*
* @param rKey		The key to write.
* @param rValue		The value to write.
* @return The old value for this key. If this key did not exist, 
*	  a null string is returned.	  
*
* @see #writeEntry
*/				
  QString writeEntry( const QString& rKey, const QString& rValue );
  
/** Write the key value pair.
* Same as above, but write a numerical value.
* @param rKey The key to write.
* @param nValue The value to write.
* @return The old value for this key. If this key did not
* exist, a null string is returned.	  
*/
  QString writeEntry( const QString& rKey, int nValue );
  
/** Write the key value pair.
* Same as above, but write a font
* @param rKey The key to write.
* @param rValue The value to write.
* @return The old value for this key. If this key did not
* exist, a null string is returned.	  
*/
  QString writeEntry( const QString& rKey, const QFont& rFont );
  
/** Write the key value pair.
* Same as above, but write a color
* @param rKey The key to write.
* @param rValue The value to write.
* @return The old value for this key. If this key did not
*  exist, a null string is returned.	  
*/
  QString writeEntry( const QString& rKey, const QColor& rColor );

/** Flush the entry cache.
* Write back dirty configuration entries to the current property,
*  This is called automatically from the destructor.
*/	
	void sync();
};

#endif
