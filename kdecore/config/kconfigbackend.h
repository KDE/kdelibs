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
#include <kgenericfactory.h>
#include <ksharedptr.h>

class KEntryMap;
class KComponentData;
class QFile;
class QByteArray;
class QDateTime;

/**
 * \class KConfigBackend kconfigbackend.h <KConfigBackEnd>
 */
class KDECORE_EXPORT KConfigBackend : public QObject, public KShared
{
    Q_OBJECT
    Q_FLAGS(ParseOption)
    Q_FLAGS(WriteOption)

public:
    /** returns a KConfigBackend object to be used with KConfig
     * @param fileName the absolute file name
     * @param system the configuration system to use. if the given system is
     *               not found or an empty string is passed in, then it tries to determine the
     *               correct backend to return.
     */
    static KSharedPtr<KConfigBackend> create(const KComponentData& componentData,
                                             const QString& fileName = QString(),
                                             const QString& system = QString());

    /** registers mappings from directory/file to configuration system
     * @param entryMap the KEntryMap to build the mappings from
     */
    static void registerMappings(const KEntryMap& entryMap);

    /** destroys the backend */
    virtual ~KConfigBackend();

    /** options passed to parseConfig. */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4 /// entries are allowed to be marked as @em expandable
    };
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** options passed to writeConfig. */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig. */
    enum ParseInfo {
        ParseOk, /// object opened read/write
        ParseImmutable, /// object is @em immutable
        ParseOpenError /// there was an error opening object
    };

    /**
     * Read permanent storage.
     * @param pWriteBackMap the KEntryMap where the entries are placed
     * @param options @see ParseOptions
     * @return @see ParseInfo
     */
    virtual ParseInfo parseConfig(const QByteArray& locale,
                                  KEntryMap& pWriteBackMap,
                                  ParseOptions options = ParseOptions()) = 0;

    /**
     * Write the @em dirty entries to permanent storage.
     * @param entryMap the KEntryMap containing the config object's entries.
     * @param options @see WriteOptions
     */
    virtual bool writeConfig(const QByteArray& locale, KEntryMap& entryMap,
                             WriteOptions options, const KComponentData &data) = 0;

    /**
     * is this object writable?
     * @note This function @b MUST be implemented by sub-classes.
     */
    virtual bool isWritable() const = 0;
    /**
     * When isWritable returns false, return an error message to
     * explain to the user why saving configuration will not work.
     */
    virtual QString nonWritableErrorMessage() const = 0;
    /**
     * get the read/write status of the configuration object.
     * @note This function @b MUST be implemented by sub-classes.
     */
    virtual KConfigBase::AccessMode accessMode() const = 0;
    /**
     * create the enclosing object of @em this object.
     * @note This function @b MUST be implemented by sub-classes.
     */
    virtual void createEnclosing() = 0;

    /** Set the file path.
     * @param path the absolute file path.
     * @note @p path @b MUST be @em absolute.
     * @note This function @b MUST be implemented by sub-classes.
     */
    virtual void setFilePath(const QString& path) = 0;

    /**
     * lock the file
     */
    virtual bool lock(const KComponentData& componentData) = 0;
    virtual void unlock() = 0;
    virtual bool isLocked() const = 0;

    /** when was the object last modified?
     * @return the date/time when the object was last modified.
     */
    QDateTime lastModified() const;
    /** The path to the object. */
    QString filePath() const;
    /** The size of the object. */
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
