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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIGBASE_H
#define KCONFIGBASE_H

#include <kdecore_export.h>
#include <kconfiggroup.h>
#include <kconfigflags.h>
#include <kdebug.h>

#include <QtGui/QColor>
#include <QtCore/QVariant>

// This include fixes linker errors under msvc:
// In qdbusmessage.h QList<QVariant> is instantiated because
// it is a base class of an exported class. This must be
// known here to avoid an additional instantiation.
#ifdef Q_CC_MSVC
#include <QtDBus/qdbusmessage.h>
#endif

template <typename KT, typename KV> class QMap;
class QString;
#ifdef KDE3_SUPPORT
class Q3StrList;
#endif

class KConfigBackEnd;
class KComponentData;

/**
 * @short KDE Configuration Management abstract base class
 *
 * This class forms the base for all %KDE configuration. It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. Either use KConfig (for usual %KDE
 * configuration) or even KSharedConfig (stores values in shared memory).
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
 * \note the '=' char is not allowed in keys and the ']' char is not allowed in
 * a group name.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @see KGlobal#config()
 * @see KConfig
 * @see KSharedConfig
 */
class KDECORE_EXPORT KConfigBase : public KConfigFlags
{
  friend class KConfigBackEnd;
  friend class KConfigINIBackEnd;
  friend class KConfigGroup;

public:

  /**
   * Construct a KConfigBase object.
   */
  KConfigBase();

  /**
   * Construct a KConfigBase object.
   */
  KConfigBase(const KComponentData &componentData);

  /**
   * Destructs the KConfigBase object.
   */
  virtual ~KConfigBase();

  const KComponentData &componentData() const;

  /**
   * Specifies the group in which keys will be read and written.
   *
   *  Subsequent
   * calls to readEntry() and writeEntry() will be applied only in the
   * activated group.
   *
   * Switch back to the default group by passing a null string.
   * @param group The name of the new group.
   */
  KDE_DEPRECATED void setGroup( const QString& group );

  /**
   * Returns the name of the group in which we are
   *  searching for keys and from which we are retrieving entries.
   *
   * @return The current group.
   */
  KDE_DEPRECATED QString group() const;

  /**
   * Returns true if the specified group is known about.
   *
   * @param group The group to search for.
   * @return true if the group exists.
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
   * @return The value for this key. Can be QString() if aDefault is null.
   */
  KDE_DEPRECATED QString readEntry(const char *pKey, const char *aDefault = 0 ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key, or @p aDefault.
   */
  template <typename T>
      KDE_DEPRECATED T readEntry( const char* pKey, const T& aDefault) const
  { return internalGroup().readEntry(pKey, aDefault); }

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * @copydoc readEntry(const char*, const T&) const
   */
  template <typename T>
      KDE_DEPRECATED T readEntry( const QString& pKey, const T& aDefault) const
    { return internalGroup().readEntry(pKey.toUtf8().constData(), aDefault); }

  //TODO: these two are here temporarily for porting, remove before KDE4
  KDE_DEPRECATED QVariant readPropertyEntry( const QString& pKey, const QVariant& aDefault) const;
  KDE_DEPRECATED QVariant readPropertyEntry( const char *pKey, const QVariant& aDefault) const;

#ifdef KDE3_SUPPORT
  /**
   * Reads a list of strings.
   *
   * @deprecated Use readListEntry(const QString&, char) const instead.
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  KDE_DEPRECATED int readListEntry( const QString& pKey, Q3StrList &list, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @deprecated Use readListEntry(const char*, char) const instead.
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  KDE_DEPRECATED int readListEntry( const char *pKey, Q3StrList &list, char sep = ',' ) const;
#endif

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry( const QString&, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const char*, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const char *pKey, char sep = ',' ) const;

  /**
   * Reads a list of strings, but returns a default if the key
   * did not exist.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @param sep The list separator (default is ",").
   * @return The list. Contains @p aDefault if the Key does not exist.
   * @deprecated use readEntry(const char*, const QStringList&, char) const instead.
   */
  KDE_DEPRECATED QStringList readListEntry( const char* pKey,
                                            const QStringList& aDefault,
                                            char sep = ',' ) const;

  /**
   * Reads a list of strings from the config object.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @param sep The list separator.
   * @return The list. Contains @p aDefault if @p pKey does not exist.
   */
  KDE_DEPRECATED QStringList readEntry(const char* pKey, const QStringList& aDefault,
                        char sep=',') const;

  /**
   * Reads a list of strings, but returns a default if the key
   * did not exist.
   *
   * @copydoc readEntry(const char*, const QStringList&, char) const
   */
  KDE_DEPRECATED QStringList readEntry(const QString& pKey, const QStringList& aDefault,
                                       char sep=',') const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const QString&, const QList<T>&) const instead.
   */
  KDE_DEPRECATED QList<int> readIntListEntry( const QString& pKey ) const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   * @deprecated use readEntry(const char*, const QList<T>&) const instead.
   */
  KDE_DEPRECATED QList<int> readIntListEntry( const char *pKey ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
  KDE_DEPRECATED QString readPathEntry( const QString& pKey, const QString & aDefault = QString() ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be QString() if aDefault is null.
   */
  KDE_DEPRECATED QString readPathEntry( const char *pKey, const QString & aDefault = QString() ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  KDE_DEPRECATED QStringList readPathListEntry( const QString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  KDE_DEPRECATED QStringList readPathListEntry( const char *pKey, char sep = ',' ) const;


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
   * @deprecated
   */
  KDE_DEPRECATED int readNumEntry( const QString& pKey, int nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED int readNumEntry( const char *pKey, int nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED unsigned int readUnsignedNumEntry( const QString& pKey, unsigned int nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED unsigned int readUnsignedNumEntry( const char *pKey, unsigned int nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED long readLongNumEntry( const QString& pKey, long nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED long readLongNumEntry( const char *pKey, long nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED unsigned long readUnsignedLongNumEntry( const QString& pKey, unsigned long nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED unsigned long readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault = 0 ) const;

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED qint64 readNum64Entry( const QString& pKey, qint64 nDefault = 0 ) const;

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED qint64 readNum64Entry( const char *pKey, qint64 nDefault = 0 ) const;

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED quint64 readUnsignedNum64Entry( const QString& pKey, quint64 nDefault = 0 ) const;

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED quint64 readUnsignedNum64Entry( const char *pKey, quint64 nDefault = 0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED double readDoubleNumEntry( const QString& pKey, double nDefault = 0.0 ) const;

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
   * @deprecated
   */
  KDE_DEPRECATED double readDoubleNumEntry( const char *pKey, double nDefault = 0.0 ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED bool readBoolEntry( const QString& pKey, bool bDefault = false ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED bool readBoolEntry( const char *pKey, bool bDefault = false ) const;

  /**
   * Reads a QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QRect readRectEntry( const QString& pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a QRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a QRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QRect readRectEntry( const char *pKey, const QRect* pDefault = 0L ) const;

  /**
   * Reads a QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QPoint readPointEntry( const QString& pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a QPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QPoint readPointEntry( const char *pKey, const QPoint* pDefault = 0L ) const;

  /**
   * Reads a QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QSize readSizeEntry( const QString& pKey, const QSize* pDefault = 0L ) const;

  /**
   * Reads a QSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a QSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null QSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QSize readSizeEntry( const char *pKey, const QSize* pDefault = 0L ) const;


  /**
   * Reads a QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QDateTime readDateTimeEntry( const QString& pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads a QDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   * @deprecated
   */
  KDE_DEPRECATED QDateTime readDateTimeEntry( const char *pKey, const QDateTime* pDefault = 0L ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  KDE_DEPRECATED QString readEntryUntranslated( const QString& pKey,
                     const QString& aDefault = QString() ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  KDE_DEPRECATED QString readEntryUntranslated( const char *pKey,
                     const QString& aDefault = QString() ) const;

  /**
   * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
   */
  template <typename T>
      KDE_DEPRECATED void writeEntry( const char* pKey, const T& value,
                                      WriteConfigFlags pFlags = Normal )
    { internalGroup().writeEntry( pKey, value, pFlags ); }


  /**
   * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
   */
  template <typename T>
      KDE_DEPRECATED void writeEntry( const QString & pKey, const T& value,
                                      WriteConfigFlags pFlags = Normal )
    { internalGroup().writeEntry( pKey, value, pFlags ); }

#ifdef KDE3_SUPPORT
  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
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
  KDE_DEPRECATED void writeEntry( const QString& pKey, const Q3StrList &value,
                                  char sep = ',', bool bPersistent = true,
                                  bool bGlobal = false, bool bNLS = false )
    { writeEntry(pKey.toUtf8().constData(), value, sep, bPersistent, bGlobal, bNLS); }

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
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
  void writeEntry( const char *pKey, const Q3StrList &value,
                   char sep = ',', bool bPersistent = true,
                   bool bGlobal = false, bool bNLS = false );
#endif

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  KDE_DEPRECATED  void writeEntry( const QString& pKey, const QStringList &value,
		   char sep = ',',
                   WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey.toUtf8().constData(), value, sep, pFlags ); }

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const QStringList &value,
		   char sep = ',',
                   WriteConfigFlags pFlags = Normal );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param pFlags       The flags to use when writing this entry.
   */
  KDE_DEPRECATED void writePathEntry( const QString& pKey, const QString & path,
                       WriteConfigFlags pFlags = Normal );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param pFlags       The flags to use when writing this entry.
   */
  KDE_DEPRECATED void writePathEntry( const char *pKey, const QString & path,
                       WriteConfigFlags pFlags = Normal );

  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   */
  KDE_DEPRECATED void writePathEntry( const QString& pKey, const QStringList &value,
                       char sep = ',', WriteConfigFlags pFlags = Normal );

  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param value The list to write
   * @param sep  The list separator (default is ",").
   * @param pFlags       The flags to use when writing this entry.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   */
  KDE_DEPRECATED void writePathEntry( const char *pKey, const QStringList &value,
		   char sep = ',', WriteConfigFlags pFlags = Normal );

  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param pFlags       The flags to use when deleting this entry.
   */
  KDE_DEPRECATED void deleteEntry( const QString& pKey, WriteConfigFlags pFlags = Normal );

  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param pFlags       The flags to use when deleting this entry.
   */
  KDE_DEPRECATED void deleteEntry( const char *pKey, WriteConfigFlags pFlags = Normal );

public:
  /**
   * Deletes a configuration entry group
   *
   * If the group is not empty and Recursive is not set, nothing gets deleted
   * If this group is the current group and it is deleted, the
   * current group is undefined and should be set with setGroup()
   * before the next operation on the configuration object.
   *
   * @param group The name of the group
   * @param pFlags       The flags to use when writing this entry.
   */
  void deleteGroup( const QString& group, WriteConfigFlags pFlags = Normal );

  /**
   * Turns on or off "dollar  expansion" (see KConfigBase introduction)
   *  when reading config entries.
   * Dollar sign expansion is initially OFF.
   *
   * @param _bExpand Tf true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool _bExpand );

  /**
   * Returns whether dollar expansion is on or off.  It is initially OFF.
   *
   * @return true if dollar expansion is on.
   */
  bool isDollarExpansion() const;

  /**
   * Returns whether the locale has been set.
   * @return true if the locale has been initialized
   */
  bool localeInitialized() const;

  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If @p bDeep is false, only the global dirty
   * flag of the KConfig object gets cleared. If you then call
   * writeEntry() again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent sync() call.
   *
   * Classes that derive from KConfigBase should override this
   * method and implement storage-specific behavior, as well as
   * calling the KConfigBase::rollback() explicitly in the initializer.
   *
   * @param bDeep If true, the dirty flags of all entries are cleared,
   *        as well as the global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flushes all changes that currently reside only in memory
   * back to disk / permanent storage. Dirty configuration entries are
   * written to the most specific file available.
   *
   * Asks the back end to flush out all pending writes, and then calls
   * rollback().  No changes are made if the object has @p readOnly
   * status.
   *
   * You should call this from your destructor in derivative classes.
   *
   * @see rollback()
   */
  virtual void sync();

  /**
   * Checks whether the config file has any dirty (modified) entries.
   * @return true if the config file has any dirty (modified) entries.
   */
  bool isDirty() const;

  /**
   * Checks whether the key has an entry in the currently active group.
   * Use this to determine whether a key is not specified for the current
   * group (hasKey() returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param key The key to search for.
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

public:
  /**
   * Reparses all configuration files. This is useful for programs
   * that use stand alone graphical configuration tools. The base
   * method implemented here only clears the group list and then
   * appends the default group.
   *
   * Derivative classes should clear any internal data structures and
   * then simply call parseConfigFiles() when implementing this
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
   * @param key the key to check
   * @return whether the entry @p key may be changed in the current group
   * in this configuration file.
   */
  bool entryIsImmutable(const QString &key) const;

  /**
   * Possible return values for getConfigState().
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

  /**
   * Check whether the config files are writable.
   * @param warnUser Warn the user if the configuration files are not writable.
   * @return Indicates that all of the configuration files used are writable.
   */
  bool checkConfigFilesWritable(bool warnUser);

  /**
   * When set, all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   * This is off by default.
   */
  void setReadDefaults(bool b);

  /**
   * @returns true if all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   */
  bool readDefaults() const;

  /**
   * Reverts the entry with key @p key in the current group in the
   * application specific config file to either the system wide (default)
   * value or the value specified in the global KDE config file.
   *
   * To revert entries in the global KDE config file, the global KDE config
   * file should be opened explicitly in a separate config object.
   *
   * @param key The key of the entry to revert.
   */
  void revertToDefault(const QString &key);

  /**
   * Returns whether a default is specified for an entry in either the
   * system wide configuration file or the global KDE config file.
   *
   * If an application computes a default value at runtime for
   * a certain entry, e.g. like:
   * \code
   * QColor computedDefault = qApp->palette().color(QPalette::Active, QPalette::Text)
   * QColor color = config->readEntry(key, computedDefault);
   * \encode
   *
   * Then it may wish to make the following check before
   * writing back changes:
   * \code
   * if ( (value == computedDefault) && !config->hasDefault(key) )
   *    config->revertToDefault(key)
   * else
   *    config->writeEntry(key, value)
   * \endcode
   *
   * This ensures that as long as the entry is not modified to differ from
   * the computed default, the application will keep using the computed default
   * and will follow changes the computed default makes over time.
   * @param key The key of the entry to check.
   */
  bool hasDefault(const QString &key) const;

protected:
  /**
   * Reads the locale and put in the configuration data struct.
   * Note that this should be done in the constructor, but this is not
   * possible due to some mutual dependencies in KApplication::init()
   */
  void setLocale();

  /**
   * Sets the global dirty flag of the config object
   *
   * @param _bDirty How to mark the object's dirty status
   */
  void setDirty( bool _bDirty );

  /**
   * Sets the backend to use with this config object.
   * Ownership of the backend is taken over by the config object.
   */
  void setBackEnd( KConfigBackEnd* backEnd );

  /**
   * Returns the backend associated with this config object
   */
  KConfigBackEnd* backEnd() const;

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
   * @return The KEntry value (data) found for the key.  @p KEntry.aValue
   * will be the null string if nothing was located.
   * @internal
   */
  virtual KEntry lookupData(const KEntryKey &_key) const = 0;

public:
  /**
   * Overloaded public methods:
   */
  KDE_DEPRECATED void setGroup( const QByteArray &pGroup );
  KDE_DEPRECATED void setGroup( const char *pGroup );
  bool hasKey( const char *pKey ) const;

  bool hasGroup(const QByteArray &_pGroup) const;
  bool hasGroup(const char *_pGroup) const;

protected:
  /**
   * @internal
   */
  virtual bool internalHasGroup(const QByteArray &group) const = 0;
  KConfigGroup& internalGroup( ) const;

  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );

private:
  void internalSetGroup( const QByteArray &_group );

  class Private;
  Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KConfigBase::WriteConfigFlags )

#endif
