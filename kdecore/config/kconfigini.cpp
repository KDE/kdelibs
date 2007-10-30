/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>

#include "kconfig.h"
#include "kconfigbackend.h"
#include "kconfigini_p.h"
#include "kconfigdata.h"
#include <ksavefile.h>
#include <kde_file.h>
#include "kstandarddirs.h"
#include <kdebug.h>

#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdebug.h>
#include <qmetaobject.h>
#include <qregexp.h>

extern bool kde_kiosk_exception;

QString KConfigIniBackend::warningProlog(const QFile &file, int line)
{
    return QString("KConfigIni: In file %2, line %1: ")
            .arg(line).arg(file.fileName());
}

KConfigIniBackend::KConfigIniBackend()
 : KConfigBackend()
{
}

KConfigIniBackend::~KConfigIniBackend()
{
}


KConfigBackend::ParseInfo
KConfigIniBackend::parseConfig(const QByteArray& currentLocale, KEntryMap& entryMap,
                               ParseOptions options)
{
    if (filePath().isEmpty() || !QFile::exists(filePath()))
        return ParseInfo();

    bool bDefault = options&ParseDefaults;
    bool allowExecutableValues = options&ParseExpansions;

    QByteArray currentGroup("<default>");

    QFile file(filePath());
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return ParseOpenError;

    bool fileOptionImmutable = false;
    bool groupOptionImmutable = false;
    bool groupSkip = false;

    int lineNo=0;
    while (!file.atEnd()) {
        QByteArray line = file.readLine().trimmed();
        lineNo++;

        // skip empty lines and lines beginning with '#'
        if (line.isEmpty() || line.at(0) == '#')
            continue;

        if (line.at(0) == '[') { // found a group
            int end = 0;
            int opens=1;
            do { // match brackets
                end++;
                if (line.at(end) == '[')
                    opens++;
                else if (line.at(end) == ']')
                    opens--;
            } while (end < line.length() && opens > 0);

            if (end == line.length() && opens > 0) {
                kWarning() << warningProlog(file, lineNo) << "Invalid group header.";
                continue;
            } else if (end == 3 && line.at(1) == '$' && line.at(2) == 'i') {
                fileOptionImmutable = !kde_kiosk_exception;
                continue;
            }
            currentGroup = printableToString(line.mid(1,end-1), file, lineNo);
            line = line.remove(0, end+1);

            groupOptionImmutable = fileOptionImmutable;
            if (!line.isEmpty() && line.at(0) == '[' && line.at(1) == '$') { // group option follows
                if (line.at(2) == 'i')
                    groupOptionImmutable = !kde_kiosk_exception;
            }

            groupSkip = entryMap.getEntryOption(currentGroup, 0, 0, KEntryMap::EntryImmutable);

            if (groupSkip && !bDefault)
                continue;

            KEntryMap::EntryOptions opts=0;
            if (groupOptionImmutable)
                opts = KEntryMap::EntryImmutable;
            entryMap.setEntry(currentGroup, 0, QByteArray(), opts);
        } else {
            if (groupSkip && !bDefault)
                continue; // skip entry

            int end = line.indexOf('=');

            if (end == -1 && !line.contains("[$d]")) {
                // no equals sign
                kWarning() << warningProlog(file, lineNo) << "Invalid entry (missing '=')" << endl;
                continue;
            } else if (end == 1) {
                // empty key
                kWarning() << warningProlog(file, lineNo) << "Invalid entry (empty key)" << endl;
                continue;
            }
            QByteArray aKey = line.left(end);
            line = line.mid(end+1);
            if (end != 1 && aKey.length() == 0) {
                aKey = line;
                line = "";
            }

            KEntryMap::EntryOptions entryOptions=0;
            if (groupOptionImmutable)
                entryOptions |= KEntryMap::EntryImmutable;

            QByteArray locale;
            while (aKey.contains('[')) {
                int start = aKey.indexOf('[');
                end = aKey.indexOf(']', start);
                if (start != -1 && end == -1) {
                    kWarning() << warningProlog(file, lineNo)
                            << "Invalid entry (missing ']')" << endl;
                    continue;
                } else if (end < start) {
                    kWarning() << warningProlog(file, lineNo)
                            << "Invalid entry (unmatched ']')" << endl;
                    continue;
                }else if (aKey.at(start+1) == '$') { // found option(s)
                    int i = start+2;
                    while (i < end) {
                        switch (aKey.at(i)) {
                            case 'i':
                                if (!kde_kiosk_exception)
                                    entryOptions |= KEntryMap::EntryImmutable;
                                break;
                            case 'e':
                                if (allowExecutableValues)
                                    entryOptions |= KEntryMap::EntryExpansion;
                                break;
                            case 'd':
                                entryOptions |= KEntryMap::EntryDeleted;
                                aKey = printableToString(aKey.left(start), file, lineNo);
                                entryMap.setEntry(currentGroup, aKey, QByteArray(), entryOptions);
                                goto next_line;
                            default:
                                break;
                        }
                        i++;
                    }
                } else { // found a locale
                    if (!locale.isNull()) {
                        kWarning() << warningProlog(file, lineNo)
                                << "Invalid entry (second locale!?)" << endl;
                        continue;
                    }

                    locale = aKey.mid(start+1,end-start-1);
                }
                aKey = aKey.remove(start, end-start+1);
            }

            aKey = printableToString(aKey, file, lineNo);
            if (!locale.isEmpty()) {
                if (locale != currentLocale) {
                    // backward compatibility. C == en_US
                    if (locale.at(0) != 'C' || currentLocale != "en_US")
                        goto next_line;
                }
            }

            if (options&ParseGlobal)
                entryOptions |= KEntryMap::EntryGlobal;
            if (bDefault)
                entryOptions |= KEntryMap::EntryDefault;
            if (!locale.isNull())
                entryOptions |= KEntryMap::EntryLocalized;
            entryMap.setEntry(currentGroup, aKey, printableToString(line, file, lineNo), entryOptions);
        }
next_line:
        continue;
    }

    return (fileOptionImmutable? ParseImmutable: ParseInfo());
}

void KConfigIniBackend::writeEntries(const QByteArray& locale, QFile& file,
                                     const KEntryMap& map, bool defaultGroup, bool &firstEntry)
{
    QByteArray currentGroup;
    const KEntryMapConstIterator end = map.constEnd();
    for (KEntryMapConstIterator it = map.constBegin(); it != end; ++it) {
        const KEntryKey& key = it.key();

        // Either process the default group or all others
        if ((key.mGroup != "<default>") == defaultGroup)
            continue; // skip

        // Skip group headers
        if (key.mKey.isNull())
            continue; // skip

        const KEntry& currentEntry = *it;
        if (!defaultGroup && currentGroup != key.mGroup) {
            if (!firstEntry)
                file.putChar('\n');
            currentGroup = key.mGroup;
            file.putChar('[');
            file.write(stringToPrintable(currentGroup));
            file.write("]\n", 2);
        }

        firstEntry = false;
        // it is data for a group

        file.write(stringToPrintable(key.mKey)); // Key

        if (currentEntry.bNLS && locale != "C") { // locale 'C' == untranslated
            file.putChar('[');
            file.write(locale); // Locale tag
            file.putChar(']');
        }
        if (currentEntry.bDeleted) {
            if (currentEntry.bImmutable)
                file.write("[$di]", 5); // Deleted + immutable
            else
                file.write("[$d]", 4); // Deleted
        } else {
            if (currentEntry.bImmutable || currentEntry.bExpand) {
                file.write("[$", 2);
                if (currentEntry.bImmutable)
                    file.putChar('i');
                if (currentEntry.bExpand)
                    file.putChar('e');
                file.putChar(']');
            }
            file.putChar('=');
            file.write(stringToPrintable(currentEntry.mValue, ValueString));
        }
        file.putChar('\n');
    }
}

void KConfigIniBackend::writeEntries(const QByteArray& locale, QFile& file, const KEntryMap& map)
{
    bool firstEntry = true;

    // write default group
    writeEntries(locale, file, map, true, firstEntry);

    // write all other groups
    writeEntries(locale, file, map, false, firstEntry);
}

bool KConfigIniBackend::writeConfig(const QByteArray& locale, KEntryMap& entryMap,
                                    const KEntryMap& toMerge, WriteOptions options, const KComponentData &data)
{
    Q_ASSERT(!filePath().isEmpty());

    KEntryMap writeMap = toMerge;
    bool bGlobal = options & WriteGlobal;

    const KEntryMapIterator end = entryMap.end();
    for (KEntryMapIterator it=entryMap.begin(); it != end; ++it) {
        if (!it->bDirty) // not dirty, doesn't overwrite entry in writeMap. skips default entries, too.
            continue;

        const KEntryKey& key = it.key();

        // only write entries that have the same "globality" as the file
        if (it->bGlobal == bGlobal) {
            if (!it->bDeleted) {
                writeMap[key] = *it;
            } else {
                KEntryKey defaultKey = key;
                defaultKey.bDefault = true;
                if (!entryMap.contains(defaultKey))
                    writeMap.remove(key); // remove the deleted entry if there is no default
                else
                    writeMap[key] = *it; // otherwise write an explicitly deleted entry
            }
            it->bDirty = false;
        }
    }

    // now writeMap should contain only entries to be written
    // so write it out to disk

    // check if file exists
    int fileMode = -1;
    bool createNew = true;

    KDE_struct_stat buf;
    if (KDE_stat(QFile::encodeName(filePath()), &buf) == 0)
    {
        if (buf.st_uid == ::getuid() || buf.st_uid == uid_t(-2))
        {
            // Preserve file mode if file exists and is owned by user.
            fileMode = buf.st_mode & 0777;
        }
        else
        {
            // File is not owned by user:
            // Don't create new file but write to existing file instead.
            createNew = false;
        }
    }

    KSaveFile file( filePath(), data );
    if (!file.open())
      return false;

    file.setTextModeEnabled(true); // to get eol translation

    if (createNew)
        file.setPermissions(QFile::ReadUser|QFile::WriteUser);

    writeEntries(locale, file, writeMap);

    if ( !file.size() && ((fileMode == -1) || (fileMode == 0600)) ) {
        // File is empty and doesn't have special permissions: delete it.
        file.abort();
        // and the original file.
        if (QFile::exists(filePath()))
            QFile::remove(filePath());
    }

    return file.finalize();
}

bool KConfigIniBackend::isWritable() const
{
    if (!filePath().isEmpty())
        return KStandardDirs::checkAccess(filePath(), W_OK);
    return false;
}

void KConfigIniBackend::createEnclosing()
{
    const QString file = filePath();
    if (file.isEmpty())
        return; // nothing to do

    // Create the containing dir, maybe it wasn't there
    QDir dir;
    dir.mkpath(QFileInfo(file).canonicalPath());
}

void KConfigIniBackend::setFilePath(const QString& file)
{
    if (file.isEmpty())
        return;

    Q_ASSERT(QDir::isAbsolutePath(file));

    if (QFile::exists(file)) {
        const QFileInfo info(file);
        setLocalFilePath(info.canonicalFilePath());
        setLastModified(info.lastModified());
        setSize(info.size());
    } else {
        setLocalFilePath(file);
        setSize(0);
        QDateTime dummy;
        dummy.setTime_t(0);
        setLastModified(dummy);
    }
}

KConfigBase::AccessMode KConfigIniBackend::accessMode() const
{
    if (filePath().isEmpty())
        return KConfigBase::NoAccess;

    if (KStandardDirs::checkAccess(filePath(), W_OK))
        return KConfigBase::ReadWrite;

    return KConfigBase::ReadOnly;
}

bool KConfigIniBackend::lock(const KComponentData& componentData)
{
    Q_ASSERT(!filePath().isEmpty());

    lockFile = new KLockFile(filePath() + QLatin1String(".lock"), componentData);

    if (lockFile->lock() == KLockFile::LockStale) // attempt to break the lock
        lockFile->lock(KLockFile::ForceFlag);
    return lockFile->isLocked();
    return true;
}

void KConfigIniBackend::unlock()
{
    lockFile->unlock();
    lockFile.clear();
}

bool KConfigIniBackend::isLocked() const
{
    return lockFile && lockFile->isLocked();
}

QByteArray KConfigIniBackend::stringToPrintable(const QByteArray& aString, StringType type)
{
    static const char nibbleLookup[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    if (aString.isEmpty())
        return aString;
    const int l = aString.length();

    QByteArray result; // Guesstimated that it's good to avoid data() initialization for a length of l*4
    result.reserve(l * 4); // Maximum 4x as long as source string due to \x<ab> escape sequences
    register const char *s = aString.constData();
    int i = 0;

    // Protect leading space
    if (s[0] == ' ') {
        result.append("\\s");
        i++;
    }

    for (; i < l; i++/*, r++*/) {
        switch (s[i]) {
            default:
            // The \n, \t, \r cases (all < 32) are handled below; we can ignore them here
                if (((unsigned char)s[i]) < 32)
                    goto doEscape;
                result.append(s[i]);
                break;
            case '\n':
                result.append("\\n");
                break;
            case '\t':
                result.append("\\t");
                break;
            case '\r':
                result.append("\\r");
                break;
            case '\\':
                result.append("\\\\");
                break;
            case '[':
            case ']':
            case '=':
            // Above chars are OK to put in *value* strings as plaintext
                if (type == ValueString) {
                    result.append(s[i]);
                    break;
                }
        doEscape:
                result.append("\\x")
                    .append(nibbleLookup[((unsigned char)s[i]) >> 4])
                    .append(nibbleLookup[((unsigned char)s[i]) & 0x0f]);
                break;
        }
    }

    // Protect trailing space
    if (result.endsWith(' ')) {
        result.replace(result.lastIndexOf(' '), 1, "\\s");
    }
    result.squeeze();

    return result;
}

char KConfigIniBackend::charFromHex(const char *str, const QFile& file, int line)
{
    unsigned char ret = 0;
    for (int i = 0; i < 2; i++) {
        ret <<= 4;
        quint8 c = quint8(str[i]);

        if (c >= '0' && c <= '9') {
            ret |= c - '0';
        } else if (c >= 'a' && c <= 'f') {
            ret |= c - 'a' + 0x0a;
        } else if (c >= 'A' && c <= 'F') {
            ret |= c - 'A' + 0x0a;
        } else {
            QByteArray e(str, 2);
            e.prepend("\\x");
            kWarning() << warningProlog(file, line) << "Invalid hex character " << c
                    << " in \\x<nn>-type escape sequence \"" << e.constData() << "\"." << endl;
            return 'x';
        }
    }
    return char(ret);
}

QByteArray KConfigIniBackend::printableToString(const QByteArray& aString, const QFile& file, int line)
{
    if (aString.isEmpty())
        return QByteArray("");

    const char *str = aString.constData();
    int l = aString.length();

    // Strip leading white-space.
    while((l > 0) && ((*str == ' ') || (*str == '\t') || (*str == '\r'))) {
        str++; l--;
     }


    // Strip trailing white-space.
    while((l > 0) && ((str[l-1] == ' ') || (str[l-1] == '\t') || (str[l-1] == '\r'))) {
        l--;
    }

    QByteArray result(l, 0);
    char *r = result.data();

    for(int i = 0; i < l; i++, r++) {
        if (str[i] != '\\') {
            *r = str[i];
        } else {
            // Probable escape sequence
            i++;
            if (i >= l) { // Line ends after backslash - stop.
                *r = '\\';
                break;
            }

            switch(str[i]) {
                case 's':
                    *r = ' ';
                    break;
                case 't':
                    *r = '\t';
                    break;
                case 'n':
                    *r = '\n';
                    break;
                case 'r':
                    *r = '\r';
                    break;
                case '\\':
                    *r = '\\';
                    break;
                case 'x':
                    if (i + 2 < l) {
                        *r = charFromHex(str + i + 1, file, line);
                        i += 2;
                    } else {
                        *r = 'x';
                        i = l - 1;
                    }
                    break;
                default:
                    *r = '\\';
                    kWarning() << warningProlog(file, line)
                            << QString("Invalid escape sequence \"\\%1\".").arg(str[i]) << endl;
            }
        }
    }
    result.truncate(r - result.constData());
    return result;
}
