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
// Revision 1.21  1999/04/18 09:15:09  kulow
// taking out config.h from Header files. I don't know if I haven't noticed
// before, but this is even very dangerous
//
// Revision 1.20  1999/04/10 23:19:08  torben
// Torben: Added QStringList since it supports unicode. Unfortunately
// KConfig does not do that right now ...
//
// Revision 1.19  1999/04/08 01:39:44  torben
// Torben: Corrected API and docu
//
// Revision 1.18  1999/04/04 16:14:17  dfaure
// Small fix for non-existent values in config file.
// Testing if (!aValue) when aValue is a QString doesn't make much sense...
//
// Revision 1.17  1999/03/09 23:38:17  kulow
// moved the QString wrapper to the header file as inline function
//
// Revision 1.16  1999/03/08 16:05:17  rbeutler
// added writeEntry( const QString &, const char *, ... ) method to fix the problem with the implicitely conversion to bool
//
// Revision 1.15  1999/03/01 23:33:21  kulow
// CVS_SILENT ported to Qt 2.0
//
// Revision 1.14.2.2  1999/02/14 02:05:44  granroth
// Converted a lot of 'const char*' to 'QString'.  This compiles... but
// it's entirely possible that nothing will run linked to it :-P
//
// Revision 1.14.2.1  1999/01/30 20:18:56  kulow
// start porting to Qt2.0 beta
//
// Revision 1.14  1998/11/22 21:52:33  garbanzo
// Fixed some comments that referred to sync() as Sync(), and a few other case issues.
//
// Revision 1.13  1998/10/20 18:58:12  ettrich
// ugly hack to get rid of X11 includes, small fix
//
// Revision 1.12  1998/10/07 06:49:23  kalle
// Correctly read double dollar signs (patch by Harri Porten)
// Dollar expansion can be turned off with setDollarExpansion( false ).
// \sa isDollarExpansion
//
// Revision 1.11  1998/08/22 20:02:37  kulow
// make kdecore have nicer output, when compiled with -Weffc++ :)
//
// Revision 1.10  1998/03/29 19:07:30  kalle
// Methods for reading and writing bool, unsigned int, long, unsigned long,
// double, QRect, QSize, QPoint
//
// Revision 1.9  1998/01/18 14:38:51  kulow
// reverted the changes, Jacek commited.
// Only the RCS comments were affected, but to keep them consistent, I
// thought, it's better to revert them.
// I checked twice, that only comments are affected ;)
//
// Revision 1.7  1998/01/09 08:06:47  kalle
// KConfigGroupSaver
//
// Revision 1.6  1997/12/12 14:46:03  denis
// Reverting to lower case true/false
//
// Revision 1.5  1997/12/12 14:33:46  denis
// replaced true by TRUE and false by FALSE
//
// Revision 1.4  1997/10/21 20:44:45  kulow
// removed all NULLs and replaced it with 0L or "".
// There are some left in mediatool, but this is not C++
//
// Revision 1.3  1997/10/16 11:14:31  torben
// Kalle: Copyright headers
// kdoctoolbar removed
//
// Revision 1.2  1997/10/05 02:31:11  jones
// MRJ: Changed const QString& readEntry( ... ) to QString readEntry( ... )
// I had to do this - almost everything was broken.
//
// Revision 1.1  1997/10/04 19:51:04  kalle
// new KConfig
//

#ifndef _KCONFIGBASE_H
#define _KCONFIGBASE_H

// Qt includes
#include <qcolor.h>
#include <qfont.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qproperty.h>

// KDE includes
#include <kconfigdata.h>

/**
* Abstract base class for KDE configuration entries
*
*	This class forms the base for all KDE configuration. It is an
* 	abstract base class, meaning that you cannot directly instantiate
* 	objects of this class. Either use KConfig (for usual KDE
* 	configuration) or KSimpleConfig (for special needs like ksamba).
*	one application, independent of the configuration files they came
*	from.
*
*	All configuration entries are of the form "key=value" and
*	belong to a certain group. A group can be specified in a
*	configuration file with "[GroupName]". All configuration entries
*	from the beginning of a configuration file to the first group
*	declaration belong to a special group called the default group.
*
*	If there is a $ character in a entry, KConfig tries to expand
*	environment variable and uses its value instead of its name. You
*	can avoid this feature by having two consecutive $ characters in
*	your config file which get expanded to one.
*
*   Lines starting with a hash mark(#) are comment lines.
*
* @author Kalle Dalheimer (kalle@kde.org)
* @version $Id$
* @see KApplication::getConfig KConfig KSimpleConfig
* @short KDE Configuration Management abstract base class
*/

#include <qfile.h>
#include <qobject.h>
#include "kdebug.h"

class KConfigBaseData;

class KConfigBase : public QObject
{
  Q_OBJECT

private:
  KConfigBaseData* pData;

  // copy-construction and assignment are not allowed
  KConfigBase( const KConfigBase& );
  KConfigBase& operator= ( const KConfigBase& rConfig );

protected:
  /**
   * Access to the configuration data.
   *
   * @return a pointer to the configuration base data
   */
  KConfigBaseData* data() const { return pData; }

  /**
   * Read the locale and put in the configuration data struct.
   * Note: This should be done in the constructor, but this is not
   * possible due to some mutual dependencies in KApplication::init()
   */
  void setLocale();

  /**
   * Parse all configuration files for a configuration object.
   *
   * This method must be reimplemented by the derived classes. It
   *  should go through the list of appropriate files for a
   * configuration object, open the files and call
   * parseOneConfigFile() for each one of them.
   */
  virtual void parseConfigFiles() = 0;

  /**
   * Parse one configuration file.
   *
   * This method contains the actual configuration file parser. It
   * can overridden by derived classes for specific parsing
   * needs. For normal use, this should not be necessary.
   *	
   * @param rFile The configuration file to parse
   * @param pGroup
   * @param bGlobal
   */
  virtual void parseOneConfigFile( QFile& rFile,
				   KGroupDict* pGroup = 0L,
				   bool bGlobal = false );
  
  /**
   * Write configuration file back.
   *
   * This method must be reimplemented by derived classes. It should
   * dump the data of the configuration object to the appropriate
   * files.
   *
   * @param rFile The file to write
   * @param bGlobal Should the data be saved to a global file
   * @return Whether some entries are left to be written to other
   *  files.
   */
  virtual bool writeConfigFile( QFile& rFile, bool bGlobal = false ) = 0;

public:
  /**
   * Construct a KConfigBase object.
   */
  KConfigBase();

  /**
   * Destructor.
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KConfigBase();

  /**
   * Specify the group in which keys will be searched.
   *
   * Switch back to the default group by passing an empty string.
   *  @param pGroup The name of the new group.
   */
  void setGroup( const QString& pGroup );

  /**
   * Retrieve the group where keys are currently searched in.
   *
   * @return The current group
   */
  QString group() const;

  /**
   * Retrieve the group where keys are currently searched in. Note:
   * this method is deprecated; use KConfigBase::group() instead.
   *
   * @return The current group
   */
  QString getGroup() const { debug( "KConfigBase::getGroup() is deprecated, use KConfigBase::group() instead" );
  return group(); }

  /**
   * Read the value of an entry specified by rKey in the current group
   *
   * @param pKey	The key to search for.
   * @param pDefault A default value returned if the key was not found.
   * @return The value for this key or a null string if no value
   *	  was found.
   */
  QString readEntry( const QString& pKey,
		     const QString& pDefault = QString::null ) const;

  /**
   * Read the value of an entry specified by rKey in the current group.
   * The value is treated to be of the given type.
   *
   * @return an empty property on error.
   */
  QProperty readPropertyEntry( const QString& rKey, QProperty::Type ) const;
  
  /**
   * Read a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  int readListEntry( const QString& pKey, QStrList &list,
		     char sep = ',' ) const;

  /**
   * Read a list of strings.
   *
   * @param pKey The key to search for
   * @param sep  The list separator (default ",")
   * @return The list.
   */
  QStringList readListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  int readNumEntry( const QString& pKey, int nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  unsigned int readUnsignedNumEntry( const QString& pKey,
				     unsigned int nDefault = 0 ) const;
  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  long readLongNumEntry( const QString& pKey, long nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  unsigned long readUnsignedLongNumEntry( const QString& pKey,
					  unsigned long nDefault = 0 ) const;

  /**
   * Read a numerical value.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found.
   * @return The value for this key or 0 if no value was found.
   */
  double readDoubleNumEntry( const QString& pKey, double nDefault = 0.0 ) const;

  /**
   * Read a QFont.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey		The key to search for.
   * @param pDefault	A default value returned if the key was not found.
   * @return The value for this key or a default font if no value was found.
   */
  QFont readFontEntry( const QString& pKey,
		       const QFont* pDefault = 0L ) const;

  /**
   * Read a boolean entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a boolean value. Currently "on" and "true" are
   * accepted as true, everything else if false.
   *
   * @param pKey		The key to search for
   * @param bDefault    A default value returned if the key was not
   * 					found.
   * @return The value for this key or a default value if no value was
   * found.
   */
  bool readBoolEntry( const QString& pKey, const bool bDefault = false ) const;


  /**
   * Read a rect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey		The key to search for
   * @param pDefault	A default value returned if the key was not
   *	 				found.
   * @return The value for this key or a default rectangle if no value
   * was found.
   */
  QRect readRectEntry( const QString& pKey, const QRect* pDefault = 0L ) const;


  /**
   * Read a point entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey		The key to search for
   * @param pDefault	A default value returned if the key was not
   *	 				found.
   * @return The value for this key or a default point if no value
   * was found.
   */
  QPoint readPointEntry( const QString& pKey, const QPoint* pDefault = 0L ) const;


  /**
   * Read a size entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey		The key to search for
   * @param pDefault	A default value returned if the key was not
   *	 				found.
   * @return The value for this key or a default point if no value
   * was found.
   */
  QSize readSizeEntry( const QString& pKey, const QSize* pDefault = 0L ) const;


  /**
   * Read a QColor.
   *
   * Read the value of an entry specified by rKey in the current group
   * and interpret it as a color.
   *
   * @param pKey		The key to search for.
   * @param pDefault	A default value returned if the key was not found.
   * @return The value for this key or a default color if no value
   * was found.
   */
  QColor readColorEntry( const QString& pKey,
			 const QColor* pDefault = 0L ) const;

  /**
   * Write the key/value pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey		The key to write.
   *  @param pValue	The value to write.
   *  @param bPersistent	If bPersistent is false, the entry's dirty
   *			flag will not be set and thus the entry will
   *			not be written to disk at deletion time.
   *  @param bGlobal	If bGlobal is true, the pair is not saved to the
   *   application specific config file, but to the global ~/.kderc
   *  @param bNLS	If bNLS is true, the locale tag is added to the key
   *   when writing it back.
   *  @return The old value for this key. If this key did not
   *   exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, const QString& pValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * writeEntry() overriden to accept a property.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The property to write
   * @param bPersistent	If bPersistent is false, the entry's dirty flag
   *			will not be set and thus the entry will not be
   *			written to disk at deletion time.
   * @param bGlobal If bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the global ~/.kderc
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry( const QString& rKey, const QProperty& rValue,
		    bool bPersistent = true, bool bGlobal = false,
		    bool bNLS = false );
  
  /**
   * writeEntry() overriden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The list to write
   * @param bPersistent	If bPersistent is false, the entry's dirty flag
   *			will not be set and thus the entry will not be
   *			written to disk at deletion time.
   * @param bGlobal If bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the global ~/.kderc
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry ( const QString& pKey, const QStrList &rValue, char sep = ',',
		    bool bPersistent = true, bool bGlobal = false,
		    bool bNLS = false );

  /**
   * writeEntry() overriden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param rKey The key to write
   * @param rValue The list to write
   * @param bPersistent	If bPersistent is false, the entry's dirty flag
   *			will not be set and thus the entry will not be
   *			written to disk at deletion time.
   * @param bGlobal If bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the global ~/.kderc
   * @param bNLS If bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see #writeEntry
   */
  void writeEntry ( const QString& pKey, const QStringList &rValue, char sep = ',',
		    bool bPersistent = true, bool bGlobal = false,
		    bool bNLS = false );
  
  /**
   * Write the key/value pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey		The key to write.
   *  @param pValue	The value to write.
   *  @param bPersistent	If bPersistent is false, the entry's dirty
   *			flag will not be set and thus the entry will
   *			not be written to disk at deletion time.
   *  @param bGlobal	If bGlobal is true, the pair is not saved to the
   *   application specific config file, but to the global ~/.kderc
   *  @param bNLS	If bNLS is true, the locale tag is added to the key
   *   when writing it back.
   *  @return The old value for this key. If this key did not
   *   exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, const char *pValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );
  
  /**
   * Write the key value pair.
   * Same as above, but write a numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, int nValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );
  
  /**
   * Write the key value pair.
   * Same as above, but write an unsigned numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, unsigned int nValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a long numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, long nValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write an unsigned long numerical value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, unsigned long nValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, double nValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a boolean value.
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, bool bValue,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a font
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   * exist, a null string is returned.	
   */
  QString writeEntry( const QString& pKey, const QFont& rFont,
		      bool bPersistent = true, bool bGlobal = false,
		      bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a color
   * @param pKey The key to write.
   * @param rValue The color value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.	
   */
  void writeEntry( const QString& pKey, const QColor& rColor,
		   bool bPersistent = true, bool bGlobal = false,
		   bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a rectangle
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.	
   */
  void writeEntry( const QString& pKey, const QRect& rColor,
		   bool bPersistent = true, bool bGlobal = false,
		   bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a point
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.	
   */
  void writeEntry( const QString& pKey, const QPoint& rColor,
		   bool bPersistent = true, bool bGlobal = false,
		   bool bNLS = false );

  /**
   * Write the key value pair.
   * Same as above, but write a size
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal	If bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global ~/.kderc
   * @param bNLS	If bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @return The old value for this key. If this key did not
   *  exist, a null string is returned.	
   */
  void writeEntry( const QString& pKey, const QSize& rColor,
		   bool bPersistent = true, bool bGlobal = false,
		   bool bNLS = false );

  /** Turns on or off "dollar expansion" when reading config entries.
   *	@param bExpand if true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool bExpand = true );
  
  /** Returns whether dollar expansion is on or off.
   *	@return true if dollar expansion is on.
   */
  bool isDollarExpansion() const;
  
  /**
   * Don't write dirty entries at destruction time. If bDeep is
   * false, only the global dirty flag of the KConfig object gets
   * cleared. If you then call writeEntry again, the global dirty flag
   * is set again and all dirty entries will be written.
   * @param bDeep if
   * true, the dirty flags of all entries are cleared, as well as the
   * global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flush the entry cache. Write back dirty configuration entries to
   * the most specific file. This is called automatically from the
   * destructor.
   * This method must be implemented by the derived classes.
   */
  virtual void sync() = 0;

  /**
   * Check if the key has an entry in the specified group
   * Use this to determine if a key is not specified for the current
   * group (HasKey returns false) or is specified but has no value
   * ("key =EOL"; Has Key returns true, ReadEntry returns an empty
   * string)
   * @param pKey The key to search for.
   * @return if true, the key is available
   */
  bool hasKey( const QString& pKey ) const;

  /**
   * Returns an iterator on the list of groups
   * @return The group iterator. The caller is reponsable for
   * deleting the iterator after using it.
   */
  KGroupIterator* groupIterator(void) { return pData->groupIterator(); }

  /**
   * Returns an iterator on the entries in the current group
   *
   * @param pGroup the group to provide an iterator for
   * @return The iterator for the group or 0, if the group does not
   * exist. The caller is responsible for deleting the iterator after
   * using it.
   */
  KEntryIterator* entryIterator( const QString& pGroup );

  /**
   * Reparses all configuration files. This is useful for programms
   * which use standalone graphical configuration tools.
   */
  virtual void reparseConfiguration();
};


/**
  * Helper class to facilitate working with KConfig/KSimpleConfig groups
  *
  * Careful programmers always set the group of a
  * KConfig/KSimpleConfig object to the group they want to read from
  * and set it back to the old one of afterwards. This is usually
  * written as
  *
  * QString oldgroup config->group();
  * config->setGroup( "TheGroupThatIWant" );
  * ...
  * config->writeEntry( "Blah", "Blubb" );
  *
  * config->setGroup( oldgroup );
  *
  * In order to facilitate this task, you can use
  * KConfigGroupSaver. Simply construct such an object ON THE STACK
  * when you want to switch to a new group. Then, when the object goes
  * out of scope, the group will automatically be restored. If you
  * want to use several different groups within a function or method,
  * you can still use KConfigGroupSaver: Simply enclose all work with
  * one group (including the creation of the KConfigGroupSaver object)
  * in one block.
  *
  * @author Kalle Dalheimer <kalle@kde.org>
  * @version $Id$
  * @see KConfigBase, KConfig, KSimpleConfig
  * @short helper class for easier use of KConfig/KSimpleConfig groups
  */

class KConfigGroupSaver
{
public:
  /**
   * Constructor. You pass a pointer to the KConfig/KSimpleConfig
   * object you want to work with and a string indicating the _new_
   * group.
   * @param config the KConfig/KSimpleConfig object this
   * KConfigGroupSaver works on
   * @param group the new group that the KConfig/KSimpleConfig object
   * should switch to
   */
  KConfigGroupSaver( KConfigBase* config, QString group )
      : _config(config), _oldgroup(config->group())
	{
	  //	  _config = config;
	  //	  _oldgroup = _config->group();
	  _config->setGroup( group );
	};

  ~KConfigGroupSaver()
	{
	  _config->setGroup( _oldgroup );
	}

private:
  KConfigBase* _config;
  QString _oldgroup;

  KConfigGroupSaver(const KConfigGroupSaver&);
  KConfigGroupSaver& operator=(const KConfigGroupSaver&);
};

inline QString KConfigBase::writeEntry( const QString &pKey, 
                                        const char *pValue,
                                        bool bPersistent, bool bGlobal, 
					bool bNLS ) {
      return writeEntry( pKey, QString(pValue), bPersistent, bGlobal, bNLS );
}

#endif
