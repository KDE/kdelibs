/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#ifndef KCONFIG_H
#define KCONFIG_H

#include <kconfigbase.h>

#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QByteArray>
#include <QtCore/QList>

class KConfigGroup;
class KComponentData;
class KEntryMap;
class KConfigPrivate;

class KDECORE_EXPORT KConfig : public KConfigBase
{
public:
    enum OpenFlag {
        SimpleConfig     =0x00, ///< no cascading, no globals
        IncludeGlobals =0x01, ///< read globals
        CascadeConfig=0x02, ///< cascade
        DontMergeOnSync=0x04, ///< merge when sync() called
        FullConfig=IncludeGlobals|CascadeConfig ///< cascade, read globals
    };
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    static const OpenFlag OnlyLocal=SimpleConfig;
    static const OpenFlag NoGlobals=CascadeConfig;

    explicit KConfig(const QString& file = QString(), OpenFlags mode = FullConfig,
                     const char* resourceType = "config");

    explicit KConfig(const KComponentData& componentData, const QString& file = QString(),
                     OpenFlags mode = FullConfig, const char* resourceType = "config");

    virtual ~KConfig();

    virtual void sync();

    const KComponentData &componentData() const; // krazy:exclude=constref

    /**
     * Checks whether the config file contains the update @p id
     * as contained in @p updateFile. If not, it runs kconf_update
     * to update the config file.
     *
     * If you install config update files with critical fixes
     * you may wish to use this method to verify that a critical
     * update has indeed been performed to catch the case where
     * a user restores an old config file from backup that has
     * not been updated yet.
     * @param id the update to check
     * @param updateFile the file containing the update
     */
    void checkUpdate(const QString &id, const QString &updateFile);

    /**
     * Cleans all entries in the entry map, so the
     * values will not be written to disk on a later call to
     * sync().
     *
     */
    void clean();

    /**
     * Copies all entries from this config object to a new config
     * object that will save itself to @p file.
     *
     * Actual saving to @p file happens when the returned object is
     * destructed or when sync() is called upon it.
     *
     * @param file the new config object will save itself to.
     * @param config optional config object to reuse
     */
    KConfig* copyTo(const QString &file, KConfig *config=0) const;

    /**
     * Clears all internal data structures and then reread
     * configuration information from persistent storage.
     */
    void reparseConfiguration();

    /// @{ configuration object state
    /**
     * Returns the state of the config object.
     *
     * @see  ConfigState
     * @return the state of the config object
     */
    virtual ConfigState getConfigState() const;

    bool isConfigWritable(bool warnUser);
    /// @}

    /// @{ extra config files
    /**
     * Sets the merge stack to the list of files. The stack is last in first out with
     * the top of the stack being the most specific config file.
     * @param files A list of extra config files containing the full paths of the
     * local config files to set.
     */
    void addConfigSources(const QStringList &sources);
    KDE_DEPRECATED void setExtraConfigFiles(const QStringList &files) { addConfigSources(files); }

    /// @}
    /// @{ locales
    /**
     * Returns the current locale.
     */

    QString locale() const;
    /**
     * Sets the locale to @p aLocale.
     * The global locale is used as default.
     * @note If set to the empty string, @b no locale will be matched. This effectively disables
     * Native Language Support.
     * @return @c true if locale was changed and configuration was reparsed.
     */
    bool setLocale(const QString& aLocale);
    /// @}

    /// @{ defaults
    void setReadDefaults(bool b);
    bool readDefaults() const;
    /// @}

    /// @{ immutability
    bool isImmutable() const;
    /// @}

    /// @{ global
    /**
     * Forces all following write-operations to be performed on @c kdeglobals,
     * independent of the @c Global flag in writeEntry().
     * @param force true to force writing to kdeglobals
     * @see forceGlobal
     */
    KDE_DEPRECATED void setForceGlobal(bool force);
    /**
     * Returns whether all entries are being written to @c kdeglobals.
     * @return @c true if all entries are being written to @c kdeglobals
     * @see setForceGlobal
     */
    KDE_DEPRECATED bool forceGlobal() const;
    /// @}

    /// @{ groups
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
    using KConfigBase::group;

    /**
     * Returns a list of groups that are known.
     * @return a list of of groups
     */
    virtual QStringList groupList() const;

    /**
     * Does this config object contain @p aGroup?
     * @return @c true if aGroup exists.
     */
    /**
     * Delete @p aGroup. This marks @p aGroup as @em deleted in the config object. This effectively
     * removes any cascaded values from config files earlier in the stack.
     */

    /**
     * Can changes be made to the entries in @p aGroup?
     * 
     * @param aGroup The group to check for immutability.
     * @return @c false if the entries in @p aGroup can be modified.
     */

    /// @}

    /// @{ entries

    KDE_DEPRECATED QString readEntryUntranslated(const char* aKey, const QString& aDefault=QString()) const;
    /// @}

    /**
     * Returns the filename passed to the constructor.
     */
    QString name() const;
    /**
     * Returns a map (tree) of entries in a particular group.  Only the actual entry as a string
     * is returned, none of the other internal data is included.
     *
     * @param aGroup The group to get keys from
     * - If @p aGroup is the empty string "", the entries from the @em default group are returned.
     * - If @p aGroup is null, the entries from the current group are returned.
     *
     * @return A map of entries in the group specified, indexed by key.
     *         The returned map may be empty if the group is empty, or not found.
     * @see   QMap
     */
    QMap<QString, QString> entryMap(const QString &aGroup=QString()) const;

protected:
    virtual bool hasGroupImpl(const QByteArray &group) const;
    virtual KConfigGroup groupImpl( const QByteArray &b);
    virtual const KConfigGroup groupImpl(const QByteArray &b) const;
    virtual void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags = Normal);
    virtual bool groupIsImmutableImpl(const QByteArray& aGroup) const;

    friend class KConfigGroup;
    friend class KConfigGroupPrivate;

    /** Virtual hook, used to add new "virtual" functions while maintaining
     * binary compatibility. Unused in this class.
     */
    virtual void virtual_hook( int id, void* data );

    KConfigPrivate *const d_ptr;

    KConfig(KConfigPrivate &d);

private:
    QStringList keyList(const QString& aGroup=QString()) const;

    Q_DISABLE_COPY(KConfig)

    Q_DECLARE_PRIVATE(KConfig)
};
Q_DECLARE_OPERATORS_FOR_FLAGS( KConfig::OpenFlags )

#endif // KCONFIG_H
