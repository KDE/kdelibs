/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef _KCONFIGBASE_H
#define _KCONFIGBASE_H

#include <qobject.h>
#include <qcolor.h>
#include <qfont.h>
#include <qdatetime.h>
#include <qstrlist.h>
#include <qstringlist.h>
#include <qvariant.h>
#include <qmap.h>

#include "kconfigdata.h"

class KConfigBackEnd;
class KConfigBasePrivate;
class KConfigGroup;

/**
 * Abstract base class for KDE configuration entries.
 *
 * This class forms the base for all KDE configuration. It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. Either use @ref KConfig (for usual KDE
 * configuration) or @ref KSimpleConfig (for special needs as in ksamba), or
 * even KSharedConfig (stores values in shared memory).
 *
 * All configuration entries are key, value pairs.  Each entry also
 * belongs to a specific group of related entries.  All configuration
 * entries that do not explicitly specify which group they are in are
 * in a special group called the default group.
 *
 * If there is a $ character in an entry, KConfigBase tries to expand
 * environment variable and uses its value instead of its name. You
 * can avoid this feature by having two consecutive $ characters in
 * your config file which get expanded to one.
 *
 * Note: the '=' char is not allowed in keys and the ']' char is not allowed in
 * group name.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @version $Id$
 * @see  KGlobal#config()  KConfig  KSimpleConfig
 * @short KDE Configuration Management abstract base class
 */
class KConfigBase : public QObject
{
  Q_OBJECT

  friend class KConfigBackEnd;
  friend class KConfigINIBackEnd;
  friend class KConfigGroup;

public:
  /**
   * Construct a KConfigBase object.
   */
  KConfigBase();

  /**
   * Destructs the KConfigBase object.
   */
  virtual ~KConfigBase();

  /**
   * Specifies the group in which keys will be read and written.
   *
   *  Subsequent
   * calls to @ref readEntry() and @ref writeEntry() will be aplied only in the
   * activated group.
   *
   * Switch back to the default group by passing a null string.
   * @param group The name of the new group.
   */
  void setGroup( const QString& group );

  /**
   * Sets the group to the "Desktop Entry" group used for
   * desktop configuration files for applications, mime types, etc.
   */
  void setDesktopGroup();

  /**
   * Returns the name of the group in which we are
   *  searching for keys and from which we are retrieving entries.
   *
   * @return The current group.
   */
  QString group() const;

  /**
   * Returns true if the specified group is known about.
   *
   * @param group The group to search for.
   * @return Whether the group exists.
   */
  bool hasGroup(const QString &group) const;

  /**
   * Returns a list of groups that are known about.
   *
   * @return The list of groups.
   **/
  virtual QStringList groupList() const = 0;

  /**
   * Returns a the current locale.
   *
   * @return A string representing the current locale.
   */
  QString locale() const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString::null if aDefault it null.
   */
   QString readEntry(const QString& pKey,
                     const QString& aDefault = QString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString::null if aDefault it null.
   */
   QString readEntry(const char *pKey,
                     const QString& aDefault = QString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the given type.
   *
   * @deprecated
   *
   * @param pKey The key to search for.
   * @return An invalid QVariant if the key was not found or if the
   * read value cannot be converted to the given QVariant::Type.
   *
   * Note that only the following QVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   */
  QVariant readPropertyEntry( const QString& pKey, QVariant::Type ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the given type.
   *
   * @deprecated
   *
   * @param pKey The key to search for.
   * @return An invalid QVariant if the key was not found or if the
   * read value cannot be converted to the given QVariant::Type.
   *
   * Note that only the following QVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   */
  QVariant readPropertyEntry( const char *pKey, QVariant::Type ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the type of the given default value.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found or
   * if the read value cannot be converted to the QVariant::Type.
   * @return The value for the key or the default value if the key was not
   *         found.
   *
   * Note that only the following QVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   */
  QVariant readPropertyEntry( const QString& pKey,
                              const QVariant &adefault) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the type of the given default value.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found or
   * if the read value cannot be converted to the QVariant::Type.
   * @return The value for the key or the default value if the key was not
   *         found.
   *
   * Note that only the following QVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   */
  QVariant readPropertyEntry( const char *pKey,
                              const QVariant &aDefault) const;

  /**
   * Reads a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  int readListEntry( const QString& pKey, QStrList &list, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  int readListEntry( const char *pKey, QStrList &list, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  QStringList readListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  QStringList readListEntry( const char *pKey, char sep = ',' ) const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   */
  QValueList<int> readIntListEntry( const QString& pKey ) const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   */
  QValueList<int> readIntListEntry( const char *pKey ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString::null if aDefault is null.
   */
  QString readPathEntry( const QString& pKey, const QString & aDefault = QString::null ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString::null if aDefault is null.
   */
  QString readPathEntry( const char *pKey, const QString & aDefault = QString::null ) const;

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  int readNumEntry( const QString& pKey, int nDefault = 0 ) const;

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  int readNumEntry( const char *pKey, int nDefault = 0 ) const;

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned int readUnsignedNumEntry( const QString& pKey, unsigned int nDefault = 0 ) const;

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned int readUnsignedNumEntry( const char *pKey, unsigned int nDefault = 0 ) const;


  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  long readLongNumEntry( const QString& pKey, long nDefault = 0 ) const;

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  long readLongNumEntry( const char *pKey, long nDefault = 0 ) const;

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned long readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault = 0 ) const;

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned long readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault = 0 ) const;

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  double readDoubleNumEntry( const QString& pKey, double nDefault = 0.0 ) const;

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  double readDoubleNumEntry( const char *pKey, double nDefault = 0.0 ) const;

  /**
   * Reads a @ref QFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QFont readFontEntry( const QString& pKey, const QFont* pDefault = 0L ) const;

  /**
   * Reads a @ref QFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QFont readFontEntry( const char *pKey, const QFont* pDefault = 0L ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on" and "true" are
   * accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  bool readBoolEntry( const QString& pKey, const bool bDefault = false ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on" and "true" are
   * accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  bool readBoolEntry( const char *pKey, const bool bDefault = false ) const;

  /**
   * Reads a @ref QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a @ref QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QRect readRectEntry( const QString& pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a @ref QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a @ref QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QRect readRectEntry( const char *pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a @ref QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a @ref QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QPoint readPointEntry( const QString& pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a @ref QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a @ref QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QPoint readPointEntry( const char *pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a @ref QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a @ref QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QSize readSizeEntry( const QString& pKey, const QSize* pDefault = 0L ) const;

  /**
   * Reads a @ref QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a @ref QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  QSize readSizeEntry( const char *pKey, const QSize* pDefault = 0L ) const;


  /**
   * Reads a @ref QColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   */
  QColor readColorEntry( const QString& pKey, const QColor* pDefault = 0L ) const;

  /**
   * Reads a @ref QColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null QColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   */
  QColor readColorEntry( const char *pKey, const QColor* pDefault = 0L ) const;

  /**
   * Reads a @ref QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (@ref currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   */
  QDateTime readDateTimeEntry( const QString& pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads a @ref QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (@ref currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   */
  QDateTime readDateTimeEntry( const char *pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   QString readEntryUntranslated( const QString& pKey,
                     const QString& aDefault = QString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   QString readEntryUntranslated( const char *pKey,
                     const QString& aDefault = QString::null ) const;

  /**
   * Writes a key/value pair.
   *
   * This is stored in the most specific config file when destroying the
   * config object or when calling @ref sync().
   *
   * @param pKey         The key to write.
   * @param pValue       The value to write.
   * @param bPersistent  If @p bPersistent is false, the entry's dirty
   *                     flag will not be set and thus the entry will
   *                     not be written to disk at deletion time.
   * @param bGlobal      If @p bGlobal is true, the pair is not saved to the
   *                     application specific config file, but to the
   *                     global KDE config file.
   * @param bNLS         If @p bNLS is true, the locale tag is added to the key
   *                     when writing it back.
   */
  void writeEntry( const QString& pKey, const QString& pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a key/value pair.
   *
   * This is stored in the most specific config file when destroying the
   * config object or when calling @ref sync().
   *
   * @param pKey         The key to write.
   * @param pValue       The value to write.
   * @param bPersistent  If @p bPersistent is false, the entry's dirty
   *                     flag will not be set and thus the entry will
   *                     not be written to disk at deletion time.
   * @param bGlobal      If @p bGlobal is true, the pair is not saved to the
   *                     application specific config file, but to the
   *                     global KDE config file.
   * @param bNLS         If @p bNLS is true, the locale tag is added to the key
   *                     when writing it back.
   */
  void writeEntry( const char *pKey, const QString& pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * @ref writeEntry() Overridden to accept a property.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The property to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const QString& pKey, const QVariant& rValue,
                    bool bPersistent = true, bool bGlobal = false,
                    bool bNLS = false );
  /**
   * @ref writeEntry() Overridden to accept a property.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The property to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QVariant& rValue,
                    bool bPersistent = true, bool bGlobal = false,
                    bool bNLS = false );

  /**
   * @ref writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const QString& pKey, const QStrList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
  /**
   * @ref writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QStrList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * @ref writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const QString& pKey, const QStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
  /**
   * @ref writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );


 /**
   * @ref writeEntry() overridden to accept a list of Integers.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const QString& pKey, const QValueList<int>& rValue,
		   bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
 /**
   * @ref writeEntry() overridden to accept a list of Integers.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QValueList<int>& rValue,
		   bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * Write a (key/value) pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling @ref sync().
   *
   *  @param pKey               The key to write.
   *  @param pValue     The value to write.
   *  @param bPersistent        If @p bPersistent is false, the entry's dirty
   *                    flag will not be set and thus the entry will
   *                    not be written to disk at deletion time.
   *  @param bGlobal    If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   *  @param bNLS       If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const QString& pKey, const char *pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false )
    { writeEntry(pKey, QString::fromLatin1(pValue), bPersistent, bGlobal, bNLS); }
  /**
   * Write a (key/value) pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling @ref sync().
   *
   *  @param pKey               The key to write.
   *  @param pValue     The value to write.
   *  @param bPersistent        If @p bPersistent is false, the entry's dirty
   *                    flag will not be set and thus the entry will
   *                    not be written to disk at deletion time.
   *  @param bGlobal    If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   *  @param bNLS       If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, const char *pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false )
    { writeEntry(pKey, QString::fromLatin1(pValue), bPersistent, bGlobal, bNLS); }

  /**
   * Write a (key/value) pair.
   * Same as above, but writes a numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const QString& pKey, int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Write a (key/value) pair.
   * Same as above, but writes a numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const QString& pKey, unsigned int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, unsigned int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but write a long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but write a long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, unsigned long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, unsigned long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param format      @p format determines the format to which the value
   *  is converted. Default is 'g'.
   * @param precision   @p precision sets the precision with which the
   *  value is converted. Default is 6 as in QString.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, double nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      char format = 'g', int precision = 6,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param format      @p format determines the format to which the value
   *  is converted. Default is 'g'.
   * @param precision   @p precision sets the precision with which the
   *  value is converted. Default is 6 as in QString.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, double nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      char format = 'g', int precision = 6,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a boolean value.
   *
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, bool bValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a boolean value.
   *
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, bool bValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a font value.
   *
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QFont& rFont,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a font value.
   *
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QFont& rFont,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but write a color entry.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The color value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QColor& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but write a color entry.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The color value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QColor& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a date and time entry.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * @em not returned here!
   *
   * @param pKey The key to write.
   * @param rValue The date and time value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QDateTime& rDateTime,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a date and time entry.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * @em not returned here!
   *
   * @param pKey The key to write.
   * @param rValue The date and time value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QDateTime& rDateTime,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );


  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a rectangle.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QRect& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a rectangle.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QRect& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a point.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QPoint& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a point.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QPoint& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a size.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const QString& pKey, const QSize& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a size.
   *
   * Note: Unlike the other @ref writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const QSize& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with @ref readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writePathEntry( const QString& pKey, const QString & path,
                       bool bPersistent = true, bool bGlobal = false,
                       bool bNLS = false );
  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with @ref readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writePathEntry( const char *pKey, const QString & path,
                       bool bPersistent = true, bool bGlobal = false,
                       bool bNLS = false );


  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param bGlobal     If @p bGlobal is true, the pair is not removed from the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the key with the locale tag is removed.
   */
   void deleteEntry( const QString& pKey,
                   bool bNLS = false, bool bGlobal = false);
  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param bGlobal     If @p bGlobal is true, the pair is not removed from the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the key with the locale tag is removed.
   */
   void deleteEntry( const char *pKey,
                   bool bNLS = false, bool bGlobal = false);

  /**
   * Deletes a configuration entry group
   *
   * If the group is not empty and bDeep is false, nothing gets
   * deleted and false is returned.
   * If this group is the current group and it is deleted, the
   * current group is undefined and should be set with setGroup()
   * before the next operation on the configuration object.
   *
   * @param group The name of the group
   * @param bDeep Specify whether non-empty groups should be completely
   *        deleted (including their entries).
   * @param bGlobal     If @p bGlobal is true, the pair is not removed from the
   *  application specific config file, but to the global KDE config file.
   * @return If the group is not empty and bDeep is false,
   *         deleteGroup returns false.
   */
  bool deleteGroup( const QString& group, bool bDeep = true, bool bGlobal = false );


  /**
   * Turns on or off "dollar  expansion" (see KConfigBase introduction)
   *  when reading config entries.
   * Dollar sign expansion is initially OFF.
   *
   * @param _bExpand Tf true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool _bExpand = true ) { bExpand = _bExpand; }

  /**
   * Returns whether dollar expansion is on or off.  It is initially OFF.
   *
   * @return true if dollar expansion is on.
   */
  bool isDollarExpansion() const { return bExpand; }

  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If @p bDeep is false, only the global dirty
   * flag of the KConfig object gets cleared. If you then call
   * @ref writeEntry() again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent @ref sync() call.
   *
   * Classes that derive from KConfigBase should override this
   * method and implement storage-specific behavior, as well as
   * calling the @ref KConfigBase::rollback() explicitly in the initializer.
   *
   * @param bDeep If @p true, the dirty flags of all entries are cleared,
   *        as well as the global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flushes all changes that currently reside only in memory
   * back to disk / permanent storage. Dirty configuration entries are
   * written to the most specific file available.
   *
   * Asks the back end to flush out all pending writes, and then calls
   * @ref rollback().  No changes are made if the object has @p readOnly
   * status.
   *
   * You should call this from your destructor in derivative classes.
   *
   * @see #rollback(), #isReadOnly()
   */
  virtual void sync();

  /**
   * Checks whether the config file has any dirty (modified) entries.
   * @return @p true if the config file has any dirty (modified) entries.
   */
  bool isDirty() const { return bDirty; }

  /**
   * Sets the config object's read-only status.
   *
   * @param _ro If @p true, the config object will not write out any
   *        changes to disk even if it is destroyed or @ref sync() is called.
   *
   */
  virtual void setReadOnly(bool _ro) { bReadOnly = _ro; }

   /**
    * Returns the read-only status of the config object.
    *
    * @return The read-only status.
    */
  bool isReadOnly() const { return bReadOnly; }

  /**
   * Checks whether the key has an entry in the currently active group.
   * Use this to determine whether a key is not specified for the current
   * group (hasKey() returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param pKey The key to search for.
   * @return If true, the key is available.
   */
  bool hasKey( const QString& key ) const;

  /**
   * Returns a map (tree) of entries for all entries in a particular
   * group.  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param group A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   * @see   QMap
   */
  virtual QMap<QString, QString> entryMap(const QString &group) const = 0;

  /**
   * Reparses all configuration files. This is useful for programs
   * that use stand alone graphical configuration tools. The base
   * method implemented here only clears the group list and then
   * appends the default group.
   *
   * Derivative classes should clear any internal data structures and
   * then simply call @ref parseConfigFiles() when implementing this
   * method.
   *
   * @see  parseConfigFiles()
   */
  virtual void reparseConfiguration() = 0;

  /**
   * Checks whether this configuration file can be modified.
   * @return whether changes may be made to this configuration file.
   */
  bool isImmutable() const;

  /**
   * Checks whether it is possible to change the given group.
   * @param group the group to check
   * @return whether changes may be made to @p group in this configuration
   * file.
   */
  bool groupIsImmutable(const QString &group) const;

  /**
   * Checks whether it is possible to change the given entry.
   * @param the key to check
   * @return whether the entry @p key may be changed in the current group
   * in this configuration file.
   */
  bool entryIsImmutable(const QString &key) const;

  /**
   * Possible return values for @ref getConfigState().
   *
   * @see  getConfigState()
   */
  enum ConfigState { NoAccess, ReadOnly, ReadWrite };

  /**
   * Returns the state of the app-config object.
   *
   * Possible return values
   * are NoAccess (the application-specific config file could not be
   * opened neither read-write nor read-only), ReadOnly (the
   * application-specific config file is opened read-only, but not
   * read-write) and ReadWrite (the application-specific config
   * file is opened read-write).
   *
   * @see  ConfigState()
   * @return the state of the app-config object
   */
  ConfigState getConfigState() const;

protected:
  /**
   * Reads the locale and put in the configuration data struct.
   * Note that this should be done in the constructor, but this is not
   * possible due to some mutual dependencies in @ref KApplication::init()
   */
  void setLocale();

  /**
   * Sets the global dirty flag of the config object
   *
   * @param _bDirty How to mark the object's dirty status
   */
  virtual void setDirty(bool _bDirty = true) { bDirty = _bDirty; }

  /**
   * Parses all configuration files for a configuration object.
   *
   * The actual parsing is done by the associated KConfigBackEnd.
   */
  virtual void parseConfigFiles();

  /**
   * Returns a map (tree) of the entries in the specified group.
   * This may or may not return all entries that belong to the
   * config object.  The only guarantee that you are given is that
   * any entries that are dirty (i.e. modified and not yet written back
   * to the disk) will be contained in the map.  Some derivative
   * classes may choose to return everything.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup The group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
  virtual KEntryMap internalEntryMap( const QString& pGroup ) const = 0;

  /**
   * Returns a map (tree) of the entries in the tree.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @return A map of the entries in the tree.
   *
   * @internal
   *
   */
  virtual KEntryMap internalEntryMap() const = 0;

  /**
   * Inserts a (key/value) pair into the internal storage mechanism of
   * the configuration object. Classes that derive from KConfigBase
   * will need to implement this method in a storage-specific manner.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   * @param _checkGroup When false, assume that the group already exists.
   * @internal
   */
  virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true) = 0;

  /**
   * Looks up an entry in the config object's internal structure.
   * Classes that derive from KConfigBase will need to implement this
   * method in a storage-specific manner.
   *
   * Do not use this function, the implementation and return type are
   * subject to change.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return The @ref KEntry value (data) found for the key.  @p KEntry.aValue
   * will be the null string if nothing was located.
   * @internal
   */
  virtual KEntry lookupData(const KEntryKey &_key) const = 0;

  virtual bool internalHasGroup(const QCString &group) const = 0;

  /**
   * A back end for loading/saving to disk in a particular format.
   */
  KConfigBackEnd *backEnd;
public:
  /**
   * Overloaded public methods:
   */
  void setGroup( const QCString &pGroup );
  void setGroup( const char *pGroup );
  bool hasGroup(const QCString &_pGroup) const;
  bool hasGroup(const char *_pGroup) const;
  bool hasKey( const char *pKey ) const;

protected:
  QCString readEntryUtf8( const char *pKey) const;

  /**
   * The currently selected group. */
  QCString mGroup;

  /**
   * The locale to retrieve keys under if possible, i.e en_US or fr.  */
  QCString aLocaleString;

  /**
   * Indicates whether there are any dirty entries in the config object
   * that need to be written back to disk. */
  bool bDirty;

  bool bLocaleInitialized;
  bool bReadOnly;           // currently only used by KSimpleConfig
  mutable bool bExpand;     // whether dollar expansion is used

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KConfigBasePrivate *d;
};

class KConfigGroupSaverPrivate;

/**
  * Helper class to facilitate working with @ref KConfig / @ref KSimpleConfig
  * groups.
  *
  * Careful programmers always set the group of a
  * @ref KConfig @ref KSimpleConfig object to the group they want to read from
  * and set it back to the old one of afterwards. This is usually
  * written as:
  * <pre>
  *
  * QString oldgroup config->group();
  * config->setGroup( "TheGroupThatIWant" );
  * ...
  * config->writeEntry( "Blah", "Blubb" );
  *
  * config->setGroup( oldgroup );
  * </pre>
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
  * @author Matthias Kalle Dalheimer <kalle@kde.org>
  * @version $Id$
  * @see KConfigBase, KConfig, KSimpleConfig
  * @short Helper class for easier use of KConfig/KSimpleConfig groups
  */

class KConfigGroupSaver
{
public:
  /**
   * Constructor. You pass a pointer to the KConfigBase-derived
   * object you want to work with and a string indicating the _new_
   * group.
   * @param config The KConfigBase-derived object this
   *               KConfigGroupSaver works on.
   * @param group  The new group that the config object should switch to.
   */
  KConfigGroupSaver( KConfigBase* config, QString group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  KConfigGroupSaver( KConfigBase* config, const char *group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  KConfigGroupSaver( KConfigBase* config, const QCString &group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  ~KConfigGroupSaver() { _config->setGroup( _oldgroup ); }

    KConfigBase* config() { return _config; };

private:
  KConfigBase* _config;
  QString _oldgroup;

  KConfigGroupSaver(const KConfigGroupSaver&);
  KConfigGroupSaver& operator=(const KConfigGroupSaver&);

  KConfigGroupSaverPrivate *d;
};

class KConfigGroupPrivate;

class KConfigGroup: public KConfigBase
{
public:
   /**
    * Construct a config group corresponding to @p group in @p master.
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const QCString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    */
   KConfigGroup(KConfigBase *master, const QString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    * @p group is the group name encoded in UTF-8.
    */
   KConfigGroup(KConfigBase *master, const char * group);

   /**
    * Delete all entries in the entire group
    * @param bGlobal     If @p bGlobal is true, the entries are not removed
    *        from the application specific config file, but from the global
    *        KDE config file.
    */
   void deleteGroup(bool bGlobal = false);

   // The following functions are reimplemented:
   virtual void setDirty(bool b);
   virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true);
   virtual KEntry lookupData(const KEntryKey &_key) const;
   virtual void sync();

private:
   // Hide the following members:
   void setGroup() { }
   void setDesktopGroup() { }
   void group() { }
   void hasGroup() { }
   void setReadOnly(bool) { }
   void isDirty() { }

   // The following members are not used.
   virtual QStringList groupList() const { return QStringList(); }
   virtual void rollback(bool) { }
   virtual void reparseConfiguration() { }
   virtual QMap<QString, QString> entryMap(const QString &) const
    { return QMap<QString,QString>(); }
   virtual KEntryMap internalEntryMap( const QString&) const
    { return KEntryMap(); }
   virtual KEntryMap internalEntryMap() const
    { return KEntryMap(); }
   virtual bool internalHasGroup(const QCString &) const
    { return false; }

   void getConfigState() { }

   KConfigBase *mMaster;
protected:
   virtual void virtual_hook( int id, void* data );
private:
   KConfigGroupPrivate* d;
};

#endif
