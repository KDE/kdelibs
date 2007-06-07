/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include "kconfigini.h"

#include <config.h>

#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif
#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>

#include "kde_file.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "klocale.h"
#include "ksavefile.h"
#include "kstandarddirs.h"
#include "kurl.h"

extern bool checkAccess(const QString& pathname, int mode);

class KConfigINIBackEndPrivate
{
public:
    KConfigINIBackEndPrivate()
     : m_localLastSize(0)
    {}

    uint m_localLastSize;
    QDateTime m_localLastModified;

    enum StringType {
        OtherString = 0,
        ValueString = 1
    };
    inline static char charFromHex(const char *str, const QFile &file, int lineno);
    static QByteArray printableToString(const char *str, int l, const QFile &file, int lineno);
    static QByteArray stringToPrintable(const QByteArray& str, StringType strType = OtherString);
    static QString warningProlog(const QFile &file, int lineno);

    /** Write the entries in @e aTempMap to the file.*/
    void writeEntries(QFile &ts, const KEntryMap &aTempMap) const;

    void parseLocalConfig(const QString &fileName, const QString &localFileName);

    KConfigINIBackEnd *q;
};

inline char KConfigINIBackEndPrivate::charFromHex(const char *str, const QFile &file, int lineno)
{
    unsigned char ret = 0;
    unsigned char c;
    for (int i = 0; i < 2; i++) {
        ret <<= 4;
        c = (unsigned char)str[i];

        if (c >= '0' && c <= '9') {
            ret |= c - '0';
        } else if (c >= 'a' && c <= 'f') {
            ret |= c - 'a' + 0x0a;
        } else if (c >= 'A' && c <= 'F') {
            ret |= c - 'A' + 0x0a;
        } else {
            QByteArray e(str, 2);
            e.prepend("\\x");
            kWarning() << warningProlog(file, lineno) << "Invalid hex character " << c
                       << " in \\x<nn>-type escape sequence \"" << e.constData() << "\"." << endl;
            return 'x';
        }
    }
    return (char)ret;
}

/* translate escape sequences to their actual values. */
QByteArray KConfigINIBackEndPrivate::printableToString(const char *str, int l, const QFile &file, int lineno)
{
    // Strip leading white-space.
    while((l > 0) &&
          ((*str == ' ') || (*str == '\t') || (*str == '\r'))) {
        str++; l--;
    }

    // Strip trailing white-space.
    while((l > 0) &&
          ((str[l-1] == ' ') || (str[l-1] == '\t') || (str[l-1] == '\r'))) {
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
                    *r = charFromHex(str + i + 1, file, lineno);
                    i += 2;
                } else {
                    *r = 'x';
                    i = l - 1;
                }
                break;
            default:
                *r = '\\';
                kWarning() << warningProlog(file, lineno)
                           << QString("Invalid escape sequence \"\\%1\".").arg(str[i]) << endl;
            }
        }
    }
    result.truncate(r - result.constData());
    return result;
}

QByteArray KConfigINIBackEndPrivate::stringToPrintable(const QByteArray& str, StringType strType)
{
    static const char nibbleLookup[] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    const int l = str.length();
    QByteArray result; // Guesstimated that it's good to avoid data() initialization for a length of l*4
    result.resize(l * 4); // Maximum 4x as long as source string due to \x<ab> escape sequences
    register char *r = result.data();
    register const char *s = str.constData();
    int i = 0;

    if (l == 0)
        return QByteArray();

    // Protect leading space
    if (s[0] == ' ') {
        *r++ = '\\';
        *r++ = 's';
        i++;
    }

    for (; i < l; i++, r++) {
        switch (s[i]) {
        default:
            // The \n, \t, \r cases (all < 32) are handled below; we can ignore them here
            if (((unsigned char)s[i]) < 32)
                goto escape;
            *r = s[i];
            break;
        case '\n':
            *r++ = '\\';
            *r = 'n';
            break;
        case '\t':
            *r++ = '\\';
            *r = 't';
            break;
        case '\r':
            *r++ = '\\';
            *r = 'r';
            break;
        case '\\':
            *r++ = '\\';
            *r = '\\';
            break;
        case '[':
        case ']':
        case '=':
            // Above chars are OK to put in *value* strings as plaintext
            if (strType == ValueString) {
                *r = s[i];
                break;
            }
        escape:
            *r++ = '\\';
            *r++ = 'x';
            *r++ = nibbleLookup[((unsigned char)s[i]) >> 4];
            *r = nibbleLookup[((unsigned char)s[i]) & 0x0f];
            break;
        }
    }

    // Protect trailing space
    if (i - 1 >= 0 && s[i-1] == ' ') {
        *(r-1) = '\\';
        *r++ = 's';
    }

    result.truncate(r - result.constData());
    return result;
}

QString KConfigINIBackEndPrivate::warningProlog(const QFile &file, int lineno)
{
    return QString("KConfigIni: In file %2, line %1: ")
                  .arg(lineno).arg(file.fileName());
}


KConfigINIBackEnd::KConfigINIBackEnd(KConfigBase *_config, const QString &_fileName,
                                     const char * _resType, bool _useKDEGlobals)
    : KConfigBackEnd(_config, _fileName, _resType, _useKDEGlobals),
      d(new KConfigINIBackEndPrivate)
{
    d->q = this;
}

KConfigINIBackEnd::~KConfigINIBackEnd()
{
    delete d;
}

void KConfigINIBackEndPrivate::parseLocalConfig(const QString &fileName, const QString &localFileName)
{
    // Check if we can write to the local file.
    if (!localFileName.isEmpty())
    {
        if (KStandardDirs::checkAccess(localFileName, W_OK))
        {
            q->mConfigState = KConfigBase::ReadWrite;
        }
        else
        {
            // Create the containing dir, maybe it wasn't there
            KUrl path;
            path.setPath(localFileName);
            QString dir=path.directory();
            KStandardDirs::makeDir(dir);

            if (KStandardDirs::checkAccess(localFileName, W_OK))
            {
                q->mConfigState = KConfigBase::ReadWrite;
            }
        }
        QFileInfo info(localFileName);
        m_localLastModified = info.lastModified();
        m_localLastSize = info.size();
    }

    bool bReadFile = !fileName.isEmpty();
    while(bReadFile) {
        bReadFile = false;
        QString bootLanguage;
        if (q->useKDEGlobals && q->localeString.isEmpty() && !KGlobal::hasLocale()) {
            // Boot strap language
            bootLanguage = KLocale::_initLanguage(q->pConfig);
            q->setLocaleString(bootLanguage.toUtf8());
        }

        q->bFileImmutable = false;
        QStringList list;
        if ( !QDir::isRelativePath(fileName) )
            list << fileName;
        else
            list = q->pConfig->componentData().dirs()->findAllResources(q->resType, fileName);

        QListIterator<QString> it( list );
        it.toBack();
        while (it.hasPrevious()) {
            QFile aConfigFile( it.previous() );
            // we can already be sure that this file exists

            bool bIsLocal = (aConfigFile.fileName() == localFileName);
            bool isMostSpecific = !q->mMergeStack.count() || fileName == q->mMergeStack.top();
            bool bDefaults = (!bIsLocal || !isMostSpecific);
            if (aConfigFile.open( QIODevice::ReadOnly )) {
                q->parseSingleConfigFile( aConfigFile, 0L, false, bDefaults );
                aConfigFile.close();
                if (q->bFileImmutable)
                    break;
            }
        }
        if (q->pConfig->componentData().dirs()->isRestrictedResource(q->resType, fileName))
            q->bFileImmutable = true;
        QString currentLanguage;
        if (!bootLanguage.isEmpty())
        {
            currentLanguage = KLocale::_initLanguage(q->pConfig);
            // If the file changed the language, we need to read the file again
            // with the new language setting.
            if (bootLanguage != currentLanguage)
            {
                bReadFile = true;
                q->setLocaleString(currentLanguage.toUtf8());
            }
        }
    }
}

bool KConfigINIBackEnd::parseConfigFiles()
{
    // Parse all desired files from the least to the most specific.
    bFileImmutable = false;

    // Parse the general config files
    if (useKDEGlobals) {
        QStringList kdercs = pConfig->componentData().dirs()->
                             findAllResources("config", QLatin1String("kdeglobals"));

#ifdef Q_WS_WIN
        QString etc_kderc = QFile::decodeName( QByteArray(getenv("WINDIR")) + "\\kderc" );
#else
        QString etc_kderc = QLatin1String("/etc/kderc");
#endif

        if (KStandardDirs::checkAccess(etc_kderc, R_OK))
            kdercs += etc_kderc;

        kdercs += pConfig->componentData().dirs()->
                  findAllResources("config", QLatin1String("system.kdeglobals"));

        QListIterator<QString> it( kdercs );
        it.toBack();
        while (it.hasPrevious()) {
            QFile aConfigFile( it.previous() );
            if (!aConfigFile.open( QIODevice::ReadOnly ))
                continue;
            parseSingleConfigFile( aConfigFile, 0L, true,
                                   (aConfigFile.fileName() != mGlobalFileName) );
            aConfigFile.close();
            if (bFileImmutable)
                break;
        }
    }

    mConfigState = KConfigBase::ReadOnly;
    d->parseLocalConfig( mfileName, mLocalFileName );

    foreach(const QString &fileName, mMergeStack)
    {
        mConfigState = KConfigBase::ReadOnly;
        d->parseLocalConfig( fileName, fileName );
    }

    if (bFileImmutable)
        mConfigState = KConfigBase::ReadOnly;

    return true;
}

#ifdef HAVE_MMAP
#ifdef SIGBUS
static sigjmp_buf mmap_jmpbuf;
struct sigaction mmap_old_sigact;

extern "C" {
   static void mmap_sigbus_handler(int)
   {
      siglongjmp (mmap_jmpbuf, 1);
   }
}
#endif
#endif

extern bool kde_kiosk_exception;

void KConfigINIBackEnd::parseSingleConfigFile(QFile &rFile,
					      KEntryMap *pWriteBackMap,
					      bool bGlobal, bool bDefault)
{
    const char *s; // May get clobbered by sigsetjump, but we don't use them afterwards.
    const char *eof; // May get clobbered by sigsetjump, but we don't use them afterwards.
    QByteArray data;

    if (!rFile.isOpen()) // come back, if you have real work for us ;->
        return;

    //using kDebug() here leads to an infinite loop
    //remove this for the release, aleXXX
    //qWarning("Parsing %s, global = %s default = %s",
    //           rFile.name().toLatin1(), bGlobal ? "true" : "false", bDefault ? "true" : "false");

    QByteArray aCurrentGroup("<default>");

    unsigned int ll = localeString.length();

#ifdef HAVE_MMAP
    static volatile const char *map;
    map = ( const char* ) mmap(0, rFile.size(), PROT_READ, MAP_PRIVATE,
                               rFile.handle(), 0);

    if ( map != MAP_FAILED )
    {
        s = (const char*) map;
        eof = s + rFile.size();

#ifdef SIGBUS
        struct sigaction act;
        act.sa_handler = mmap_sigbus_handler;
        sigemptyset( &act.sa_mask );
#ifdef SA_ONESHOT
        act.sa_flags = SA_ONESHOT;
#else
        act.sa_flags = SA_RESETHAND;
#endif
        sigaction( SIGBUS, &act, &mmap_old_sigact );

        if (sigsetjmp (mmap_jmpbuf, 1))
        {
            qWarning("SIGBUS while reading %s", rFile.fileName().toLatin1().constData());
            munmap(( char* )map, rFile.size());
            sigaction (SIGBUS, &mmap_old_sigact, 0);
            return;
        }
#endif
    }
    else
#endif
    {
        rFile.seek(0);
        data = rFile.readAll();
        s = data.constData();
        eof = s + data.size();
    }

    bool fileOptionImmutable = false;
    bool groupOptionImmutable = false;
    bool groupSkip = false;

    int line = 0;
    for(; s < eof; s++)
    {
        line++;

        while((s < eof) && isspace(*s) && (*s != '\n'))
            s++; //skip leading whitespace, shouldn't happen too often

        //skip empty lines, lines starting with #
        if ((s < eof) && ((*s == '\n') || (*s == '#')))
        {
        sktoeol:	//skip till end-of-line
            while ((s < eof) && (*s != '\n'))
                s++;
            continue; // Empty or comment or no keyword
        }
        const char *startLine = s;

        if (*s == '[')  //group
        {
            // In a group [[ and ]] have a special meaning
            while ((s < eof) && (*s != '\n'))
            {
                if (*s == ']')
                {
                    if ((s+1 < eof) && (*(s+1) == ']'))
                        s++; // Skip "]]"
                    else
                        break;
                }

                s++; // Search till end of group
            }
            const char *e = s;
            while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file
            if ((e >= eof) || (*e != ']'))
            {
                kWarning() << d->warningProlog(rFile, line) << "Invalid group header." << endl;
                continue;
            }
            // group found; get the group name by taking everything in
            // between the brackets
            if ((e-startLine == 3) &&
                (startLine[1] == '$') &&
                (startLine[2] == 'i'))
            {
                if (!kde_kiosk_exception)
                    fileOptionImmutable = true;
                continue;
            }

            aCurrentGroup = d->printableToString(startLine + 1, e - startLine - 1,
                                                 rFile, line);
            //cout<<"found group ["<<aCurrentGroup<<"]"<<endl;

            groupOptionImmutable = fileOptionImmutable;

            e++;
            if ((e+2 < eof) && (*e++ == '[') && (*e++ == '$')) // Option follows
            {
                if ((*e == 'i') && !kde_kiosk_exception)
                {
                    groupOptionImmutable = true;
                }
            }

            KEntryKey groupKey(aCurrentGroup, 0);
            KEntry entry = pConfig->lookupData(groupKey);
            groupSkip = entry.bImmutable;

            if (groupSkip && !bDefault)
                continue;

            entry.bImmutable |= groupOptionImmutable;
            pConfig->putData(groupKey, entry, false);

            if (pWriteBackMap)
            {
                // add the special group key indicator
                (*pWriteBackMap)[groupKey] = entry;
            }

            continue;
        }
        if (groupSkip && !bDefault)
            goto sktoeol; // Skip entry

        bool optionImmutable = groupOptionImmutable;
        bool optionDeleted = false;
        bool optionExpand = false;
        const char *endOfKey = 0, *locale = 0, *elocale = 0;
        for (; (s < eof) && (*s != '\n'); s++)
        {
            if (*s == '=') //find the equal sign
            {
                if (!endOfKey)
                    endOfKey = s;
                goto haveeq;
            }
            if (*s == '[') //find the locale or options.
            {
                const char *option;
                const char *eoption;
                endOfKey = s;
                option = ++s;
                for (;; s++)
                {
                    if ((s >= eof) || (*s == '\n') || (*s == '=')) {
                        kWarning() << d->warningProlog(rFile, line)
                                   << "Invalid entry (missing ']')." << endl;
                        goto sktoeol;
                    }
                    if (*s == ']')
                        break;
                }
                eoption = s;
                if (*option != '$')
                {
                    // Locale
                    if (locale) {
                        kWarning() << d->warningProlog(rFile, line)
                                   << "Invalid entry (second locale!?)." << endl;
                        goto sktoeol;
                    }
                    locale = option;
                    elocale = eoption;
                }
                else
                {
                    // Option
                    while (option < eoption)
                    {
                        option++;
                        if ((*option == 'i') && !kde_kiosk_exception)
                            optionImmutable = true;
                        else if (*option == 'e')
                            optionExpand = true;
                        else if (*option == 'd')
                        {
                            optionDeleted = true;
                            goto haveeq;
                        }
                        else if (*option == ']')
                            break;
                    }
                }
            }
        }
        kWarning() << d->warningProlog(rFile, line)
                   << "Invalid entry (missing '=')." << endl;
        continue;

    haveeq:
        for (endOfKey--; ; endOfKey--)
        {
            if (endOfKey < startLine)
            {
                kWarning() << d->warningProlog(rFile, line)
                           << "Invalid entry (empty key)." << endl;
                goto sktoeol;
            }
            if (!isspace(*endOfKey))
                break;
        }

        const char *st = ++s;
        while ((s < eof) && (*s != '\n')) s++; // Search till end of line / end of file

        if (locale) {
            unsigned int cl = static_cast<unsigned int>(elocale - locale);
            if ((ll != cl) || memcmp(locale, localeString.constData(), ll))
            {
                // backward compatibility. C == en_US
                if ( cl != 1 || ll != 5 || *locale != 'C' || memcmp(localeString.constData(), "en_US", 5)) {
                    //cout<<"mismatched locale '"<<QByteArray(locale, elocale-locale +1)<<"'"<<endl;
                    // We can ignore this one
                    if (!pWriteBackMap)
                        continue; // We just ignore it
                    // We just store it as is to be able to write it back later.
                    endOfKey = elocale;
                    locale = 0;
                }
            }
        }

        // insert the key/value line
        QByteArray key = d->printableToString(startLine, endOfKey - startLine + 1, rFile, line);
        QByteArray val = d->printableToString(st, s - st, rFile, line);
        //qDebug("found key '%s' with value '%s'", key.constData(), val.constData());

        KEntryKey aEntryKey(aCurrentGroup, key);
        aEntryKey.bLocal = (locale != 0);
        aEntryKey.bDefault = bDefault;

        KEntry aEntry;
        aEntry.mValue = val;
        aEntry.bGlobal = bGlobal;
        aEntry.bImmutable = optionImmutable;
        aEntry.bDeleted = optionDeleted;
        aEntry.bExpand = optionExpand;
        aEntry.bNLS = (locale != 0);

        if (pWriteBackMap) {
            // don't insert into the config object but into the temporary
            // scratchpad map
            pWriteBackMap->insert(aEntryKey, aEntry);
        } else {
            // directly insert value into config object
            // no need to specify localization; if the key we just
            // retrieved was localized already, no need to localize it again.
            pConfig->putData(aEntryKey, aEntry, false);
        }
    }
    if (fileOptionImmutable)
        bFileImmutable = true;

#ifdef HAVE_MMAP
    if (map)
    {
        munmap(( char* )map, rFile.size());
#ifdef SIGBUS
        sigaction (SIGBUS, &mmap_old_sigact, 0);
#endif
    }
#endif
}

void KConfigINIBackEnd::sync(bool bMerge)
{
    // write-sync is only necessary if there are dirty entries
    if (!pConfig->isDirty()) {
        return;
    }

    bool bEntriesLeft = true;

    // find out the file to write to (most specific writable file)
    // try local app-specific file first
    QString fileName = mMergeStack.count() ? mMergeStack.top()
                       : mLocalFileName;

    if (!fileName.isEmpty()) {
        // Create the containing dir if needed
        if ((resType!="config") && !QDir::isRelativePath(fileName))
        {
            KUrl path;
            path.setPath(fileName);
            QString dir=path.directory();
            KStandardDirs::makeDir(dir);
        }

        // Can we allow the write? We can, if the program
        // doesn't run SUID. But if it runs SUID, we must
        // check if the user would be allowed to write if
        // it wasn't SUID.
        if (KStandardDirs::checkAccess(fileName, W_OK)) {
            // File is writable
            KLockFile::Ptr lf;

            bool mergeFile = bMerge;
            // Check if the file has been updated since.
            if (mergeFile)
            {
                lf = lockFile(false); // Lock file for local file
                if (lf && lf->isLocked())
                    lf = 0; // Already locked, we don't need to lock/unlock again

                if (lf)
                {
                    lf->lock( KLockFile::ForceFlag );
                    // But what if the locking failed? Ignore it for now...
                }

                QFileInfo info(fileName);
                if ((d->m_localLastSize == info.size()) &&
                    (d->m_localLastModified == info.lastModified()))
                {
                    // Not changed, don't merge.
                    mergeFile = false;
                }
                else
                {
                    // Changed...
                    d->m_localLastModified = QDateTime();
                    d->m_localLastSize = 0;
                }
            }

            bEntriesLeft = writeConfigFile( fileName, false, mergeFile );

            // Only if we didn't have to merge anything can we use our in-memory state
            // the next time around. Otherwise the config-file may contain entries
            // that are different from our in-memory state which means we will have to
            // do a merge from then on.
            // We do not automatically update the in-memory state with the on-disk
            // state when writing the config to disk. We only do so when
            // KCOnfig::reparseConfiguration() is called.
            // For KDE 4.0 we may wish to reconsider that.
            if (!mergeFile)
            {
                QFileInfo info(fileName);
                d->m_localLastModified = info.lastModified();
                d->m_localLastSize = info.size();
            }
            if (lf) lf->unlock();
        }
    }

    // only write out entries to the kdeglobals file if there are any
    // entries marked global (indicated by bEntriesLeft) and
    // the useKDEGlobals flag is set.
    if (bEntriesLeft && useKDEGlobals) {

        // can we allow the write? (see above)
        if (KStandardDirs::checkAccess ( mGlobalFileName, W_OK )) {
            KLockFile::Ptr lf = lockFile(true); // Lock file for global file
            if (lf && lf->isLocked())
                lf = 0; // Already locked, we don't need to lock/unlock again

            if (lf)
            {
                lf->lock( KLockFile::ForceFlag );
                // But what if the locking failed? Ignore it for now...
            }
            writeConfigFile( mGlobalFileName, true, bMerge ); // Always merge
            if (lf) lf->unlock();
        }
    }

}

void KConfigINIBackEndPrivate::writeEntries(QFile &file, const KEntryMap &aTempMap) const
{
    bool defaultGroup;
    //once for the default group, once for others
    for (int i = 0; i < 2; i++) {
        QByteArray currentGroup;
        defaultGroup = (i == 0);

        for (KEntryMapConstIterator aIt = aTempMap.begin(); aIt != aTempMap.end(); ++aIt)
        {
            const KEntryKey &key = aIt.key();

            // Either proces the default group or all others
            if ((key.mGroup != "<default>") == defaultGroup)
                continue; // Skip

            // Skip default values and group headers.
            if ((key.bDefault) || key.mKey.isEmpty()) {
                continue; // Skip
            }

            const KEntry &currentEntry = *aIt;

            KEntryMapConstIterator aTestIt = aIt;
            ++aTestIt;
            bool hasDefault = (aTestIt != aTempMap.end());
            if (hasDefault)
            {
                const KEntryKey &defaultKey = aTestIt.key();
                if ((!defaultKey.bDefault) ||
                    (defaultKey.mKey != key.mKey) ||
                    (defaultKey.mGroup != key.mGroup) ||
                    (defaultKey.bLocal != key.bLocal))
                    hasDefault = false;
            }


            if ( hasDefault ) {
                // Entry had a default value
                if ( (currentEntry.mValue == (*aTestIt).mValue) &&
                    (currentEntry.bDeleted == (*aTestIt).bDeleted) ) {
                    continue; // Same as default, don't write.
                }
            } else if ( currentEntry.bDeleted ) {
                // Entry had no default value.
                continue; // Don't write deleted entries if there is no default.
            }

            if (!defaultGroup && (currentGroup != key.mGroup)) {
                if (i > 0)
                    file.putChar('\n');
                currentGroup = key.mGroup;
                file.putChar('[');
                file.write(stringToPrintable(currentGroup));
                file.write("]\n", 2);
            }

            // it is data for a group
            file.write(stringToPrintable(key.mKey));   // Key

            if ( currentEntry.bNLS )
            {
                file.putChar('[');
                file.write(q->localeString);
                file.putChar(']');
            }

            if (currentEntry.bDeleted) {
                file.write("[$d]\n"); // Deleted
            } else {
                if (currentEntry.bImmutable || currentEntry.bExpand)
                {
                    file.putChar('[');
                    file.putChar('$');
                    if (currentEntry.bImmutable)
                        file.putChar('i');
                    if (currentEntry.bExpand)
                        file.putChar('e');

                    file.putChar(']');
                }
                file.putChar('=');
                file.write(stringToPrintable(currentEntry.mValue, ValueString));
                file.putChar('\n');
            }
        } // iterator
    } //default / non-default
}

bool KConfigINIBackEnd::getEntryMap(KEntryMap &aTempMap, bool bGlobal,
                                    QFile *mergeFile)
{
    bool bEntriesLeft = false;
    bFileImmutable = false;

    // Read entries from disk
    if (mergeFile && mergeFile->open(QIODevice::ReadOnly))
    {
        // fill the temporary structure with entries from the file
        parseSingleConfigFile(*mergeFile, &aTempMap, bGlobal, false );

        if (bFileImmutable) // File has become immutable on disk
            return bEntriesLeft;
    }

    KEntryMap aMap = pConfig->internalEntryMap();

    // augment this structure with the dirty entries from the config object
    for (KEntryMapIterator aIt = aMap.begin();
         aIt != aMap.end(); ++aIt)
    {
        const KEntry currentEntry = *aIt;
        const KEntryKey key = aIt.key();
        if(aIt.key().bDefault)
        {
            aTempMap.remove(key);
            aTempMap.insert(key, currentEntry);
            continue;
        }

        if (mergeFile && !currentEntry.bDirty)
            continue;

        // only write back entries that have the same
        // "globality" as the file
        if (currentEntry.bGlobal != bGlobal)
        {
            // wrong "globality" - might have to be saved later
            bEntriesLeft = true;
            continue;
        }

        // put this entry from the config object into the
        // temporary map, possibly replacing an existing entry
        KEntryMapIterator aIt2 = aTempMap.find(aIt.key());
        if (aIt2 != aTempMap.end() && (*aIt2).bImmutable)
            continue; // Bail out if the on-disk entry is immutable

        aTempMap.remove(key);
        aTempMap.insert(key, currentEntry);
    } // loop

    return bEntriesLeft;
}

bool KConfigINIBackEnd::writeConfigFile(const QString &filename, bool bGlobal,
					bool bMerge)
{
    // is the config object read-only?
    //TODO: replace with a check for isWritable() with new backend API
    //  if (pConfig->isReadOnly())
    //  return true; // pretend we wrote it

    KEntryMap aTempMap;
    QFile *mergeFile = (bMerge ? new QFile(filename) : 0);
    bool bEntriesLeft = getEntryMap(aTempMap, bGlobal, mergeFile);
    delete mergeFile;
    if (bFileImmutable)
        return true; // pretend we wrote it

    // OK now the temporary map should be full of ALL entries.
    // write it out to disk.

    // Check if file exists:
    int fileMode = -1;
    bool createNew = true;

    KDE_struct_stat buf;
    if (KDE_stat(QFile::encodeName(filename), &buf) == 0)
    {
        if (buf.st_uid == getuid() || buf.st_uid == -2)
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

    KSaveFile *pConfigFile = 0;
    QFile *f = 0;

    if (createNew)
    {
        pConfigFile = new KSaveFile(filename, pConfig->componentData());

        if (!pConfigFile->open())
        {
            delete pConfigFile;
            return bEntriesLeft;
        }
        pConfigFile->setPermissions(QFile::ReadUser|QFile::WriteUser);

        if (!bGlobal && (fileMode == -1))
            fileMode = mFileMode;

        if (fileMode != -1)
        {
            fchmod(pConfigFile->handle(), fileMode);
        }

        f = pConfigFile;
    }
    else
    {
        // Open existing file.
        // We use open() to ensure that we call without O_CREAT.
        f = new QFile( filename );
        if (!f->open( QIODevice::WriteOnly | QIODevice::Append ))
        {
            delete f;
            return bEntriesLeft;
        }
    }
    d->writeEntries(*f, aTempMap);

    if (pConfigFile)
    {
        bool bEmptyFile = f->pos() == 0;
        if ( bEmptyFile && ((fileMode == -1) || (fileMode == 0600)) )
        {
            // File is empty and doesn't have special permissions: delete it.
            pConfigFile->abort();

            if ( fileMode != -1 ) {
                // also remove the old file in case it existed. this can happen
                // when we delete all then entries in an existing config file
                // if we don't do this, then deletions and revertToDefault's
                // will mysteriously fail
                QFile::remove( filename );
            }

        }
        else
        {
            // Normal case: Close the file
            pConfigFile->finalize();
        }
        delete pConfigFile;
    }
    else
    {
        f->close();
    }

    return bEntriesLeft;
}
