// $Id$
//
// $Log$
// Revision 1.2  1997/04/15 20:01:54  kalle
// Kalles changes for 0.8
//
// Revision 1.1.1.1  1997/04/13 14:42:41  cvsuser
// Source imported
//
// Revision 1.1.1.1  1997/04/09 00:28:07  cvsuser
// Sources imported
//
// Revision 1.12  1997/03/16 22:47:35  kalle
// KStream nicht mehr included
//
// Revision 1.11  1997/03/09 17:29:05  kalle
// KTextStream -> QTextStream
//
// Revision 1.10  1996/12/14 12:58:44  kalle
// read and write numerical config values
//
// Revision 1.9  1996/12/14 12:49:40  kalle
// method names start with a small letter
//
// Revision 1.8  1996/12/01 10:31:01  kalle
// bPersistent parameter for WriteEntry()
//
// Revision 1.7  1996/11/23 21:17:54  kalle
// KTextStream instead of QTextStream
//
// Revision 1.6  1996/11/20 20:17:05  kalle
// Write access:
//
// One additional parameter for ParseOneConfigFile (parsing into temp structures)
// new: WriteConfigFile
//
// Revision 1.5  1996/11/17 09:50:34  kalle
// DOC++ documentation
// WriteEntry() and Sync() added
//
// Revision 1.4  1996/11/15 18:12:36  kalle
// SetGroup returns void
//
// Revision 1.3  1996/11/14 21:34:44  kalle
// include config.h instead of localconfig.h
//
// Revision 1.2  1996/11/14 21:04:27  kalle
// include localconfig.h
//
// Revision 1.1  1996/11/10 18:31:18  kalle
// Initial revision
//
// Revision 1.5  1996/11/03 21:02:47  kalle
// Change to chaed config entries
//
// Revision 1.4  1996/10/26 18:33:30  kalle
// Explanation of constructor more precise
//
// Revision 1.3  1996/10/26 08:34:43  kalle
// pStream parameter in constructor defaults to NULL
//
// Revision 1.2  1996/10/25 18:35:09  kalle
// add: FindEntry
//
// Revision 1.1  1996/10/25 17:16:27  kalle
// Initial revision
//
// Revision 1.1  1996/10/20 19:55:31  kalle
// Initial revision
//

// KConfig: configuration options for the Kool Desktop Environment
// 
// KConfig.h
//
// (C) 1996 by Matthias Kalle Dalheimer

#ifndef _KCONFIG_H
#define _KCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qcolor.h>   // QColor
#include <qfile.h>    // QFile
#include <qfont.h>    // QFont
#include <qstring.h>  // QString
#include <qtstream.h> // QTextStream

#ifndef _KCONFIGDATA_H
#include <Kconfigdata.h>
#endif

class KConfigData;

/// KDE configuration entries.
/** The KConfig class encapsulates all the configuration entries for
  one application, independent of the configuration files they came
  from. All configuration entries are of the form "key=value" and
  belong to a certain group. A group can be specified in a
  configuration file with "[GroupName]". All configuration entries
  from the beginning of a configuration file to the first group
  declaration belong to a special group called the default group.
  If there is a $ character in a entry, KConfig tries to expand
  environment variable and uses its value instead of its name.
  */
class KConfig
{
private:
  KConfigData* pData;

  // copy-construction and assignment are not allowed
  KConfig( const KConfig& );
  KConfig& operator= ( const KConfig& rConfig );

  void parseConfigFiles();
  void parseOneConfigFile( QTextStream*, KGroupDict* pGroup = NULL );
  void writeConfigFile( QFile& );

protected:

public:
  /// Construct a KConfig object.
  /** Construct a KConfig object. Files searched will be all
	standard config files and the file given by pStream (if pStream
    != NULL ). The QFile associated with QTextStream already must be 
    opened.
	*/
  KConfig( QTextStream* pStream = NULL );

  /// Destroy the KConfig object and write back dirty config entries.
  /** Destructor. Writes back any dirty configuration entries.
   */
  ~KConfig();

  /// Set the group in which to search for keys.
  /** Specify the group in which keys will be searched
	Switch back to the default group by passing an empty string.
   */
  void setGroup( const QString& rGroup );

  /// Retrieve the current group.
  /** Retrieve the group where keys are currently searched in.
   */
  const QString& getGroup() const;

  /// Read the value associated with rKey
  /** Read the value of an entry specified by rKey in the current
    group
   */
  QString readEntry( const QString& rKey ) const;

  /// Read a numerical value
  /** Read the value of an entry specified by rKey in the current group
	and interpret it numerically.
	*/
  int readNumEntry( const QString& rKey ) const;

  /// Read a QFont
  /** Read the value of an entry specified by rKey in the current
	  group and interpret it numerically.
	  */
  QFont readFontEntry( const QString& rKey ) const;

  /// Read a QColor
  /** Read the value of an entry specified by rKey in the current
	  group and interpret it as a color.
	  */
  QColor readColorEntry( const QString& rKey ) const;

  /// Write the key/value pair
  /** Write the key/value pair. This is stored to the most specific
    config file when destroying the config object or when calling
    Sync().
    The old value for this key is returned. If this key did not
    exist, a NULL string is returned.
	If bPersistent is false, the entry's dirty flag will not be set
	and thus the entry will not be written to disk at deletion time.
   */
  QString writeEntry( const QString& rKey, const QString& rValue,
					  bool bPersistent = true );

  /// Write the key value pair
  /** Same as above, but write a numerical value. */
  QString writeEntry( const QString& rKey, int rValue,
					  bool bPersistent = true );

  /// Write the key value pair
  /** Same as above, but write a font */
  QString writeEntry( const QString& rKey, const QFont& rFont,
					  bool bPersistent = true );

  /// Write the key value pair
  /** Same as above, but write a color */
  QString writeEntry( const QString& rKey, const QColor& rColor,
					  bool bPersistent = true );

  /// Don't write dirty entries at destruction time.
  /** Don't write dirty entries at destruction time. If bDeep is
	false, only the global dirty flag of the KConfig object gets
	cleared. If you then call WriteEntry again, the global dirty flag
	is set again and all dirty entries will be written. If bDeep is
	true, the dirty flags of all entries are cleared, as well as the
	global dirty flag. */
  void rollback( bool bDeep = true );

  /// Flush the entry cache.
  /** Write back dirty configuration entries to the most specific
    file. This is called automatically from the destructor.
   */
  void sync();

  /// Check if there is a value for rKey.
  /** Check if the key has an entry in the specified group
    Use this to determine if a key is not specified for the current
    group (HasKey returns false) or is specified but has no value
    ("key =EOL"; Has Key returns true, ReadEntry returns an empty
    string)
   */
  bool hasKey( const QString& rKey ) const;
};


#endif

