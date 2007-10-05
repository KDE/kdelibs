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

#ifndef KCONFIGBASE_H
#define KCONFIGBASE_H

#include <kdecore_export.h>

#include <QtCore/QtGlobal>

class QStringList;
class KConfigGroup;

class KDECORE_EXPORT KConfigBase
{
public:
    /**
     * Flags to control write entry
     */
    enum WriteConfigFlag
    {
        Persistent = 0x01,
        /**<
         * Save this entry when saving the config object.
         */
        Global = 0x02,
        /**<
         * Save the entry to the global %KDE config file instead of the
         * application specific config file.
         */
        Localized = 0x04,
        NLS = Localized,
        /**<
         * Add the locale tag to the key when writing it.
         */
        Normal=Persistent
        /**<
         * Save the entry to the application specific config file without
         * a locale tag. This is the default.
         */

    };
    Q_DECLARE_FLAGS(WriteConfigFlags, WriteConfigFlag)


    /**
     * Destructs the KConfigBase object.
     */
    virtual ~KConfigBase();

    /**
     * Returns true if the specified group is known about.
     *
     * @param group The group to search for.
     * @return true if the group exists.
     */
    bool hasGroup(const QString &group) const;
    bool hasGroup(const char *group) const;
    bool hasGroup(const QByteArray &group) const;

  /**
   * Returns a list of groups that are known about.
   *
   * @return The list of groups.
   **/
    virtual QStringList groupList() const = 0;

    KConfigGroup group( const QByteArray &b);
    KConfigGroup group( const QString &str);
    KConfigGroup group( const char *str);

    const KConfigGroup group( const QByteArray &b ) const;
    const KConfigGroup group( const QString &s ) const;
    const KConfigGroup group( const char *s ) const;


  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If @p bDeep is false, only the global dirty
   * flag of the KConfig object gets cleared. If you then call
   * writeEntry() again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent sync() call.
   *
   */
    virtual void clean() = 0;

    KDE_DEPRECATED void rollback() { clean(); }

  /**
   * Checks whether this configuration object can be modified.
   * @return whether changes may be made to this configuration object.
   */
    virtual bool isImmutable() const = 0;

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
    virtual ConfigState getConfigState() const = 0;

    void deleteGroup(const QByteArray &group, WriteConfigFlags flags = Normal);
    void deleteGroup(const QString &group, WriteConfigFlags flags = Normal);
    void deleteGroup(const char *group, WriteConfigFlags flags = Normal);

    bool groupIsImmutable(const QByteArray& aGroup) const;
    bool groupIsImmutable(const QString& aGroup) const;
    bool groupIsImmutable(const char *aGroup) const;

    virtual void sync() = 0;

protected:
    KConfigBase();

    virtual bool hasGroupImpl(const QByteArray &group) const = 0;
    virtual KConfigGroup groupImpl( const QByteArray &b) = 0;
    virtual const KConfigGroup groupImpl(const QByteArray &b) const = 0;
    virtual void deleteGroupImpl(const QByteArray &group, WriteConfigFlags flags = Normal) = 0;
    virtual bool groupIsImmutableImpl(const QByteArray& aGroup) const = 0;

    /** Virtual hook, used to add new "virtual" functions while maintaining
     * binary compatibility. Unused in this class.
     */
    virtual void virtual_hook( int id, void* data );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBase::WriteConfigFlags)



#endif // KCONFIG_H
