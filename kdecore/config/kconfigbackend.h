/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Portions copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#ifndef KCONFIGBACKEND_H
#define KCONFIGBACKEND_H

#include <QtCore/QObject>
#include <QtCore/QString>

#include <kdecore_export.h>
#include <kconfigbase.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <ksharedptr.h>

class KEntryMap;
class KComponentData;
class QFile;
class QByteArray;
class QDateTime;

/**
 * \class KConfigBackend kconfigbackend.h <KConfigBackend>
 *
 * Provides the implementation for accessing configuration sources.
 *
 * KDELibs only provides an INI backend, but this class can be used
 * to create plugins that allow access to other file formats and
 * configuration systems.
 */
class KDECORE_EXPORT KConfigBackend : public QObject, public KShared
{
    Q_OBJECT
    Q_FLAGS(ParseOption)
    Q_FLAGS(WriteOption)

public:
    /**
     * Creates a new KConfig backend.
     *
     * If no @p system is given, or the given @p system is unknown, this method tries
     * to determine the correct backend to use.
     *
     * @param componentData the owning component
     * @param fileName      the absolute file name of the configuration file
     * @param system        the configuration system to use
     * @return a KConfigBackend object to be used with KConfig
     */
    static KSharedPtr<KConfigBackend> create(const KComponentData& componentData,
                                             const QString& fileName = QString(),
                                             const QString& system = QString());

    /**
     * Registers mappings from directories/files to configuration systems
     *
     * Allows you to tell KConfigBackend that create() should use a particular
     * backend for a particular file or directory.
     *
     * @warning currently does nothing
     *
     * @param entryMap the KEntryMap to build the mappings from
     */
    static void registerMappings(const KEntryMap& entryMap);

    /** Destroys the backend */
    virtual ~KConfigBackend();

    /** Allows the behaviour of parseConfig() to be tuned */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4 /// entries are allowed to be marked as @em expandable
    };
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** Allows the behaviour of writeConfig() to be tuned */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig() */
    enum ParseInfo {
        ParseOk, /// the configuration was opened read/write
        ParseImmutable, /// the configuration is @em immutable
        ParseOpenError /// the configuration could not be opened
    };

    /**
     * Read persistent storage
     *
     * @param locale the locale to read entries for (if the backend supports localized entries)
     * @param pWriteBackMap the KEntryMap where the entries are placed
     * @param options @see ParseOptions
     * @return @see ParseInfo
     */
    virtual ParseInfo parseConfig(const QByteArray& locale,
                                  KEntryMap& pWriteBackMap,
                                  ParseOptions options = ParseOptions()) = 0;

    /**
     * Write the @em dirty entries to permanent storage
     *
     * @param locale the locale to write entries for (if the backend supports localized entries)
     * @param entryMap the KEntryMap containing the config object's entries.
     * @param options @see WriteOptions
     * @param data the component that requested the write
     *
     * @return @c true if the write was successful, @c false if writing the configuration failed
     */
    virtual bool writeConfig(const QByteArray& locale, KEntryMap& entryMap,
                             WriteOptions options, const KComponentData &data) = 0;

    /**
     * If isWritable() returns false, writeConfig() will always fail.
     *
     * @return @c true if the configuration is writable, @c false if it is immutable
     */
    virtual bool isWritable() const = 0;
    /**
     * When isWritable() returns @c false, return an error message to
     * explain to the user why saving configuration will not work.
     *
     * The return value when isWritable() returns @c true is undefined.
     *
     * @returns a translated user-visible explanation for the configuration
     *          object not being writable
     */
    virtual QString nonWritableErrorMessage() const = 0;
    /**
     * @return the read/write status of the configuration object
     *
     * @see KConfigBase::AccessMode
     */
    virtual KConfigBase::AccessMode accessMode() const = 0;
    /**
     * Create the enclosing object of the configuration object
     *
     * For example, if the configuration object is a file, this should create
     * the parent directory.
     */
    virtual void createEnclosing() = 0;

    /**
     * Set the file path.
     *
     * @note @p path @b MUST be @em absolute.
     *
     * @param path the absolute file path
     */
    virtual void setFilePath(const QString& path) = 0;

    /**
     * Lock the file
     */
    virtual bool lock(const KComponentData& componentData) = 0;
    /**
     * Release the lock on the file
     */
    virtual void unlock() = 0;
    /**
     * @return @c true if the file is locked, @c false if it is not locked
     */
    virtual bool isLocked() const = 0;

    /**
     * @return the date and time when the object was last modified
     */
    QDateTime lastModified() const;
    /** @return the absolute path to the object */
    QString filePath() const;
    /** @return the size of the object */
    qint64 size() const;

protected:
    KConfigBackend();
    void setLastModified(const QDateTime& dt);
    void setSize(qint64 sz);
    void setLocalFilePath(const QString& file);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBackend::ParseOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigBackend::WriteOptions)

/**
 * Register a KConfig backend when it is contained in a loadable module
 */
#define K_EXPORT_KCONFIGBACKEND(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("kconfigbackend_" #libname))


#endif // KCONFIGBACKEND_H
