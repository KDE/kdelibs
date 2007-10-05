/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
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
#include <kconfigbase.h>
#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QVariant>

class KConfig;
class KConfigGroupPrivate;
class KSharedConfig;
template <typename T> class KSharedPtr;
typedef KSharedPtr<KSharedConfig> KSharedConfigPtr;

/**
 * A class for one specific group in a KConfig object.
 * 
 * If you want to access the top-level entries of a KConfig
 * object, which are not associated with any group, use an
 * empty group name.
 * A KConfigGroup can be read-only if it is constructed from a const config object
 * or from a read-only group.
 */
class KDECORE_EXPORT KConfigGroup : public KConfigBase
{
public:
    /**
     * Constructs a null group. A null group is invalid.
     *
     * \see isValid
     */
    KConfigGroup();

    /**
     * Construct a config group corresponding to @p group in @p master.
     * @p group is the group name encoded in UTF-8.
     */
    KConfigGroup(KConfigBase *master, const QByteArray &group);
    KConfigGroup(KConfigBase *master, const QString &group);
    KConfigGroup(KConfigBase *master, const char *group);

    KConfigGroup(KSharedConfigPtr &master, const QByteArray &group);
    KConfigGroup(KSharedConfigPtr &master, const QString &group);
    KConfigGroup(KSharedConfigPtr &master, const char *group);

    /**
     * Construct a read-only config group. A read-only group will silently ignore
     * any attempts to write to it.
     */
    KConfigGroup(const KConfigBase *master, const QString &group);
    KConfigGroup(const KConfigBase *master, const QByteArray &group);
    KConfigGroup(const KConfigBase *master, const char *group);

    KConfigGroup(const KSharedConfigPtr &master, const QString& group);
    KConfigGroup(const KSharedConfigPtr &master, const QByteArray& group);
    KConfigGroup(const KSharedConfigPtr &master, const char* group);

    KConfigGroup(const KConfigGroup &);
    KConfigGroup &operator=(const KConfigGroup &);

    ~KConfigGroup();

    /**
     * Returns \p true if the group is valid; otherwise returns \p false. A group is invalid if it
     * was constructed without arguments.
     *
     * You should not call any functions on an invalid group.
     */
    bool isValid() const;

    /**
     * Delete all entries in the entire group
     * @param pFlags flags passed to KConfig::deleteGroup
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

    /**
     * Syncs the configuration object that this group belongs to.
     */
    void sync();

    /**
     * Changes the group of the object. This is a convenience function and should
     * not be overused. Prefer another object for another group to avoid mixture of
     * groups. A subgroup can only change to another subgroup of the parent.
     */
    void changeGroup( const QString &group );
    void changeGroup( const QByteArray &group);
    void changeGroup( const char *group);

    QString name() const;
    bool exists() const;

    /**
     * Return the config object that this group belongs to.
     */
    KConfig* config();
    const KConfig* config() const;

    /**
     * Return the data in @p value converted to a QVariant.
     * 
     * @param pKey The name of the entry being converted, this is only used for error
     * reporting.
     * @param value The UTF-8 data to be converted.
     * @param aDefault The default value if @p pKey is not found.
     * @return @p value converted to QVariant, or @p aDefault if @p value is invalid or cannot be converted.
     */
    static QVariant convertToQVariant(const char* pKey, const QByteArray& value, const QVariant& aDefault);

    QString readEntry(const char *key, const char * aDefault = 0) const
    { return readEntry(QByteArray(key), aDefault); }
    QString readEntry(const QByteArray &key, const char * aDefault = 0) const;
    QString readEntry(const QString &key, const char * aDefault = 0) const
    { return readEntry(key.toUtf8(), aDefault); }


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
     * @note Only the following QVariant types are allowed : String,
     * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
     * Double, LongLong, ULongLong, DateTime and Date.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key, or @p aDefault.
     */
    template <typename T>
    inline T readEntry(const QByteArray &key, const T &defaultValue) const
    { return readListCheck(key, defaultValue); }

    template <typename T>
        T readEntry( const QString& pKey, const T& aDefault) const
    { return readEntry(pKey.toUtf8(), aDefault); }
    template <typename T>
        T readEntry( const char *key, const T& aDefault) const
    { return readEntry(QByteArray(key), aDefault); }

    /**
     * Reads a list of strings from the config object.
     * @param pKey The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @param sep The list separator.
     * @return The list. Contains @p aDefault if @p pKey does not exist.
     */
    QStringList readEntry(const char* pKey, const QStringList& aDefault,
                          char sep=',') const;
    QStringList readEntry(const QByteArray& pKey, const QStringList& aDefault,
                          char sep=',') const;
    QStringList readEntry(const QString& pKey, const QStringList& aDefault,
                          char sep=',') const;

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
    QString readPathEntry( const QByteArray &key, const QString & aDefault = QString() ) const;
    QString readPathEntry( const char *key, const QString & aDefault = QString() ) const;

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
    QStringList readPathListEntry( const QByteArray &key, char sep = ',' ) const;
    QStringList readPathListEntry( const char *key, char sep = ',' ) const;

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
    QString readEntryUntranslated( const QByteArray &key,
                                   const QString& aDefault = QString() ) const;
    QString readEntryUntranslated( const char *key,
                                   const QString& aDefault = QString() ) const;

    /**
     * @copydoc writeEntry( const char*, const QString&, WriteConfigFlags )
     */
    template <typename T>
    void writeEntry(const QByteArray &key, const T& value, WriteConfigFlags pFlags = Normal )
        { writeListCheck(key, value, pFlags); }
    template <typename T>
    void writeEntry( const QString& pKey, const T& value, WriteConfigFlags pFlags = Normal )
        { writeEntry( pKey.toUtf8(), value, pFlags ); }
    template <typename T>
    void writeEntry( const char *key, const T& value, WriteConfigFlags pFlags = Normal )
        { writeEntry( QByteArray(key), value, pFlags ); }


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
                     WriteConfigFlags pFlags = Normal );
    void writeEntry( const char *pKey, const QStringList &value,
                     char sep = ',',
                     WriteConfigFlags pFlags = Normal );
    void writeEntry( const QByteArray &key, const QStringList &value,
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
    void writePathEntry( const QString& pKey, const QString & path,
                         WriteConfigFlags pFlags = Normal );
    void writePathEntry( const char *pKey, const QString & path,
                         WriteConfigFlags pFlags = Normal );
    void writePathEntry( const QByteArray &key, const QString & path,
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
    void writePathEntry( const char *pKey, const QStringList &value,
                         char sep = ',', WriteConfigFlags pFlags = Normal );
    void writePathEntry( const QByteArray &key, const QStringList &value,
                         char sep = ',', WriteConfigFlags pFlags = Normal );

    /**
     * Deletes the entry specified by @p pKey in the current group.
     *
     * @param pKey The key to delete.
     * @param pFlags       The flags to use when deleting this entry.
     */
    void deleteEntry( const QString& pKey, WriteConfigFlags pFlags = Normal );
    void deleteEntry( const char *pKey, WriteConfigFlags pFlags = Normal );
    void deleteEntry(const QByteArray &key, WriteConfigFlags flags = Normal);

    /**
     * Checks whether the key has an entry in the currently active group.
     * Use this to determine whether a key is not specified for the current
     * group (hasKey() returns false). Keys with null data are considered
     * nonexistent.
     *
     * @param key The key to search for.
     * @return If true, the key is available.
     */
    bool hasKey(const QByteArray &key) const;
    bool hasKey(const QString &key) const;
    bool hasKey(const char *key) const;


    /**
     * Checks whether it is possible to change the given group.
     * @return whether changes may be made to this group.
     */
    bool isImmutable() const;

    /**
     * Checks whether it is possible to change the given entry.
     * @param key the key to check
     * @return whether the entry @p key may be changed in the current group
     * in this configuration file.
     */
    bool entryIsImmutable(const QString &key) const;
    bool entryIsImmutable(const QByteArray &key) const;
    bool entryIsImmutable(const char *key) const;

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
    void revertToDefault(const char* key);
    void revertToDefault(const QByteArray &key);

    /**
     * Returns whether a default is specified for an entry in either the
     * system wide configuration file or the global KDE config file.
     *
     * If an application computes a default value at runtime for
     * a certain entry, e.g. like:
     * \code
     * QColor computedDefault = qApp->palette().color(QPalette::Active, QPalette::Text)
     * QColor color = config->readEntry(key, computedDefault);
     * \endcode
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
    bool hasDefault(const char* key) const;
    bool hasDefault(const QByteArray &key) const;

    QStringList groupList() const;
    QStringList keyList() const;
    void clean();
    ConfigState getConfigState() const;

    inline void writeEntry(const char *key, const char *value, WriteConfigFlags pFlags = Normal);
    inline void writeEntry(const QByteArray &key, const char *value, WriteConfigFlags pFlags = Normal);
    inline void writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags = Normal);
protected:
    bool hasGroupImpl(const QByteArray &group) const;
    KConfigGroup groupImpl(const QByteArray &b);
    const KConfigGroup groupImpl(const QByteArray &b) const;
    void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags);
    bool groupIsImmutableImpl(const QByteArray& aGroup) const;

private:
    QExplicitlySharedDataPointer<KConfigGroupPrivate> d;

    template<class T>
    inline T readListCheck(const QByteArray &key, const T &defaultValue) const;

    template<class T>
    inline QList<T> readListCheck(const QByteArray &key, const QList<T> &defaultValue) const;

    template<class T>
    inline void writeListCheck(const QByteArray &key, const T &value, WriteConfigFlags pFlags);

    template<class T>
    inline void writeListCheck(const QByteArray &key, const QList<T> &value, WriteConfigFlags pFlags);

    friend class KConfigGroupPrivate;
};

#define KCONFIGGROUP_ENUMERATOR_ERROR(ENUM) \
"The Qt MetaObject system does not seem to know about \"" ENUM \
"\" please use Q_ENUMS or Q_FLAGS to register it."

#define KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)                     \
template <>                                                                \
inline Class::Enum KConfigGroup::readEntry(const QByteArray &key, const Class::Enum& def) const\
{                                                                          \
const QMetaObject* M_obj = &Class::staticMetaObject;                       \
int M_index = M_obj->indexOfEnumerator(#Enum);                             \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Enum) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                       \
const QByteArray str = readEntry(key, QByteArray(M_enum.valueToKey(def))); \
return static_cast<Class::Enum>(M_enum.keyToValue(str.constData()));       \
}                                                                          \
template <>                                                                \
inline void KConfigGroup::writeEntry(const QByteArray &key, const Class::Enum& value, WriteConfigFlags flags)\
{                                                                          \
const QMetaObject* M_obj = &Class::staticMetaObject;                       \
int M_index = M_obj->indexOfEnumerator(#Enum);                             \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Enum) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                       \
writeEntry(key, QByteArray(M_enum.valueToKey(value)), flags);              \
}

#define KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)                    \
template <>                                                                 \
inline Class::Flags KConfigGroup::readEntry(const QByteArray &key, const Class::Flags& def) const\
{                                                                           \
const QMetaObject* M_obj = &Class::staticMetaObject;                        \
int M_index = M_obj->indexOfEnumerator(#Flags);                             \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Flags) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                        \
const QByteArray str = readEntry(key, QByteArray(M_enum.valueToKeys(def))); \
return static_cast<Class::Flags>(M_enum.keysToValue(str.constData()));      \
}                                                                           \
template <>                                                                 \
inline void KConfigGroup::writeEntry(const QByteArray &key, const Class::Flags& value, WriteConfigFlags flags)\
{                                                                           \
const QMetaObject* M_obj = &Class::staticMetaObject;                        \
int M_index = M_obj->indexOfEnumerator(#Flags);                             \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Flags) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                        \
writeEntry(key, QByteArray(M_enum.valueToKeys(value)), flags);              \
}

#include <conversion_check.h>

template<>
KDECORE_EXPORT QVariantList KConfigGroup::readEntry<QVariantList>(const QByteArray &key, const QVariantList &defaultValue) const;
template<>
KDECORE_EXPORT QVariant KConfigGroup::readEntry<QVariant>(const QByteArray &key, const QVariant &defaultValue) const;
template<>
KDECORE_EXPORT QString KConfigGroup::readEntry<QString>(const QByteArray &key, const QString &defaultValue) const;
template<class T>
T KConfigGroup::readListCheck(const QByteArray &key, const T &defaultValue) const
{
    ConversionCheck::to_QVariant<T>();
    return qVariantValue<T>(readEntry(key, QVariant::fromValue(defaultValue)));
}

template <typename T>
QList<T> KConfigGroup::readListCheck(const QByteArray &key, const QList<T> &defaultValue) const
{
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();

  if (!hasKey(key))
    return defaultValue;

  QVariantList vList;

  if (!defaultValue.isEmpty()) {
    Q_FOREACH (const T &aValue, defaultValue)
      vList.append( aValue );
  }
  vList = readEntry( key, vList );

  QList<T> list;
  if (!vList.isEmpty()) {
    Q_FOREACH (const QVariant &aValue, vList) {
      Q_ASSERT(aValue.canConvert<T>());
      list.append( qvariant_cast<T>(aValue) );
    }
  }

  return list;
}

template<>
KDECORE_EXPORT void KConfigGroup::writeEntry<QVariant>(const QByteArray &key, const QVariant &value, KConfigBase::WriteConfigFlags pFlags);
template<>
KDECORE_EXPORT void KConfigGroup::writeEntry<const char *>(const QByteArray &key, const char * const &value, KConfigBase::WriteConfigFlags pFlags);
template<>
KDECORE_EXPORT void KConfigGroup::writeEntry<QString>(const QByteArray &key, const QString &value, KConfigBase::WriteConfigFlags pFlags);
template<>
KDECORE_EXPORT void KConfigGroup::writeEntry<QByteArray>(const QByteArray &key, const QByteArray &value, KConfigBase::WriteConfigFlags pFlags);
template<>
KDECORE_EXPORT void KConfigGroup::writeEntry<QVariantList>(const QByteArray &key, const QVariantList &value, KConfigBase::WriteConfigFlags pFlags);

inline void KConfigGroup::writeEntry(const char *key, const char *value, WriteConfigFlags pFlags)
{ writeEntry<const char *>(QByteArray(key), value, pFlags); }
inline void KConfigGroup::writeEntry(const QByteArray &key, const char *value, WriteConfigFlags pFlags)
{ writeEntry<const char *>(key, value, pFlags); }
inline void KConfigGroup::writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags)
{ writeEntry<const char *>(key.toUtf8(), value, pFlags); }

template <typename T>
void KConfigGroup::writeListCheck( const QByteArray &key, const QList<T>& list,
                              WriteConfigFlags pFlags )
{
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();

  QVariantList vList;
  Q_FOREACH(const T &value, list)
    vList.append(value);

  writeEntry( key, QVariant(vList), pFlags );
}

template <typename T>
inline void KConfigGroup::writeListCheck( const QByteArray &key, const T& value,
                              WriteConfigFlags pFlags )
{
  ConversionCheck::to_QVariant<T>();
  writeEntry( key, qVariantFromValue(value), pFlags );
}

#endif // KCONFIGGROUP_H
