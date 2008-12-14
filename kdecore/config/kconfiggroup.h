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

#include "kconfigbase.h"

#include <kdecore_export.h>

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

class KConfig;
class KConfigGroupPrivate;
class KSharedConfig;
template <typename T> class KSharedPtr;
typedef KSharedPtr<KSharedConfig> KSharedConfigPtr;

/**
 * \class KConfigGroup kconfiggroup.h <KConfigGroup>
 *
 * A class for one specific group in a KConfig object.
 *
 * If you want to access the top-level entries of a KConfig
 * object, which are not associated with any group, use an
 * empty group name.
 *
 * A KConfigGroup will be read-only if it is constructed from a
 * const config object or from another read-only group.
 */
class KDECORE_EXPORT KConfigGroup : public KConfigBase
{
public:
    /**
     * Constructs an invalid group.
     *
     * \see isValid
     */
    KConfigGroup();

    /**
     * Construct a config group corresponding to @p group in @p master.
     *
     * This allows the creation of subgroups by passing another
     * group as @p master.
     *
     * @p group is the group name encoded in UTF-8.
     */
    KConfigGroup(KConfigBase *master, const QString &group);
    /** @overload "KConfigGroup(KConfigBase*, const QString &)" */
    KConfigGroup(KConfigBase *master, const char *group);

    /**
     * Construct a read-only config group.
     *
     * A read-only group will silently ignore any attempts to write to it.
     *
     * This allows the creation of subgroups by passing an existing group
     * as @p master.
     */
    KConfigGroup(const KConfigBase *master, const QString &group);
    /** @overload "KConfigGroup(const KConfigBase*, const QString &)" */
    KConfigGroup(const KConfigBase *master, const char *group);

    /** @overload "KConfigGroup(const KConfigBase*, const QString &)" */
    KConfigGroup(const KSharedConfigPtr &master, const QString& group);
    /** @overload "KConfigGroup(const KConfigBase*, const QString &)" */
    KConfigGroup(const KSharedConfigPtr &master, const char* group);

    /**
     * Creates a read-only copy of a read-only group.
     */
    KConfigGroup(const KConfigGroup &);
    KConfigGroup &operator=(const KConfigGroup &);

    ~KConfigGroup();

    /**
     * Whether the group is valid.
     *
     * A group is invalid if it was constructed without arguments.
     *
     * You should not call any functions on an invalid group.
     *
     * @return @c true if the group is valid, @c false if it is invalid.
     */
    bool isValid() const;

    /**
     * The name of this group.
     *
     * The root group is named "<default>".
     */
    QString name() const;

    /**
     * Check whether the containing KConfig object acutally contains a
     * group with this name.
     */
    bool exists() const;

    /// @reimp
    void sync();

    /// @reimp
    void markAsClean();

    /// @reimp
    AccessMode accessMode() const;

    /**
     * Return the config object that this group belongs to.
     */
    KConfig* config();
    /**
     * Return the config object that this group belongs to.
     */
    const KConfig* config() const;

    /**
     * @deprecated
     *
     * Changes the group of the object.
     *
     * Create another KConfigGroup from the parent of this group instead.
     */
    KDE_DEPRECATED void changeGroup( const QString &group );
    /** @deprecated
     * @overload "changeGroup(const QString &)"
     */
    KDE_DEPRECATED void changeGroup( const char *group);

    /**
     * Copies the entries in this group to another configuration object.
     *
     * @note @p other can be either another group or a different file.
     *
     * @param other  the configuration object to copy this group's entries to
     * @param pFlags the flags to use when writing the entries to the
     *               other configuration object
     *
     * @since 4.1
     */
    void copyTo(KConfigBase *other, WriteConfigFlags pFlags = Normal) const;

    /**
     * Changes the configuration object that this group belongs to.
     *
     * @note @p other can be another group, the top-level KConfig object or
     * a different KConfig object entirely.
     *
     * If @p parent is already the parent of this group, this method will have
     * no effect.
     *
     * @param parent the config object to place this group under
     * @param pFlags the flags to use in determining which storage source to
     *               write the data to
     *
     * @since 4.1
     */
    void reparent(KConfigBase* parent, WriteConfigFlags pFlags = Normal);

    /**
     * Returns the group that this group belongs to.
     *
     * @returns the parent group, or an invalid group if this is a top-level
     *          group
     *
     * @since 4.1
     */
    KConfigGroup parent() const;

    /**
     * @reimp
     */
    QStringList groupList() const;

    /**
     * Returns a list of keys this group contains.
     */
    QStringList keyList() const;

    /**
     * Delete all entries in the entire group
     * @param pFlags flags passed to KConfig::deleteGroup
     */
    void deleteGroup(WriteConfigFlags pFlags=Normal);
    using KConfigBase::deleteGroup;

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
        inline T readEntry(const QString& key, const T& aDefault) const
            { return readCheck(key.toUtf8().constData(), aDefault); }
    /** @overload "readEntry(const QString& const T&) const" */
    template <typename T>
        inline T readEntry(const char *key, const T& aDefault) const
            { return readCheck(key, aDefault); }

    /**
     * Reads the value of an entry specified by @p key in the current group.
     *
     * @param key The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key. Can be QVariant() if aDefault is null.
     */
    QVariant readEntry(const QString& key, const QVariant &aDefault) const;
    /** @overload "readEntry(char*, const QVariant &) const" */
    QVariant readEntry(const char* key, const QVariant &aDefault) const;

    /**
     * Reads the string value of an entry specified by @p key in the current group.
     *
     * If you want to read a path, please use readPathEntry().
     *
     * @param key The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key. Can be QString() if aDefault is null.
     */
    QString readEntry(const QString& key, const QString& aDefault) const;
    /** @overload "readEntry(char*, const QString &) const" */
    QString readEntry(const char* key, const QString& aDefault ) const;

    /** @overload "readEntry(char*, const QString &) const" */
    QString readEntry(const QString &key, const char * aDefault = 0) const;
    /** @overload "readEntry(char*, const QString &) const" */
    QString readEntry(const char *key, const char *aDefault = 0 ) const;

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
    QVariantList readEntry( const QString &key, const QVariantList& aDefault ) const;
    QVariantList readEntry( const char* key, const QVariantList& aDefault ) const;

    /**
     * Reads a list of strings from the config object.
     * @param key The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @return The list. Contains @p aDefault if @p key does not exist.
     */
    QStringList readEntry(const QString &key, const QStringList& aDefault) const;
    QStringList readEntry(const char* key, const QStringList& aDefault) const;

    /**
     * Reads a list of values from the config object.
     * @param key The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @return The list. Contains @p aDefault if @p key does not exist.
     */
    template<typename T>
        inline QList<T> readEntry(const QString& key, const QList<T> &aDefault) const
            { return readListCheck(key.toUtf8().constData(), aDefault); }
    template<typename T>
        inline QList<T> readEntry(const char* key, const QList<T> &aDefault) const
            { return readListCheck(key, aDefault); }

    /**
     * Reads a list of strings from the config object, following XDG
     * desktop entry spec separator semantics.
     * @param pKey The key to search for.
     * @param aDefault The default value to use if the key does not exist.
     * @return The list. Contains @p aDefault if @p pKey does not exist.
     */
    QStringList readXdgListEntry(const QString& pKey, const QStringList& aDefault = QStringList()) const;
    QStringList readXdgListEntry(const char* pKey, const QStringList& aDefault = QStringList()) const;

    /**
     * Reads a path.
     *
     * Read the value of an entry specified by @p pKey in the current group
     * and interpret it as a path. This means, dollar expansion is activated
     * for this value, so that e.g. $HOME gets expanded.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The value for this key. Can be QString() if @p aDefault is null.
     */
    QString readPathEntry( const QString& pKey, const QString & aDefault ) const;
    QString readPathEntry( const char *key, const QString & aDefault ) const;

    /**
     * Reads a list of string paths.
     *
     * Read the value of an entry specified by @p pKey in the current group
     * and interpret it as a list of paths. This means, dollar expansion is activated
     * for this value, so that e.g. $HOME gets expanded.
     *
     * @param pKey The key to search for.
     * @param aDefault A default value returned if the key was not found.
     * @return The list. Empty if the entry does not exist.
     */
    QStringList readPathEntry( const QString& pKey, const QStringList& aDefault ) const;
    QStringList readPathEntry( const char *key, const QStringList& aDefault ) const;

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
    QString readEntryUntranslated( const char *key,
                                   const QString& aDefault = QString() ) const;

    /**
     * Writes a value to the config object.
     *
     * @param key The key to write
     * @param value The value to write
     * @param pFlags The flags to use when writing this entry.
     */
    void writeEntry( const QString& key, const QVariant& value,
                     WriteConfigFlags pFlags = Normal );
    void writeEntry( const char *key, const QVariant& value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * Writes a value to the config object.
     *
     * @param key The key to write
     * @param value The value to write
     * @param pFlags The flags to use when writing this entry.
     */
    void writeEntry( const QString& key, const QString& value,
                     WriteConfigFlags pFlags = Normal );
    void writeEntry( const char *key, const QString& value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * Writes a value to the config object.
     *
     * @param key The key to write
     * @param value The value to write
     * @param pFlags The flags to use when writing this entry.
     */
    void writeEntry( const QString& key, const QByteArray& value,
                     WriteConfigFlags pFlags = Normal );
    void writeEntry( const char *key, const QByteArray& value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * Writes a value to the config object.
     *
     * @param key The key to write
     * @param value The value to write
     * @param pFlags The flags to use when writing this entry.
     */
    void writeEntry(const QString &key, const char *value, WriteConfigFlags pFlags = Normal);
    void writeEntry(const char *key, const char *value, WriteConfigFlags pFlags = Normal);

    /**
     * Writes a value to the config object.
     *
     * @param key The key to write
     * @param value The value to write
     * @param pFlags The flags to use when writing this entry.
     */
    template <typename T>
        inline void writeEntry( const char *key, const T& value, WriteConfigFlags pFlags = Normal )
            { writeCheck( key, value, pFlags ); }

    template <typename T>
        inline void writeEntry( const QString& key, const T& value, WriteConfigFlags pFlags = Normal )
            { writeCheck( key.toUtf8().constData(), value, pFlags ); }

    /**
     * writeEntry() overridden to accept a list of strings.
     *
     * @param key The key to write
     * @param value The list to write
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  writeEntry()
     */
    void writeEntry( const QString& key, const QStringList &value,
                     WriteConfigFlags pFlags = Normal );

    void writeEntry( const char* key, const QStringList &value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * writeEntry() overridden to accept a list of QVariant values.
     *
     * @param key The key to write
     * @param value The list to write
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  writeEntry()
     */
    void writeEntry( const QString& key, const QVariantList &value,
                     WriteConfigFlags pFlags = Normal );

    void writeEntry( const char* key, const QVariantList &value,
                     WriteConfigFlags pFlags = Normal );

    /**
     * Writes a list to the config object.
     *
     * @param key The key to write
     * @param value The list to write
     * @param pFlags The flags to use when writing this entry.
     */
    template <typename T>
        inline void writeEntry(const QString& key, const QList<T> &value, WriteConfigFlags pFlags = Normal)
            { writeListCheck( key.toUtf8().constData(), value, pFlags ); }

    template <typename T>
        inline void writeEntry(const char* key, const QList<T> &value, WriteConfigFlags pFlags = Normal)
            { writeListCheck( key, value, pFlags ); }

    /**
     * Writes a list of strings to the config object, following XDG
     * desktop entry spec separator semantics.
     *
     * @param pKey The key to write
     * @param value The list to write
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  writeEntry()
     */
    void writeXdgListEntry( const QString& pKey, const QStringList &value,
                            WriteConfigFlags pFlags = Normal );
    void writeXdgListEntry( const char *pKey, const QStringList &value,
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

    /**
     * writePathEntry() overridden to accept a list of paths (strings).
     *
     * It is checked whether the paths are located under $HOME. If so each of
     * the paths are written out with the user's home-directory replaced with
     * $HOME. The paths should be read back with readPathEntry()
     *
     * @param pKey The key to write
     * @param value The list to write
     * @param pFlags       The flags to use when writing this entry.
     *
     * @see  readPathEntry()
     */
    void writePathEntry( const QString& pKey, const QStringList &value,
                         WriteConfigFlags pFlags = Normal );
    void writePathEntry( const char *pKey, const QStringList &value,
                         WriteConfigFlags pFlags = Normal );

    /**
     * Deletes the entry specified by @p pKey in the current group.
     * This also hides system wide defaults.
     *
     * @param pKey The key to delete.
     * @param pFlags       The flags to use when deleting this entry.
     */
    void deleteEntry(const QString& pKey, WriteConfigFlags pFlags = Normal);
    void deleteEntry(const char *pKey, WriteConfigFlags pFlags = Normal);

    /**
     * Checks whether the key has an entry in the currently active group.
     * Use this to determine whether a key is not specified for the current
     * group (hasKey() returns false).
     *
     * @param key The key to search for.
     * @return If true, the key is available.
     */
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
    bool isEntryImmutable(const QString &key) const;
    bool isEntryImmutable(const char *key) const;

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

    /**
     * Returns a map (tree) of entries for all entries in this group.
     *
     * Only the actual entry string is returned, none of the
     * other internal data should be included.
     *
     * @return A map of entries in this group, indexed by key.
     */
    QMap<QString, QString> entryMap() const;

protected:
    bool hasGroupImpl(const QByteArray &group) const;
    KConfigGroup groupImpl(const QByteArray &b);
    const KConfigGroup groupImpl(const QByteArray &b) const;
    void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags);
    bool isGroupImmutableImpl(const QByteArray& aGroup) const;

private:
    QExplicitlySharedDataPointer<KConfigGroupPrivate> d;

    template<typename T>
    inline T readCheck(const char* key, const T &defaultValue) const;

    template<typename T>
    inline QList<T> readListCheck(const char* key, const QList<T> &defaultValue) const;

    template<typename T>
    inline void writeCheck(const char* key, const T &value, WriteConfigFlags pFlags);

    template<typename T>
    inline void writeListCheck(const char* key, const QList<T> &value, WriteConfigFlags pFlags);

    friend class KConfigGroupPrivate;

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
    friend class KServicePrivate; // XXX yeah, ugly^5
};

#define KCONFIGGROUP_ENUMERATOR_ERROR(ENUM) \
"The Qt MetaObject system does not seem to know about \"" ENUM \
"\" please use Q_ENUMS or Q_FLAGS to register it."

#define KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Class, Enum)                     \
inline Class::Enum readEntry(const KConfigGroup& group, const char* key, const Class::Enum& def) \
{                                                                          \
const QMetaObject* M_obj = &Class::staticMetaObject;                       \
const int M_index = M_obj->indexOfEnumerator(#Enum);                       \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Enum) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                       \
const QByteArray M_data = group.readEntry(key, QByteArray(M_enum.valueToKey(def)));\
return static_cast<Class::Enum>(M_enum.keyToValue(M_data.constData()));    \
}                                                                          \
inline void writeEntry(KConfigGroup& group, const char* key, const Class::Enum& value, KConfigBase::WriteConfigFlags flags = KConfigBase::Normal)\
{                                                                          \
const QMetaObject* M_obj = &Class::staticMetaObject;                       \
const int M_index = M_obj->indexOfEnumerator(#Enum);                       \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Enum) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                       \
group.writeEntry(key, QByteArray(M_enum.valueToKey(value)), flags);              \
}

#define KCONFIGGROUP_DECLARE_FLAGS_QOBJECT(Class, Flags)                    \
inline Class::Flags readEntry(const KConfigGroup& group, const char* key, const Class::Flags& def) \
{                                                                           \
const QMetaObject* M_obj = &Class::staticMetaObject;                        \
const int M_index = M_obj->indexOfEnumerator(#Flags);                       \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Flags) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                        \
const QByteArray M_data = group.readEntry(key, QByteArray(M_enum.valueToKeys(def)));\
return static_cast<Class::Flags>(M_enum.keysToValue(M_data.constData()));   \
}                                                                           \
inline void writeEntry(KConfigGroup& group, const char* key, const Class::Flags& value, KConfigBase::WriteConfigFlags flags = KConfigBase::Normal)\
{                                                                           \
const QMetaObject* M_obj = &Class::staticMetaObject;                        \
const int M_index = M_obj->indexOfEnumerator(#Flags);                       \
kFatal(M_index == -1) << KCONFIGGROUP_ENUMERATOR_ERROR(#Flags) << endl;     \
const QMetaEnum M_enum = M_obj->enumerator(M_index);                        \
group.writeEntry(key, QByteArray(M_enum.valueToKeys(value)), flags);              \
}

#include "conversion_check.h"

template <typename T>
T KConfigGroup::readCheck(const char* key, const T &defaultValue) const
{
  ConversionCheck::to_QVariant<T>();
  return qvariant_cast<T>(readEntry(key, qVariantFromValue(defaultValue)));
}

template <typename T>
QList<T> KConfigGroup::readListCheck(const char* key, const QList<T> &defaultValue) const
{
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();

  QVariantList data;

  Q_FOREACH(const T& value, defaultValue)
    data.append(qVariantFromValue(value));

  QList<T> list;
  Q_FOREACH (const QVariant &value, readEntry<QVariantList>(key, data)) {
    Q_ASSERT(qVariantCanConvert<T>(value));
    list.append( qvariant_cast<T>(value) );
  }

  return list;
}

template <typename T>
void KConfigGroup::writeCheck( const char* key, const T& value,
                               WriteConfigFlags pFlags )
{
    ConversionCheck::to_QVariant<T>();
    writeEntry( key, qVariantFromValue(value), pFlags );
}

template <typename T>
void KConfigGroup::writeListCheck( const char* key, const QList<T>& list,
                                   WriteConfigFlags pFlags )
{
  ConversionCheck::to_QVariant<T>();
  ConversionCheck::to_QString<T>();
  QVariantList data;
  Q_FOREACH(const T &value, list)
    data.append(qVariantFromValue(value));

  writeEntry( key, data, pFlags );
}

#endif // KCONFIGGROUP_H
