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

#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/Q_PID>
#include <QtCore/QTextCodec>
#include <QtCore/QTextIStream>

#include "kconfigbackend.h"

#include "kconfigbase.h"
#include "kconfigdata.h"
#include "kde_file.h"
#include "kglobal.h"
#include "kcomponentdata.h"
#include "kcomponentdata_p.h"
#include "klocale.h"
#include "ksavefile.h"
#include "kstandarddirs.h"
#include "kurl.h"

class KConfigBackEnd::Private
{
public:
   KLockFile::Ptr localLockFile;
   KLockFile::Ptr globalLockFile;
};

void KConfigBackEnd::changeFileName(const QString &_fileName,
                                    const char * _resType,
                                    bool _useKDEGlobals)
{
    mfileName = _fileName;
    resType = _resType;
    useKDEGlobals = _useKDEGlobals;
    if (mfileName.isEmpty()) {
        mLocalFileName.clear();
    }
    else if (QDir::isRelativePath(mfileName)) {
        mLocalFileName = pConfig->componentData().dirs()->saveLocation(resType) + mfileName;
    }
    else {
        mLocalFileName = mfileName;
    }

    if (useKDEGlobals)
        mGlobalFileName = pConfig->componentData().dirs()->saveLocation("config") +
                          QLatin1String("kdeglobals");
    else
        mGlobalFileName.clear();

    if (d->localLockFile) {
        pConfig->componentData().d->ref(); // deleting the KLockFile derefs the KComponentData, but we derefed it manually
        d->localLockFile = 0;
    }
    if (d->globalLockFile) {
        pConfig->componentData().d->ref(); // deleting the KLockFile derefs the KComponentData, but we derefed it manually
        d->globalLockFile = 0;
    }
}

QStringList KConfigBackEnd::extraConfigFiles() const
{
    return mMergeStack.toList();
}

void KConfigBackEnd::setExtraConfigFiles( const QStringList &files )
{
    removeAllExtraConfigFiles();
    foreach( const QString &file, files )
        mMergeStack.push(file);
}

void KConfigBackEnd::removeAllExtraConfigFiles() { mMergeStack.clear(); }

KLockFile::Ptr KConfigBackEnd::lockFile(bool bGlobal)
{
    if (bGlobal)
    {
        if (d->globalLockFile)
            return d->globalLockFile;

        if (!mGlobalFileName.isEmpty())
        {
            d->globalLockFile = new KLockFile(mGlobalFileName+".lock", pConfig->componentData());
            pConfig->componentData().d->deref(); // KLockFile refs the KComponentData, but it's enough
            // if pConfig refs it
            return d->globalLockFile;
        }
    }
    else
    {
        if (d->localLockFile)
            return d->localLockFile;

        QString fileName = mMergeStack.count() ? mMergeStack.top()
                           : mLocalFileName;
        if (!fileName.isEmpty())
        {
            d->localLockFile = new KLockFile(fileName+".lock", pConfig->componentData());
            pConfig->componentData().d->deref(); // KLockFile refs the KComponentData, but it's enough
            // if pConfig refs it
            return d->localLockFile;
        }
    }
    return KLockFile::Ptr();
}

KConfigBackEnd::KConfigBackEnd(KConfigBase *_config,
			       const QString &_fileName,
			       const char * _resType,
			       bool _useKDEGlobals)
    : pConfig(_config), bFileImmutable(false), mConfigState(KConfigBase::ReadWrite), mFileMode(-1),
      d(new Private)
{
    changeFileName(_fileName, _resType, _useKDEGlobals);
}

KConfigBackEnd::~KConfigBackEnd()
{
    if (d->localLockFile) {
        pConfig->componentData().d->ref(); // deleting the KLockFile derefs the KComponentData, but we derefed it manually
    }
    if (d->globalLockFile) {
        pConfig->componentData().d->ref(); // deleting the KLockFile derefs the KComponentData, but we derefed it manually
    }
    delete d;
}

void KConfigBackEnd::setFileWriteMode(int mode)
{
    mFileMode = mode;
}

void KConfigBackEnd::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

bool KConfigBackEnd::checkConfigFilesWritable(bool warnUser)
{
    // WARNING: Do NOT use the event loop as it may not exist at this time.
    bool allWritable = true;
    QString errorMsg;
    if ( !mLocalFileName.isEmpty() && !bFileImmutable && !KStandardDirs::checkAccess(mLocalFileName,W_OK) )
    {
        errorMsg = i18n("Will not save configuration.\n");
        allWritable = false;
        errorMsg += i18n("Configuration file \"%1\" not writable.\n", mLocalFileName);
    }
    // We do not have an immutability flag for kdeglobals. However, making kdeglobals mutable while making
    // the local config file immutable is senseless.
    if ( !mGlobalFileName.isEmpty() && useKDEGlobals && !bFileImmutable && !KStandardDirs::checkAccess(mGlobalFileName,W_OK) )
    {
        if ( errorMsg.isEmpty() )
            errorMsg = i18n("Will not save configuration.\n");
        errorMsg += i18n("Configuration file \"%1\" not writable.\n", mGlobalFileName);
        allWritable = false;
    }

    if (warnUser && !allWritable)
    {
        // Note: We don't ask the user if we should not ask this question again because we can't save the answer.
        errorMsg += i18n("Please contact your system administrator.");
        QString cmdToExec = KStandardDirs::findExe(QString("kdialog"));
        if (!cmdToExec.isEmpty() && pConfig->componentData().isValid())
        {
            QProcess::execute(cmdToExec,QStringList() << "--title" << pConfig->componentData().componentName() << "--msgbox" << errorMsg.toLocal8Bit());
        }
    }
    return allWritable;
}

