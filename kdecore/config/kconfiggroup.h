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

#ifndef KCONFIGGROUP_H
#define KCONFIGGROUP_H

#include <kdecore_export.h>
#include <kdebug.h>
#include <kconfigflags.h>
#include <QtGui/QColor>
#include <QtCore/QVariant>

class KConfigBase;
class KSharedConfigPtr;

/**
 * A class for one specific group in a KConfig object.
 */
class KDECORE_EXPORT KConfigGroup : public KConfigFlags
{
public:
    /**
     * Construct a config group corresponding to @p group in @p master.
     * @p group is the group name encoded in UTF-8.
     */
    KConfigGroup(KConfigBase *master, const QByteArray &group);
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
    KConfigGroup(KSharedConfigPtr master, const QByteArray &group);
    KConfigGroup(KSharedConfigPtr master, const QString &group);
    KConfigGroup(KSharedConfigPtr master, const char * group);

    virtual ~KConfigGroup();

    /**
     * Delete all entries in the entire group
     * @param pFlags flags passed to KConfigBase::deleteGroup
     */
    void deleteGroup(WriteConfigFlags pFlags=Normal);

    /**
     * Returns a map (tree) of entries for all entries in this group.
     *
     * Only the actual entry string is returned, none of the
     * other internal data should be included.
     *
     * @return A map of entries in this group, indexed by key.
     */
    QMap<QString, QString> entryMap() const;

    // The following functions are reimplemented:
    virtual void setDirty(bool _bDirty);
    virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true);
    virtual KEntry lookupData(const KEntryKey &_key) const;
    virtual void sync();

    /**
      * Changes the group of the object. This is a convenience function and should
      * not be overused. Prefer another object for another group to avoid mixture of
      * groups.
    */
    void changeGroup( const char *group );
    void changeGroup( const QString &group ) { changeGroup(group.toLatin1()); }
    void changeGroup( const QByteArray &group) { changeGroup(group.data()); }

    bool hasKey(const char* key) const;

    KConfigGroup(const KConfigGroup &);
    KConfigGroup &operator=(const KConfigGroup &);

    QString group() const;
    bool exists() const;

    /**
     * Return the config object behind this group.
     */
    KConfigBase* config() const;
    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     * If you want to read a path, please use readPathEntry().
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key. Can be QString() if aDefault is null.
     */
    QString readEntry(const char *pKey,
                      const QString& aDefault ) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key. Can be QString() if aDefault is null.
     */
    QString readEntry(const char *pKey, const char *aDefault = 0 ) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     * The value is treated as if it is of the type of the given default value.
     *
     * @note Only the following QVariant types are allowed : String,
     * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
     * Double, LongLong, ULongLong, DateTime and Date.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found or
     * if the read value cannot be converted to the QVariant::Type.
     * @return The value for the key or the default value if the key was not
     *         found.
     */
    QVariant readEntry( const char *pKey, const QVariant &aDefault) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     *
     * This template method makes it possible to write
     *    QString foo = readEntry("...", QString("default"));
     * and the same with all other types supported by QVariant.
     * The return type of the method is simply the same as the type of the default value.
     *
     * @note readEntry("...", Qt::white) cannot compile because Qt::white is an enum.
     * You must turn it into readEntry("...", QColor(Qt::white)).
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key, or @p aDefault.
     */
    template <typename T>
        inline T readEntry( const char* pKey, const T& aDefault) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     * @copydoc readEntry(const char*, const T&) const
     */
    template <typename T>
        T readEntry( const QString& pKey, const T& aDefault) const
    { return readEntry(pKey.toUtf8().constData(), aDefault); }

    /**
     * Reads a list from the config object.
     *
     * @note This function only works for those types that QVariant can convert
     * from QString.
     * @param pKey The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @return The list.
     */
    template <typename T>
        inline QList<T> readEntry( const char* pKey, const QList<T>& aDefault ) const;

    /**
     * Reads a list from the config object.
     *
     * @copydoc readEntry(const char*, const QList<T>&) const
     *
     * @warning This function doesn't convert the items returned
     *          to any type. It's actually a list of QVariant::String's. If you
     *          want the items converted to a specific type use
     *          readEntry(const char*, const QList<T>&) const
     */
    QVariantList readEntry( const char* pKey, const QVariantList& aDefault ) const;

    /**
     * Reads a list of strings from the config object.
     * @param pKey The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @param sep The list separator.
     * @return The list. Contains @p aDefault if @p pKey does not exist.
     */
    QStringList readEntry(const char* pKey, const QStringList& aDefault,
                          char sep=',') const;

    /**
     * Reads a list of strings, but returns a default if the key
     * did not exist.
     *
     * @copydoc readEntry(const char*, const QStringList&, char) const
     */
    QStringList readEntry(const QString& pKey, const QStringList& aDefault,
                          char sep=',') const
    { return readEntry(pKey.toUtf8().constData(), aDefault, sep); }

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
    QString readPathEntry( const QString& pKey, const QString & aDefault = QString() ) const;

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
    QString readPathEntry( const char *pKey, const QString & aDefault = QString() ) const;

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
    QStringList readPathListEntry( const QString& pKey, char sep = ',' ) const;

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
    QStringList readPathListEntry( const char *pKey, char sep = ',' ) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     * The untranslated entry is returned, you normally do not need this.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key.
     */
    QString readEntryUntranslated( const QString& pKey,
                                   const QString& aDefault = QString() ) const;

    /**
     * Reads the value of an entry specified by @p pKey in the current group.
     * The untranslated entry is returned, you normally do not need this.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key.
     */
    QString readEntryUntranslated( const char *pKey,
                                   const QString& aDefault = QString() ) const;

    /**
     * Writes a key/value pair.
     *
     * This is stored in the most specific config file when destroying the
     * config object or when calling sync().
     *
     * If you want to write a path, please use writePathEntry().
     *
     * @param pKey         The key to write.
     * @param value       The value to write.
     * @param pFlags       The flags to use when writing this entry.
     */
    void writeEntry( const char *pKey, const QString& value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * writeEntry() Overridden to accept a property.
     *
     * @param pKey The key to write
     * @param value The property to write
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  writeEntry()
     */
    void writeEntry( const char *pKey, const QVariant& value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
     */
    template <typename T>
        void writeEntry( const char* pKey, const T& value,
                         WriteConfigFlags pFlags = Normal );

    /**
     * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
     */
    template <typename T>
        void writeEntry( const QString& pKey, const T& value,
                         WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey.toUtf8().constData(), value, pFlags ); }

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
    void writeEntry( const QString& pKey, const QStringList &value,
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
     * writeEntry() overridden to accept a list.
     *
     * @param pKey The key to write
     * @param value The list to write.
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  writeEntry()
     */
    template <typename T>
        void writeEntry( const char* pKey, const QList<T>& value,
                         WriteConfigFlags pFlags = Normal );

    /**
     * writeEntry() overridden to accept a list of variants.
     * @copydoc writeEntry(const char*, const QList<T>&, WriteConfigFlags)
     */
    void writeEntry( const char* pKey, const QVariantList& value,
                     WriteConfigFlags pFlags = Normal )
    { writeEntry( pKey, QVariant(value), pFlags ); }

    /**
     * Write a (key/value) pair where the value is a const char*.
     *
     * This is stored to the most specific config file when destroying the
     * config object or when calling sync().
     *
     *  @param pKey               The key to write.
     *  @param value     The value to write; assumed to be in latin1 encoding.
     *  @param pFlags       The flags to use when writing this entry.
     */
    void writeEntry( const char *pKey, const char *value,
                     WriteConfigFlags pFlags = Normal )
    { writeEntry(pKey, QString::fromLatin1(value), pFlags); }

    /**
     * Write a (key/value) pair where the value is a QByteArray.
     *
     * This is stored to the most specific config file when destroying the
     * config object or when calling sync().
     *
     *  @param pKey               The key to write.
     *  @param value     The value to write; assumed to be in latin1 encoding.
     *                    If it contains the null character between 0 and size()-1,
     *                    the string will be truncated at the null character.
     *
     * @param pFlags       The flags to use when writing this entry.
     */
    void writeEntry( const char *pKey, const QByteArray& value,
                     WriteConfigFlags pFlags = Normal )
    { writeEntry(pKey, QString::fromLatin1(value, value.size()), pFlags); }

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
    void writePathEntry( const QString& pKey, const QString & path,
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
    void writePathEntry( const char *pKey, const QString & path,
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
    void writePathEntry( const QString& pKey, const QStringList &value,
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
    void writePathEntry( const char *pKey, const QStringList &value,
                         char sep = ',', WriteConfigFlags pFlags = Normal );

    /**
     * Deletes the entry specified by @p pKey in the current group.
     *
     * @param pKey The key to delete.
     * @param pFlags       The flags to use when deleting this entry.
     */
    void deleteEntry( const QString& pKey, WriteConfigFlags pFlags = Normal );

    /**
     * Deletes the entry specified by @p pKey in the current group.
     *
     * @param pKey The key to delete.
     * @param pFlags       The flags to use when deleting this entry.
     */
    void deleteEntry( const char *pKey, WriteConfigFlags pFlags = Normal );

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
     * Checks whether it is possible to change the given group.
     * @return whether changes may be made to this group in this configuration
     * file.
     */
    bool isImmutable() const;

    /**
     * Checks whether it is possible to change the given entry.
     * @param key the key to check
     * @return whether the entry @p key may be changed in the current group
     * in this configuration file.
     */
    bool entryIsImmutable(const QString &key) const;

    /**
     * When set, all readEntry and readXXXEntry calls return the system
     * wide (default) values instead of the user's preference.
     * This is off by default.
     */
    void setReadDefaults(bool b);

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
     * QColor color = config->readEntry(key, computedDefault).value<QColor>();
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
    QByteArray readEntryUtf8( const char *pKey) const;

    void init(KConfigBase *master);

protected:
    /** Virtual hook, used to add new "virtual" functions while maintaining
        binary compatibility. Unused in this class.
    */
    virtual void virtual_hook( int id, void* data );
private:
    class Private;
    Private * const d;
};

#define KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)                         \
template <>                                                               \
inline Class::Enum KConfigGroup::readEntry(const char* pKey, const Class::Enum& value) const\
{                                                                         \
const QMetaObject* Mobj = &Class::staticMetaObject;                       \
const QMetaEnum Menum = Mobj->enumerator(Mobj->indexOfEnumerator(#Enum)); \
int tmp = value;                                                          \
if (Menum.isFlag()) {                                                     \
  const QString str = readEntry(pKey, Menum.valueToKeys(tmp));            \
  tmp = Menum.keysToValue(str.toLatin1().constData());                    \
} else {                                                                  \
  const QString str = readEntry(pKey, Menum.valueToKey(tmp));             \
  tmp = Menum.keyToValue(str.toLatin1().constData());                     \
}                                                                         \
return static_cast<Class::Enum>(tmp);                                     \
}                                                                         \
template <>                                                               \
void KConfigGroup::writeEntry(const char* pKey, const Class::Enum& value, WriteConfigFlags flags)\
{                                                                         \
const QMetaObject* Mobj = &Class::staticMetaObject;                       \
const QMetaEnum Menum = Mobj->enumerator(Mobj->indexOfEnumerator(#Enum)); \
if (Menum.isFlag()) writeEntry(pKey, Menum.valueToKeys(value), flags);    \
else writeEntry(pKey, Menum.valueToKey(value), flags);                    \
}

#define KCONFIG_QVARIANT_CHECK 1
#if KCONFIG_QVARIANT_CHECK
#include <conversion_check.h>
#endif

/*
 * just output error, or any wrong config file will
 * let the app exit, which is IMHO bad
 */
#define kcbError kWarning

template <typename T>
inline QList<T> KConfigGroup::readEntry( const char* pKey, const QList<T>& aDefault) const
{
  QVariant::Type wanted = QVariant(T()).type();
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();
#else
  kcbError(!QVariant(QVariant::String).canConvert(wanted))
    << "QString cannot convert to \"" << QVariant::typeToName(wanted)
    << "\" information will be lost" << endl;
#endif

  if (!hasKey(pKey))
    return aDefault;

  QList<QVariant> vList;

  if (!aDefault.isEmpty()) {
    Q_FOREACH (const T &aValue, aDefault)
      vList.append( aValue );
  }
  vList = readEntry( pKey, vList );

  QList<T> list;
  if (!vList.isEmpty()) {
    Q_FOREACH (QVariant aValue, vList) {
      kcbError(!aValue.convert(wanted)) << "conversion to "
        << QVariant::typeToName(wanted) << " information has been lost" << endl;
      list.append( qvariant_cast<T>(aValue) );
    }
  }

  return list;
}

template <typename T>
inline T KConfigGroup::readEntry( const char* pKey, const T& aDefault) const
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
#endif
  return qvariant_cast<T>(readEntry(pKey, qVariantFromValue(aDefault)));
}

template <typename T>
void KConfigGroup::writeEntry( const char* pKey, const QList<T>& value,
                              WriteConfigFlags pFlags )
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();
#else
  QVariant dummy QVariant(T());
  kcbError(!dummy.canConvert(QVariant::String))
    << QVariant::typeToName(dummy.type())
    << " cannot convert to QString information will be lost" << endl;
#endif

  QVariantList vList;
  Q_FOREACH(const T &aValue, value)
    vList.append(aValue);

  writeEntry( pKey, QVariant(vList), pFlags );
}

template <typename T>
inline void KConfigGroup::writeEntry( const char* pKey, const T& value,
                              WriteConfigFlags pFlags )
{
#if KCONFIG_QVARIANT_CHECK
  ConversionCheck::to_QVariant<T>();
#endif
  writeEntry( pKey, qVariantFromValue(value), pFlags );
}

#endif // KCONFIGGROUP_H
