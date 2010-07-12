/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <kde_file.h>

#include <QtCore/QDate>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#ifdef _WIN32_WCE
#include <QtCore/QDir>
#endif

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include "kconfigutils.h"

class KonfUpdate
{
public:
    KonfUpdate();
    ~KonfUpdate();
    QStringList findUpdateFiles(bool dirtyOnly);

    QTextStream &log();
    QTextStream &logFileError();

    bool checkFile(const QString &filename);
    void checkGotFile(const QString &_file, const QString &id);

    bool updateFile(const QString &filename);

    void gotId(const QString &_id);
    void gotFile(const QString &_file);
    void gotGroup(const QString &_group);
    void gotRemoveGroup(const QString &_group);
    void gotKey(const QString &_key);
    void gotRemoveKey(const QString &_key);
    void gotAllKeys();
    void gotAllGroups();
    void gotOptions(const QString &_options);
    void gotScript(const QString &_script);
    void gotScriptArguments(const QString &_arguments);
    void resetOptions();

    void copyGroup(const KConfigBase *cfg1, const QString &group1,
                   KConfigBase *cfg2, const QString &group2);
    void copyGroup(const KConfigGroup &cg1, KConfigGroup &cg2);
    void copyOrMoveKey(const QStringList &srcGroupPath, const QString &srcKey, const QStringList &dstGroupPath, const QString &dstKey);
    void copyOrMoveGroup(const QStringList &srcGroupPath, const QStringList &dstGroupPath);

    QStringList parseGroupString(const QString &_str);

protected:
    KConfig *m_config;
    QString m_currentFilename;
    bool m_skip;
    bool m_skipFile;
    bool m_debug;
    QString m_id;

    QString m_oldFile;
    QString m_newFile;
    QString m_newFileName;
    KConfig *m_oldConfig1; // Config to read keys from.
    KConfig *m_oldConfig2; // Config to delete keys from.
    KConfig *m_newConfig;

    QStringList m_oldGroup;
    QStringList m_newGroup;

    bool m_bCopy;
    bool m_bOverwrite;
    bool m_bUseConfigInfo;
    QString m_arguments;
    QTextStream *m_textStream;
    QFile *m_file;
    QString m_line;
    int m_lineCount;
};

KonfUpdate::KonfUpdate()
        : m_textStream(0), m_file(0)
{
    bool updateAll = false;
    m_oldConfig1 = 0;
    m_oldConfig2 = 0;
    m_newConfig = 0;

    m_config = new KConfig("kconf_updaterc");
    KConfigGroup cg(m_config, QString());

    QStringList updateFiles;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    m_debug = args->isSet("debug");

    m_bUseConfigInfo = false;
    if (args->isSet("check")) {
        m_bUseConfigInfo = true;
        QString file = KStandardDirs::locate("data", "kconf_update/" + args->getOption("check"));
        if (file.isEmpty()) {
            qWarning("File '%s' not found.", args->getOption("check").toLocal8Bit().data());
            log() << "File '" << args->getOption("check") << "' passed on command line not found" << endl;
            return;
        }
        updateFiles.append(file);
    } else if (args->count()) {
        for (int i = 0; i < args->count(); i++) {
            KUrl url = args->url(i);
            if (!url.isLocalFile()) {
                KCmdLineArgs::usageError(i18n("Only local files are supported."));
            }
            updateFiles.append(url.path());
        }
    } else {
        if (cg.readEntry("autoUpdateDisabled", false))
            return;
        updateFiles = findUpdateFiles(true);
        updateAll = true;
    }

    for (QStringList::ConstIterator it = updateFiles.constBegin();
            it != updateFiles.constEnd();
            ++it) {
        updateFile(*it);
    }

    if (updateAll && !cg.readEntry("updateInfoAdded", false)) {
        cg.writeEntry("updateInfoAdded", true);
        updateFiles = findUpdateFiles(false);

        for (QStringList::ConstIterator it = updateFiles.constBegin();
                it != updateFiles.constEnd();
                ++it) {
            checkFile(*it);
        }
        updateFiles.clear();
    }
}

KonfUpdate::~KonfUpdate()
{
    delete m_config;
    delete m_file;
    delete m_textStream;
}

QTextStream & operator<<(QTextStream & stream, const QStringList & lst)
{
    stream << lst.join(", ");
    return stream;
}

QTextStream &
KonfUpdate::log()
{
    if (!m_textStream) {
        QString file = KStandardDirs::locateLocal("data", "kconf_update/log/update.log");
        m_file = new QFile(file);
        if (m_file->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_textStream = new QTextStream(m_file);
        } else {
            // Error
            m_textStream = new QTextStream(stderr, QIODevice::WriteOnly);
        }
    }

    (*m_textStream) << QDateTime::currentDateTime().toString(Qt::ISODate) << " ";

    return *m_textStream;
}

QTextStream &
KonfUpdate::logFileError()
{
    return log() << m_currentFilename << ':' << m_lineCount << ":'" << m_line << "': ";
}

QStringList KonfUpdate::findUpdateFiles(bool dirtyOnly)
{
    QStringList result;
    const QStringList list = KGlobal::dirs()->findAllResources("data", "kconf_update/*.upd",
                             KStandardDirs::NoDuplicates);
    for (QStringList::ConstIterator it = list.constBegin();
            it != list.constEnd();
            ++it) {
        QString file = *it;
        KDE_struct_stat buff;
        if (KDE::stat(file, &buff) == 0) {
            int i = file.lastIndexOf('/');
            if (i != -1) {
                file = file.mid(i + 1);
            }
            KConfigGroup cg(m_config, file);
            time_t ctime = cg.readEntry("ctime", 0);
            time_t mtime = cg.readEntry("mtime", 0);
            if (!dirtyOnly ||
                    (ctime != buff.st_ctime) || (mtime != buff.st_mtime)) {
                result.append(*it);
            }
        }
    }
    return result;
}

bool KonfUpdate::checkFile(const QString &filename)
{
    m_currentFilename = filename;
    int i = m_currentFilename.lastIndexOf('/');
    if (i != -1) {
        m_currentFilename = m_currentFilename.mid(i + 1);
    }
    m_skip = true;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream ts(&file);
    ts.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    int lineCount = 0;
    resetOptions();
    QString id;
    while (!ts.atEnd()) {
        QString line = ts.readLine().trimmed();
        lineCount++;
        if (line.isEmpty() || (line[0] == '#')) {
            continue;
        }
        if (line.startsWith("Id=")) {
            id = m_currentFilename + ':' + line.mid(3);
        } else if (line.startsWith("File=")) {
            checkGotFile(line.mid(5), id);
        }
    }

    return true;
}

void KonfUpdate::checkGotFile(const QString &_file, const QString &id)
{
    QString file;
    int i = _file.indexOf(',');
    if (i == -1) {
        file = _file.trimmed();
    } else {
        file = _file.mid(i + 1).trimmed();
    }

//   qDebug("File %s, id %s", file.toLatin1().constData(), id.toLatin1().constData());

    KConfig cfg(file, KConfig::SimpleConfig);
    KConfigGroup cg(&cfg, "$Version");
    QStringList ids = cg.readEntry("update_info", QStringList());
    if (ids.contains(id)) {
        return;
    }
    ids.append(id);
    cg.writeEntry("update_info", ids);
}

/**
 * Syntax:
 * # Comment
 * Id=id
 * File=oldfile[,newfile]
 * AllGroups
 * Group=oldgroup[,newgroup]
 * RemoveGroup=oldgroup
 * Options=[copy,][overwrite,]
 * Key=oldkey[,newkey]
 * RemoveKey=ldkey
 * AllKeys
 * Keys= [Options](AllKeys|(Key|RemoveKey)*)
 * ScriptArguments=arguments
 * Script=scriptfile[,interpreter]
 *
 * Sequence:
 * (Id,(File(Group,Keys)*)*)*
 **/
bool KonfUpdate::updateFile(const QString &filename)
{
    m_currentFilename = filename;
    int i = m_currentFilename.lastIndexOf('/');
    if (i != -1) {
        m_currentFilename = m_currentFilename.mid(i + 1);
    }
    m_skip = true;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    log() << "Checking update-file '" << filename << "' for new updates" << endl;

    QTextStream ts(&file);
    ts.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    m_lineCount = 0;
    resetOptions();
    while (!ts.atEnd()) {
        m_line = ts.readLine().trimmed();
        m_lineCount++;
        if (m_line.isEmpty() || (m_line[0] == '#')) {
            continue;
        }
        if (m_line.startsWith(QLatin1String("Id="))) {
            gotId(m_line.mid(3));
        } else if (m_skip) {
            continue;
        } else if (m_line.startsWith(QLatin1String("Options="))) {
            gotOptions(m_line.mid(8));
        } else if (m_line.startsWith(QLatin1String("File="))) {
            gotFile(m_line.mid(5));
        } else if (m_skipFile) {
            continue;
        } else if (m_line.startsWith(QLatin1String("Group="))) {
            gotGroup(m_line.mid(6));
        } else if (m_line.startsWith(QLatin1String("RemoveGroup="))) {
            gotRemoveGroup(m_line.mid(12));
            resetOptions();
        } else if (m_line.startsWith(QLatin1String("Script="))) {
            gotScript(m_line.mid(7));
            resetOptions();
        } else if (m_line.startsWith(QLatin1String("ScriptArguments="))) {
            gotScriptArguments(m_line.mid(16));
        } else if (m_line.startsWith(QLatin1String("Key="))) {
            gotKey(m_line.mid(4));
            resetOptions();
        } else if (m_line.startsWith(QLatin1String("RemoveKey="))) {
            gotRemoveKey(m_line.mid(10));
            resetOptions();
        } else if (m_line == "AllKeys") {
            gotAllKeys();
            resetOptions();
        } else if (m_line == "AllGroups") {
            gotAllGroups();
            resetOptions();
        } else {
            logFileError() << "Parse error" << endl;
        }
    }
    // Flush.
    gotId(QString());

    KDE_struct_stat buff;
    KDE::stat(filename, &buff);
    KConfigGroup cg(m_config, m_currentFilename);
    cg.writeEntry("ctime", int(buff.st_ctime));
    cg.writeEntry("mtime", int(buff.st_mtime));
    cg.sync();
    return true;
}



void KonfUpdate::gotId(const QString &_id)
{
    if (!m_id.isEmpty() && !m_skip) {
        KConfigGroup cg(m_config, m_currentFilename);

        QStringList ids = cg.readEntry("done", QStringList());
        if (!ids.contains(m_id)) {
            ids.append(m_id);
            cg.writeEntry("done", ids);
            cg.sync();
        }
    }

    // Flush pending changes
    gotFile(QString());
    KConfigGroup cg(m_config, m_currentFilename);

    QStringList ids = cg.readEntry("done", QStringList());
    if (!_id.isEmpty()) {
        if (ids.contains(_id)) {
            //qDebug("Id '%s' was already in done-list", _id.toLatin1().constData());
            if (!m_bUseConfigInfo) {
                m_skip = true;
                return;
            }
        }
        m_skip = false;
        m_skipFile = false;
        m_id = _id;
        if (m_bUseConfigInfo) {
            log() << m_currentFilename << ": Checking update '" << _id << "'" << endl;
        } else {
            log() << m_currentFilename << ": Found new update '" << _id << "'" << endl;
        }
    }
}

void KonfUpdate::gotFile(const QString &_file)
{
    // Reset group
    gotGroup(QString());

    if (!m_oldFile.isEmpty()) {
        // Close old file.
        delete m_oldConfig1;
        m_oldConfig1 = 0;

        KConfigGroup cg(m_oldConfig2, "$Version");
        QStringList ids = cg.readEntry("update_info", QStringList());
        QString cfg_id = m_currentFilename + ':' + m_id;
        if (!ids.contains(cfg_id) && !m_skip) {
            ids.append(cfg_id);
            cg.writeEntry("update_info", ids);
        }
        cg.sync();
        delete m_oldConfig2;
        m_oldConfig2 = 0;

        QString file = KStandardDirs::locateLocal("config", m_oldFile);
        KDE_struct_stat s_buf;
        if (KDE::stat(file, &s_buf) == 0) {
            if (s_buf.st_size == 0) {
                // Delete empty file.
                QFile::remove(file);
            }
        }

        m_oldFile.clear();
    }
    if (!m_newFile.isEmpty()) {
        // Close new file.
        KConfigGroup cg(m_newConfig, "$Version");
        QStringList ids = cg.readEntry("update_info", QStringList());
        QString cfg_id = m_currentFilename + ':' + m_id;
        if (!ids.contains(cfg_id) && !m_skip) {
            ids.append(cfg_id);
            cg.writeEntry("update_info", ids);
        }
        m_newConfig->sync();
        delete m_newConfig;
        m_newConfig = 0;

        m_newFile.clear();
    }
    m_newConfig = 0;

    int i = _file.indexOf(',');
    if (i == -1) {
        m_oldFile = _file.trimmed();
    } else {
        m_oldFile = _file.left(i).trimmed();
        m_newFile = _file.mid(i + 1).trimmed();
        if (m_oldFile == m_newFile) {
            m_newFile.clear();
        }
    }

    if (!m_oldFile.isEmpty()) {
        m_oldConfig2 = new KConfig(m_oldFile, KConfig::NoGlobals);
        QString cfg_id = m_currentFilename + ':' + m_id;
        KConfigGroup cg(m_oldConfig2, "$Version");
        QStringList ids = cg.readEntry("update_info", QStringList());
        if (ids.contains(cfg_id)) {
            m_skip = true;
            m_newFile.clear();
            log() << m_currentFilename << ": Skipping update '" << m_id << "'" << endl;
        }

        if (!m_newFile.isEmpty()) {
            m_newConfig = new KConfig(m_newFile, KConfig::NoGlobals);
            KConfigGroup cg(m_newConfig, "$Version");
            ids = cg.readEntry("update_info", QStringList());
            if (ids.contains(cfg_id)) {
                m_skip = true;
                log() << m_currentFilename << ": Skipping update '" << m_id << "'" << endl;
            }
        } else {
            m_newConfig = m_oldConfig2;
        }

        m_oldConfig1 = new KConfig(m_oldFile, KConfig::NoGlobals);
    } else {
        m_newFile.clear();
    }
    m_newFileName = m_newFile;
    if (m_newFileName.isEmpty()) {
        m_newFileName = m_oldFile;
    }

    m_skipFile = false;
    if (!m_oldFile.isEmpty()) { // if File= is specified, it doesn't exist, is empty or contains only kconf_update's [$Version] group, skip
        if (m_oldConfig1 != NULL
                && (m_oldConfig1->groupList().isEmpty()
                    || (m_oldConfig1->groupList().count() == 1 && m_oldConfig1->groupList().first() == "$Version"))) {
            log() << m_currentFilename << ": File '" << m_oldFile << "' does not exist or empty, skipping" << endl;
            m_skipFile = true;
        }
    }
}

QStringList KonfUpdate::parseGroupString(const QString &str)
{
    bool ok;
    QString error;
    QStringList lst = KConfigUtils::parseGroupString(str, &ok, &error);
    if (!ok) {
        logFileError() << error;
    }
    return lst;
}

void KonfUpdate::gotGroup(const QString &_group)
{
    QString group = _group.trimmed();
    if (group.isEmpty()) {
        m_oldGroup = m_newGroup = QStringList();
        return;
    }

    QStringList tokens = group.split(',');
    m_oldGroup = parseGroupString(tokens.at(0));
    if (tokens.count() == 1) {
        m_newGroup = m_oldGroup;
    } else {
        m_newGroup = parseGroupString(tokens.at(1));
    }
}

void KonfUpdate::gotRemoveGroup(const QString &_group)
{
    m_oldGroup = parseGroupString(_group);

    if (!m_oldConfig1) {
        logFileError() << "RemoveGroup without previous File specification" << endl;
        return;
    }

    KConfigGroup cg = KConfigUtils::openGroup(m_oldConfig2, m_oldGroup);
    if (!cg.exists()) {
        return;
    }
    // Delete group.
    cg.deleteGroup();
    log() << m_currentFilename << ": RemoveGroup removes group " << m_oldFile << ":" << m_oldGroup << endl;
}


void KonfUpdate::gotKey(const QString &_key)
{
    QString oldKey, newKey;
    int i = _key.indexOf(',');
    if (i == -1) {
        oldKey = _key.trimmed();
        newKey = oldKey;
    } else {
        oldKey = _key.left(i).trimmed();
        newKey = _key.mid(i + 1).trimmed();
    }

    if (oldKey.isEmpty() || newKey.isEmpty()) {
        logFileError() << "Key specifies invalid key" << endl;
        return;
    }
    if (!m_oldConfig1) {
        logFileError() << "Key without previous File specification" << endl;
        return;
    }
    copyOrMoveKey(m_oldGroup, oldKey, m_newGroup, newKey);
}

void KonfUpdate::copyOrMoveKey(const QStringList &srcGroupPath, const QString &srcKey, const QStringList &dstGroupPath, const QString &dstKey)
{
    KConfigGroup dstCg = KConfigUtils::openGroup(m_newConfig, dstGroupPath);
    if (!m_bOverwrite && dstCg.hasKey(dstKey)) {
        log() << m_currentFilename << ": Skipping " << m_newFileName << ":" << dstCg.name() << ":" << dstKey << ", already exists." << endl;
        return;
    }

    KConfigGroup srcCg = KConfigUtils::openGroup(m_oldConfig1, srcGroupPath);
    QString value = srcCg.readEntry(srcKey, QString());
    log() << m_currentFilename << ": Updating " << m_newFileName << ":" << dstCg.name() << ":" << dstKey << " to '" << value << "'" << endl;
    dstCg.writeEntry(dstKey, value);

    if (m_bCopy) {
        return; // Done.
    }

    // Delete old entry
    if (m_oldConfig2 == m_newConfig
        && srcGroupPath == dstGroupPath
        && srcKey == dstKey) {
        return; // Don't delete!
    }
    KConfigGroup srcCg2 = KConfigUtils::openGroup(m_oldConfig2, srcGroupPath);
    srcCg2.deleteEntry(srcKey);
    log() << m_currentFilename << ": Removing " << m_oldFile << ":" << srcCg2.name() << ":" << srcKey << ", moved." << endl;
}

void KonfUpdate::copyOrMoveGroup(const QStringList &srcGroupPath, const QStringList &dstGroupPath)
{
    KConfigGroup cg = KConfigUtils::openGroup(m_oldConfig1, srcGroupPath);

    // Keys
    Q_FOREACH(const QString &key, cg.keyList()) {
        copyOrMoveKey(srcGroupPath, key, dstGroupPath, key);
    }

    // Subgroups
    Q_FOREACH(const QString &group, cg.groupList()) {
        QStringList groupPath = QStringList() << group;
        copyOrMoveGroup(srcGroupPath + groupPath, dstGroupPath + groupPath);
    }
}

void KonfUpdate::gotRemoveKey(const QString &_key)
{
    QString key = _key.trimmed();

    if (key.isEmpty()) {
        logFileError() << "RemoveKey specifies invalid key" << endl;
        return;
    }

    if (!m_oldConfig1) {
        logFileError() << "Key without previous File specification" << endl;
        return;
    }

    KConfigGroup cg1 = KConfigUtils::openGroup(m_oldConfig1, m_oldGroup);
    if (!cg1.hasKey(key)) {
        return;
    }
    log() << m_currentFilename << ": RemoveKey removes " << m_oldFile << ":" << m_oldGroup << ":" << key << endl;

    // Delete old entry
    KConfigGroup cg2 = KConfigUtils::openGroup(m_oldConfig2, m_oldGroup);
    cg2.deleteEntry(key);
    /*if (m_oldConfig2->deleteGroup(m_oldGroup, KConfig::Normal)) { // Delete group if empty.
       log() << m_currentFilename << ": Removing empty group " << m_oldFile << ":" << m_oldGroup << endl;
    }   (this should be automatic)*/
}

void KonfUpdate::gotAllKeys()
{
    if (!m_oldConfig1) {
        logFileError() << "AllKeys without previous File specification" << endl;
        return;
    }

    copyOrMoveGroup(m_oldGroup, m_newGroup);
}

void KonfUpdate::gotAllGroups()
{
    if (!m_oldConfig1) {
        logFileError() << "AllGroups without previous File specification" << endl;
        return;
    }

    const QStringList allGroups = m_oldConfig1->groupList();
    for (QStringList::ConstIterator it = allGroups.begin();
            it != allGroups.end(); ++it) {
        m_oldGroup = QStringList() << *it;
        m_newGroup = m_oldGroup;
        gotAllKeys();
    }
}

void KonfUpdate::gotOptions(const QString &_options)
{
    const QStringList options = _options.split(',');
    for (QStringList::ConstIterator it = options.begin();
            it != options.end();
            ++it) {
        if ((*it).toLower().trimmed() == "copy") {
            m_bCopy = true;
        }

        if ((*it).toLower().trimmed() == "overwrite") {
            m_bOverwrite = true;
        }
    }
}

void KonfUpdate::copyGroup(const KConfigBase *cfg1, const QString &group1,
                           KConfigBase *cfg2, const QString &group2)
{
    KConfigGroup cg1(cfg1, group1);
    KConfigGroup cg2(cfg2, group2);
    copyGroup(cg1, cg2);
}

void KonfUpdate::copyGroup(const KConfigGroup &cg1, KConfigGroup &cg2)
{
    // Copy keys
    QMap<QString, QString> list = cg1.entryMap();
    for (QMap<QString, QString>::ConstIterator it = list.constBegin();
            it != list.constEnd(); ++it) {
        if (m_bOverwrite || !cg2.hasKey(it.key())) {
            cg2.writeEntry(it.key(), it.value());
        }
    }

    // Copy subgroups
    Q_FOREACH(const QString &group, cg1.groupList()) {
        copyGroup(&cg1, group, &cg2, group);
    }
}

void KonfUpdate::gotScriptArguments(const QString &_arguments)
{
    m_arguments = _arguments;
}

void KonfUpdate::gotScript(const QString &_script)
{
    QString script, interpreter;
    int i = _script.indexOf(',');
    if (i == -1) {
        script = _script.trimmed();
    } else {
        script = _script.left(i).trimmed();
        interpreter = _script.mid(i + 1).trimmed();
    }


    if (script.isEmpty()) {
        logFileError() << "Script fails to specify filename";
        m_skip = true;
        return;
    }



    QString path = KStandardDirs::locate("data", "kconf_update/" + script);
    if (path.isEmpty()) {
        if (interpreter.isEmpty()) {
            path = KStandardDirs::locate("lib", "kconf_update_bin/" + script);
        }

        if (path.isEmpty()) {
            logFileError() << "Script '" << script << "' not found" << endl;
            m_skip = true;
            return;
        }
    }

    if (!m_arguments.isNull()) {
        log() << m_currentFilename << ": Running script '" << script << "' with arguments '" << m_arguments << "'" << endl;
    } else {
        log() << m_currentFilename << ": Running script '" << script << "'" << endl;
    }

    QString cmd;
    if (interpreter.isEmpty()) {
        cmd = path;
    } else {
        cmd = interpreter + ' ' + path;
    }

    if (!m_arguments.isNull()) {
        cmd += ' ';
        cmd += m_arguments;
    }

    KTemporaryFile scriptIn;
    scriptIn.open();
    KTemporaryFile scriptOut;
    scriptOut.open();
    KTemporaryFile scriptErr;
    scriptErr.open();

    int result;
    if (m_oldConfig1) {
        if (m_debug) {
            scriptIn.setAutoRemove(false);
            log() << "Script input stored in " << scriptIn.fileName() << endl;
        }
        KConfig cfg(scriptIn.fileName(), KConfig::SimpleConfig);

        if (m_oldGroup.isEmpty()) {
            // Write all entries to tmpFile;
            const QStringList grpList = m_oldConfig1->groupList();
            for (QStringList::ConstIterator it = grpList.begin();
                    it != grpList.end();
                    ++it) {
                copyGroup(m_oldConfig1, *it, &cfg, *it);
            }
        } else {
            KConfigGroup cg1 = KConfigUtils::openGroup(m_oldConfig1, m_oldGroup);
            KConfigGroup cg2(&cfg, QString());
            copyGroup(cg1, cg2);
        }
        cfg.sync();
#ifndef _WIN32_WCE
        result = system(QFile::encodeName(QString("%1 < %2 > %3 2> %4").arg(cmd, scriptIn.fileName(), scriptOut.fileName(), scriptErr.fileName())));
#else
        QString path_ = QDir::convertSeparators ( QFileInfo ( cmd ).absoluteFilePath() );
        QString file_ = QFileInfo ( cmd ).fileName();
        SHELLEXECUTEINFO execInfo;
        memset ( &execInfo,0,sizeof ( execInfo ) );
        execInfo.cbSize = sizeof ( execInfo );
        execInfo.fMask =  SEE_MASK_FLAG_NO_UI;
        execInfo.lpVerb = L"open";
        execInfo.lpFile = (LPCWSTR) path_.utf16();
        execInfo.lpDirectory = (LPCWSTR) file_.utf16();
        execInfo.lpParameters = (LPCWSTR) QString(" < %1 > %2 2> %3").arg( scriptIn.fileName(), scriptOut.fileName(), scriptErr.fileName()).utf16();
        result = ShellExecuteEx ( &execInfo );
        if (result != 0)
        {
            result = 0;
        }
        else
        {
            result = -1;
        }
#endif
    } else {
        // No config file
#ifndef _WIN32_WCE
        result = system(QFile::encodeName(QString("%1 2> %2").arg(cmd, scriptErr.fileName())));
#else
        QString path_ = QDir::convertSeparators ( QFileInfo ( cmd ).absoluteFilePath() );
        QString file_ = QFileInfo ( cmd ).fileName();
        SHELLEXECUTEINFO execInfo;
        memset ( &execInfo,0,sizeof ( execInfo ) );
        execInfo.cbSize = sizeof ( execInfo );
        execInfo.fMask =  SEE_MASK_FLAG_NO_UI;
        execInfo.lpVerb = L"open";
        execInfo.lpFile = (LPCWSTR) path_.utf16();
        execInfo.lpDirectory = (LPCWSTR) file_.utf16();
        execInfo.lpParameters = (LPCWSTR) QString(" 2> %1").arg( scriptErr.fileName()).utf16();
        result = ShellExecuteEx ( &execInfo );
        if (result != 0)
        {
            result = 0;
        }
        else
        {
            result = -1;
        }
#endif
    }

    // Copy script stderr to log file
    {
        QFile output(scriptErr.fileName());
        if (output.open(QIODevice::ReadOnly)) {
            QTextStream ts(&output);
            ts.setCodec(QTextCodec::codecForName("UTF-8"));
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                log() << "[Script] " << line << endl;
            }
        }
    }

    if (result) {
        log() << m_currentFilename << ": !! An error occurred while running '" << cmd << "'" << endl;
        return;
    }

    if (!m_oldConfig1) {
        return; // Nothing to merge
    }

    if (m_debug) {
        scriptOut.setAutoRemove(false);
        log() << "Script output stored in " << scriptOut.fileName() << endl;
    }

    // Deleting old entries
    {
        QStringList group = m_oldGroup;
        QFile output(scriptOut.fileName());
        if (output.open(QIODevice::ReadOnly)) {
            QTextStream ts(&output);
            ts.setCodec(QTextCodec::codecForName("UTF-8"));
            while (!ts.atEnd()) {
                QString line = ts.readLine();
                if (line.startsWith('[')) {
                    group = parseGroupString(line);
                } else if (line.startsWith(QLatin1String("# DELETE "))) {
                    QString key = line.mid(9);
                    if (key[0] == '[') {
                        int j = key.lastIndexOf(']') + 1;
                        if (j > 0) {
                            group = parseGroupString(key.left(j));
                            key = key.mid(j);
                        }
                    }
                    KConfigGroup cg = KConfigUtils::openGroup(m_oldConfig2, group);
                    cg.deleteEntry(key);
                    log() << m_currentFilename << ": Script removes " << m_oldFile << ":" << group << ":" << key << endl;
                    /*if (m_oldConfig2->deleteGroup(group, KConfig::Normal)) { // Delete group if empty.
                       log() << m_currentFilename << ": Removing empty group " << m_oldFile << ":" << group << endl;
                    } (this should be automatic)*/
                } else if (line.startsWith(QLatin1String("# DELETEGROUP"))) {
                    QString str = line.mid(13).trimmed();
                    if (!str.isEmpty()) {
                        group = parseGroupString(str);
                    }
                    KConfigGroup cg = KConfigUtils::openGroup(m_oldConfig2, group);
                    cg.deleteGroup();
                    log() << m_currentFilename << ": Script removes group " << m_oldFile << ":" << group << endl;
                }
            }
        }
    }

    // Merging in new entries.
    KConfig scriptOutConfig(scriptOut.fileName(), KConfig::NoGlobals);
    if (m_newGroup.isEmpty()) {
        // Copy "default" keys as members of "default" keys
        copyGroup(&scriptOutConfig, QString(), m_newConfig, QString());
    } else {
        // Copy default keys as members of m_newGroup
        KConfigGroup srcCg = KConfigUtils::openGroup(&scriptOutConfig, QStringList());
        KConfigGroup dstCg = KConfigUtils::openGroup(m_newConfig, m_newGroup);
        copyGroup(srcCg, dstCg);
    }
    Q_FOREACH(const QString &group, scriptOutConfig.groupList()) {
        copyGroup(&scriptOutConfig, group, m_newConfig, group);
    }
}

void KonfUpdate::resetOptions()
{
    m_bCopy = false;
    m_bOverwrite = false;
    m_arguments.clear();
}


extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    KCmdLineOptions options;
    options.add("debug", ki18n("Keep output results from scripts"));
    options.add("check <update-file>", ki18n("Check whether config file itself requires updating"));
    options.add("+[file]", ki18n("File to read update instructions from"));

    KAboutData aboutData("kconf_update", 0, ki18n("KConf Update"),
                         "1.0.2",
                         ki18n("KDE Tool for updating user configuration files"),
                         KAboutData::License_GPL,
                         ki18n("(c) 2001, Waldo Bastian"));

    aboutData.addAuthor(ki18n("Waldo Bastian"), KLocalizedString(), "bastian@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KComponentData componentData(&aboutData);

    KonfUpdate konfUpdate;

    return 0;
}
