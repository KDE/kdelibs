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
#define _KROOTPROP_H

typedef unsigned long Atom;

#include <qcolor.h>
#include <qfont.h>
#include <qmap.h>
#include <qstringlist.h>

class KRootPropPrivate;

/**
* Access KDE desktop resources stored on the root window.
*
* A companion to the @ref KConfig class.
*
* The KRootProp class is used for reading and writing configuration entries
* to properties on the root window.
*
* All configuration entries are of the form "key=value".
*
* @see  KConfig::KConfig
* @author Mark Donohoe (donohe@kde.org)
* @version $Id$
*/
class KRootProp
{
private:	
  Atom atom;
  QMap<QString,QString> propDict;
  QString property_;
  bool dirty;
  KRootPropPrivate *d;

public:
  /**
   * Constructs a KRootProp object for the property @p rProp.
   * @param rProp the property that will be searched, null to
   *              do nothing
   * @see setProp()
   **/
   KRootProp( const QString& rProp = QString::null );
  /**
   * Destructs the KRootProp object.
   *
   * Writes back any dirty configuration entries.
   **/
  ~KRootProp();
   
  /**
   * Sets the property in which keys will be searched.
   * @param rProp the property that will be searched
   **/	
   void setProp(const QString& rProp="");
   /**
    * Returns the name of the property under which keys are searched.
    * @return the property that will be searched
    **/
   QString prop() const;
   
   /**
    * Destroys the property completely.
    *
    * I.e. all entries will be cleared
    * and the property will be removed from the root window.
    **/
   void destroy();

 /**
  * Reads the value of an entry specified by @p rKey in the current property.
  *
  * @param rKey	The key to search for.
  * @param pDefault A default value returned if the key was not found.
  * @return The value for this key or the default if no value
  *	  was found.
  **/	
 QString readEntry( const QString& rKey,
		    const QString& pDefault = QString::null ) const ;
					
 /**
  * Reads a numerical value.
  *
  * Reads the value of an entry specified by @p rKey in the current property
  * and interprets it numerically.
  *
  * @param rKey The key to search for.
  * @param nDefault A default value returned if the key was not found.
  * @return The value for this key or the default if no value was found.
  */
 int readNumEntry( const QString& rKey, int nDefault = 0 ) const;
 
 /**
  * Reads a @ref QFont value.
  *
  * Reads the value of an entry specified by @p rKey in the current property
  * and interpret it as a font object.
  *
  * @param rKey		The key to search for.
  * @param pDefault	A default value returned if the key was not found.
  * @return The value for this key or a default font if no value was found.
  */
 QFont readFontEntry( const QString& rKey,
		      const QFont* pDefault = 0 ) const;
 
 /**
  * Reads a @ref QColor.
  *
  * Reads the value of an entry specified by @p rKey in the current property
  * and interprets it as a color.
  *
  * @param rKey		The key to search for.
  * @param pDefault	A default value returned if the key was not found.
  * @return The value for this key or a default color if no value
  * was found.
  */					
 QColor readColorEntry( const QString& rKey,
			const QColor* pDefault = 0 ) const;
							
	
 /**
  * Writes a (key/value) pair.
  *
  * This is stored to the current property when destroying the
  * config object or when calling @ref sync().
  *
  * @param rKey		The key to write.
  * @param rValue		The value to write.
  * @return The old value for this key. If this key did not exist,
  *	  a null string is returned.	
  *
  **/				
 QString writeEntry( const QString& rKey, const QString& rValue );

 /**
  * Writes the (key/value) pair.
  * Same as above, but writes a numerical value.
  * @param rKey The key to write.
  * @param nValue The value to write.
  * @return The old value for this key. If this key did not
  * exist, a null string is returned.	
  **/
 QString writeEntry( const QString& rKey, int nValue );

 /**
  * Writes the (key/value) pair.
  * Same as above, but writes a font.
  * @param rKey The key to write.
  * @param rValue The value to write.
  * @return The old value for this key. If this key did not
  * exist, a null string is returned.	
  **/
  QString writeEntry( const QString& rKey, const QFont& rFont );
 
  /**
   * Writes the (key/value) pair.
   * Same as above, but writes a color.
   * @param rKey The key to write.
   * @param rValue The value to write.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.	
   **/
  QString writeEntry( const QString& rKey, const QColor& rColor );
  
  /**
   * Removes an entry.
   * @param rKey The key to remove.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.
   **/
  QString removeEntry(const QString& rKey);

  /**
   * Returns a list of all keys.
   * @return A @ref QStringList containing all the keys.
   **/
  QStringList listEntries() const;

  /**
   * Flushes the entry cache.
   * Writes back dirty configuration entries to the current property,
   * This is called automatically from the destructor.
   **/	
  void sync();
};

#endif
